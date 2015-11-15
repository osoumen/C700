/************************************************************************

        Copyright (c) 2003-2005 Brad Martin.

This file is part of OpenSPC.

OpenSPC is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

OpenSPC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with OpenSPC; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



dsp.c: implements functions that emulate the DSP part of the SPC chip.
Some of these functions cannot be static because the SPC core needs access
to them, however they are not intended for external library use, and their
specific implementations and prototypes are subject to change.

 ************************************************************************/

/*========== INCLUDES ==========*/

#include <math.h>
#include <stdio.h>

#include "SPCimpl.h"
#include "dsp.h"
#include "gauss.h"

/*========== DEFINES ==========*/

#undef DEBUG
#undef DBG_KEY
#undef DBG_ENV
#undef DBG_PMOD
#undef DBG_BRR
#undef DBG_ECHO
#undef DBG_INTRP

#undef NO_PMOD
#undef NO_ECHO

#define CPU_RATE        ( 1024000 )
#define SAMP_FREQ       ( 32000 )

/*========== VARIABLES ==========*/

int                     keyed_on;
int                     keys;       /* 8-bits for 8 voices 			*/
voice_state_type        voice_state[ 8 ];

/* Noise stuff */
int		                noise_cnt;
int                     noise_lev;

/* These are for the FIR echo filter */
#ifndef NO_ECHO
static short            FIRlbuf[ 8 ];
static short            FIRrbuf[ 8 ];
static int              FIRptr;
static int              echo_ptr;
#endif

/*========== CONSTANTS ==========*/

/* Original SPC DSP took samples 32000 times a second, which is once every
   (1024000/32000 = 32) cycles. */
const int               TS_CYC = CPU_RATE / SAMP_FREQ;

/* Ptrs to Gaussian table */
static const int *      G1 = &gauss[ 256 ];
static const int *      G2 = &gauss[ 512 ];
static const int *      G3 = &gauss[ 255 ];
static const int *      G4 = &gauss[ -1  ];    

static const int        mask = 0xFF;

/* This table is for envelope timing.  It represents the number of counts
   that should be subtracted from the counter each sample period (32kHz). 
   The counter starts at 30720 (0x7800). */
static const int        CNT_INIT = 0x7800;
static const int        ENVCNT[ 0x20 ]
  = {
    0x0000, 0x000F, 0x0014, 0x0018, 0x001E, 0x0028, 0x0030, 0x003C,
    0x0050, 0x0060, 0x0078, 0x00A0, 0x00C0, 0x00F0, 0x0140, 0x0180,
    0x01E0, 0x0280, 0x0300, 0x03C0, 0x0500, 0x0600, 0x0780, 0x0A00,
    0x0C00, 0x0F00, 0x1400, 0x1800, 0x1E00, 0x2800, 0x3C00, 0x7800
    };

/*========== MACROS ==========*/

/* Make reading the ADSR code easier */
#define SL( v )         ( DSPregs[ ( ( v ) << 4 ) + 6 ] >> 5 )
                                    /* Returns SUSTAIN level        */
#define SR( v )         ( DSPregs[ ( ( v ) << 4 ) + 6 ] & 0x1F )
                                    /* Returns SUSTAIN rate         */

/* Handle endianness */
#ifdef WORDS_BIGENDIAN
#define LEtoME16( x )                                                \
    ( ( ( ( x ) >> 8 ) & 0xFF ) | ( ( ( x ) << 8 ) & 0xFF00 ) )
#define MEtoLE16( x )                                                \
    ( ( ( ( x ) >> 8 ) & 0xFF ) | ( ( ( x ) << 8 ) & 0xFF00 ) )
#else
#define LEtoME16( x )   ( x )
#define MEtoLE16( x )   ( x )
#endif

/*========== PROCEDURES ==========*/

static int AdvanceEnvelope          /* Run envelope step & retn ENVX*/
    (
    int                 v           /* Voice to process envelope for*/
    );

/* Privately shared functions (for internal library use only) */

/***** DSP_Reset *****/

void DSP_Reset                      /* Reset emulated DSP           */
    ( void )
{
int                     i;

#ifdef DEBUG
fprintf( stderr,"DSP_Reset\n" );
#endif

for( i = 0; i < 8; i++ )
    {
#ifndef NO_ECHO
    FIRlbuf[ i ]            = 0;
    FIRrbuf[ i ]            = 0;
#endif
    voice_state[ i ].on_cnt = 0;
    }

#ifndef NO_ECHO
FIRptr   = 0;
echo_ptr = 0;
#endif
keys             = 0;
keyed_on         = 0;
noise_cnt        = 0;
noise_lev        = 0x4000;
DSPregs[ 0x6C ] |= 0xE0;
DSPregs[ 0x4C ]  = 0;
DSPregs[ 0x5C ]  = 0;

}   /* DSP_Reset() */


/***** DSP_Update *****/

void DSP_Update                     /* Mix one sample of audio      */
    (
    short *             sound_ptr   /* Pointer to mix audio into    */
    )
{
int                     V;
#ifndef NO_ECHO
int                     echo_base;
#endif
int                     echol;
int                     echor;
int                     envx;
int                     m;
int                     outl;
int                     outr;
signed long             outx;       /* Smpl height (must be signed) */
src_dir_type *          sd;
int                     v;
int                     vl;
voice_state_type *      vp;
int                     vr;

sd = ( src_dir_type * )&SPC_RAM[ ( int )DSPregs[ 0x5D ] << 8 ];

/* Check for reset */
if( DSPregs[ 0x6C ] & 0x80 )
    {
    DSP_Reset();
    }

/* Here we check for keys on/off.  Docs say that successive writes to KON/KOF
   must be separated by at least 2 Ts periods or risk being neglected.
   Therefore DSP only looks at these during an update, and not at the time of
   the write.  Only need to do this once however, since the regs haven't
   changed over the whole period we need to catch up with. */
#ifdef DBG_KEY
DSPregs[ 0x4C ] &= mask;
#endif

/* Keying on a voice resets that bit in ENDX */
DSPregs[ 0x7C ] &= ~DSPregs[ 0x4C ];

/* Question: what is the expected behavior when pitch modulation is enabled on
   voice 0?  Jurassic Park 2 does this.  For now, using outx of zero for first
   voice. */
outx = 0;
/* Same table for noise and envelope */
noise_cnt -= ENVCNT[ DSPregs[ 0x6C ] & 0x1F ];
if( noise_cnt <= 0 )
    {
    noise_cnt = CNT_INIT;
    noise_lev = ( ( ( noise_lev << 13 ) ^ ( noise_lev << 14 ) ) & 0x4000 )
              | ( noise_lev >> 1                                         );
    }
outl  = 0;
outr  = 0;
echol = 0;
echor = 0;
for( v = 0, m = 1, V = 0; v < 8; v++, V += 16, m <<= 1 )
    {
    vp = &voice_state[ v ];
    if( vp->on_cnt && ( --vp->on_cnt == 0 ) )
        {
        /* Voice was keyed on */
        keys       |= m;
        keyed_on   |= m;
        vl          = DSPregs[ ( v<<4 ) + 4 ];
        vp->samp_id = *( unsigned long * )&sd[ vl ];
        vp->mem_ptr = LEtoME16( sd[ vl ].vptr );

#ifdef DBG_KEY
        fprintf(
               stderr,
               "Keying on voice %d, samp=0x%04X (0x%02X)\n",
               v,
               vp->mem_ptr,
               vl
               );
#endif

        vp->header_cnt = 0;
        vp->half       = 0;
        vp->envx       = 0;
        vp->end        = 0;
        vp->sampptr    = 0;
        vp->mixfrac    = 3 * 4096;

        /* NOTE: Real SNES does *not* appear to initialize the envelope
           counter to anything in particular.  The first cycle always seems to
           come at a random time sooner than expected; as yet, I have been
           unable to find any pattern.  I doubt it will matter though, so
           we'll go ahead and do the full time for now. */
        vp->envcnt   = CNT_INIT;
        vp->envstate = ATTACK;
        }

    if( DSPregs[ 0x4C ] & m & ~DSPregs[ 0x5C ] )
        {
        /* Voice doesn't come on if key off is set */
        DSPregs[ 0x4C ] &= ~m;
        vp->on_cnt       = 8;

#ifdef DBG_KEY
        fprintf( stderr, "Key on set for voice %d\n", v );
#endif
        }

    if( keys & DSPregs[ 0x5C ] & m )
        {
        /* Voice was keyed off */
        vp->envstate = RELEASE;
        vp->on_cnt   = 0;

#ifdef DBG_KEY
        fprintf( stderr, "Keying off voice %d\n", v );
#endif
        }

    if( !( keys & m & mask ) || ( ( envx = AdvanceEnvelope( v ) ) < 0 ) )
        {
        DSPregs[ V + 8 ] = 0;
        DSPregs[ V + 9 ] = 0;
        outx             = 0;
        continue;
        }

    vp->pitch = LEtoME16( *( ( unsigned short * )&DSPregs[ V + 2 ] ) )
              & 0x3FFF;

#ifndef NO_PMOD
    /* Pitch mod uses OUTX from last voice for this one.  Luckily we haven't
       modified OUTX since it was used for last voice. */
    if(DSPregs[ 0x2D ] & m )
        {
#ifdef DBG_PMOD
        fprintf(
               stderr,
               "Pitch Modulating voice %d, outx=%ld, old pitch=%d, ",
               v,
               outx,
               vp->pitch
               );
#endif
        vp->pitch = ( vp->pitch * ( outx + 32768 ) ) >> 15;
        }
#endif

#ifdef DBG_PMOD
    fprintf( stderr, "pitch=%d\n", vp->pitch );
#endif

    for( ; vp->mixfrac >= 0; vp->mixfrac -= 4096 )
        {
        /* This part performs the BRR decode 'on-the-fly'.  This is more
           correct than the old way, which could be fooled if the data and/or
           the loop point changed while the sample was playing, or if the BRR
           decode didn't produce the same result every loop because of the
           filters.  The event interface still has no chance of keeping up
           with those kinds of tricks, though. */
        if( !vp->header_cnt )
            {
            if( vp->end & 1 )
                {
                /* Docs say ENDX bit is set when decode of block with source
                   end flag set is done.  Does this apply to looping samples?
                   Some info I've seen suggests yes. */
                DSPregs[ 0x7C ] |= m;
                if( vp->end & 2 )
                    {
                    vp->mem_ptr = LEtoME16( sd[ DSPregs[ V + 4 ] ].lptr );

#ifdef DBG_BRR
                    fprintf(
                           stderr,
                           "BRR looping to 0x%04X\n",
                           vp->mem_ptr
                           );
#endif
                    }
                else
                    {
#ifdef DBG_KEY
                    fprintf( stderr, "BRR decode end, voice %d\n", v );
#endif

                    keys &= ~m;
                    DSPregs[ V + 8 ] = 0;
                    vp->envx         = 0;
                    while( vp->mixfrac >= 0 )
                        {
                        vp->sampbuf[ vp->sampptr ] = 0;
                        outx         = 0;
                        vp->sampptr  = ( vp->sampptr + 1 ) & 3;
                        vp->mixfrac -= 4096;
                        }
                    break;
                    }
                }
            vp->header_cnt = 8;
            vl             = ( unsigned char )SPC_RAM[ vp->mem_ptr++ ];
            vp->range      = vl >> 4;
            vp->end        = vl & 3;
            vp->filter     = ( vl & 12 ) >> 2;

#ifdef DBG_BRR
            fprintf(
                   stderr,
                   "V%d: header read, range=%d, end=%d, filter=%d\n",
                   v,
                   vp->range,
                   vp->end,
                   vp->filter
                   );
#endif
            }
        if( vp->half == 0 )
            {
            vp->half = 1;
            outx     = ( ( signed char )SPC_RAM[ vp->mem_ptr ] ) >> 4;
            }
        else
            {
            vp->half = 0;
            /* Funkiness to get 4-bit signed to carry through */
            outx   = ( signed char )( SPC_RAM[ vp->mem_ptr++ ] << 4 );
            outx >>= 4;
            vp->header_cnt--;
            }

#ifdef DBG_BRR
        fprintf(
               stderr,
               "V%d: nybble=%X, ptr=%04X, smp1=%d, smp2=%d\n",
               v,
               outx & 0xF,
               vp->mem_ptr,
               vp->smp1,
               vp->smp2
               );
#endif

        /* For invalid ranges (D,E,F): if the nybble is negative, the result
           is F000.  If positive, 0000.  Nothing else like previous range,
           etc. seems to have any effect.  If range is valid, do the shift
           normally.  Note these are both shifted right once to do the filters
           properly, but the output will be shifted back again at the end. */
        if( vp->range <= 0xC )
            {
            outx = ( outx << vp->range ) >> 1;
            }
        else
            {
            outx &= ~0x7FF;

#ifdef DBG_BRR
            fprintf( stderr, "V%d: invalid range! (%X)\n", v, vp->range );
#endif
            }

#ifdef DBG_BRR
        fprintf( 
               stderr,
               "V%d: shifted delta=%04X\n",
               v,
               ( unsigned short )outx
               );
#endif

        switch( vp->filter )
            {
            case 0:
                break;

            case 1:
                outx += ( vp->smp1 >> 1 ) + ( ( -vp->smp1 ) >> 5 );
                break;

            case 2:
                outx += vp->smp1
                      + ( ( -( vp->smp1 + ( vp->smp1 >> 1 ) ) ) >> 5 )
                      - ( vp->smp2 >> 1 ) + ( vp->smp2 >> 5 );
                break;

            case 3:
                outx += vp->smp1
                      + ( ( -( vp->smp1 + ( vp->smp1 << 2 )
                             + ( vp->smp1 << 3 )            ) ) >> 7 )
                      - ( vp->smp2 >> 1                              )
                      + ( ( vp->smp2 + ( vp->smp2 >> 1 )      ) >> 4 );
                break;
            }

        if( outx < ( signed short )0x8000 )
            {
            outx = ( signed short )0x8000;
            }
        else if( outx > ( signed short )0x7FFF )
            {
            outx = ( signed short )0x7FFF;
            }

#ifdef DBG_BRR
        fprintf(
               stderr,
               "V%d: filter + delta=%04X\n",
               v,
               ( unsigned short )outx
               );
#endif

        vp->smp2                   = ( signed short )vp->smp1;
        vp->smp1                   = ( signed short )( outx << 1 );
        vp->sampbuf[ vp->sampptr ] = vp->smp1;

#ifdef DBG_BRR
        fprintf(
               stderr,
               "V%d: final output: %04X\n",
               v,
               vp->sampbuf[ vp->sampptr ]
               );
#endif

        vp->sampptr = ( vp->sampptr + 1 ) & 3;
        }

    if(DSPregs[ 0x3D ] & m )
        {
#ifdef DBG_PMOD
        fprintf( stderr, "Noise enabled, voice %d\n", v );
#endif
        outx = ( signed short )( noise_lev << 1 );
        }
    else
        {
        /* Perform 4-Point Gaussian interpolation.  Take an approximation of a
           Gaussian bell-curve, and move it through the sample data at a rate
           determined by the pitch.  The sample output at any given time is
           the sum of the products of each input sample point with the value
           of the bell-curve corresponding to that point. */
        vl  = vp->mixfrac >> 4;
        vr  = ( ( G4[ -vl ] * vp->sampbuf[ vp->sampptr ] ) >> 11 ) & ~1;
        vr += ( ( G3[ -vl ]
                * vp->sampbuf[ ( vp->sampptr + 1 ) & 3 ] ) >> 11 ) & ~1;
        vr += ( ( G2[ vl ]
                * vp->sampbuf[ ( vp->sampptr + 2 ) & 3 ] ) >> 11 ) & ~1;

        /* This is to do the wrapping properly.  Based on my tests with the
           SNES, it appears clipping is done only if it is the fourth addition
           that would cause a wrap.  If it has already wrapped before the
           fourth addition, it is not clipped. */
        vr  = ( signed short )vr;
        vr += ( ( G1[ vl ]
                * vp->sampbuf[ ( vp->sampptr + 3 ) & 3 ] ) >> 11 ) & ~1;
        if( vr > 32767 )
            {
            vr = 32767;
            }
        else if( vr < -32768 )
            {
            vr = -32768;
            }
        outx = ( signed short )vr;

#ifdef DBG_INTRP
        fprintf(
               stderr,
               "V%d: mixfrac=%d: [%d]*%d + [%d]*%d + [%d]*%d + [%d]*%d "\
                "= %d\n",
               v,
               vl,
               G1[ vl ],
               vp->sampbuf[ ( vp->sampptr + 3 ) & 3 ],
               G2[ vl ],
               vp->sampbuf[ ( vp->sampptr + 2 ) & 3 ],
               G3[ -vl ],
               vp->sampbuf[ ( vp->sampptr + 1 ) & 3 ],
               G4[ -vl ],
               vp->sampbuf[ vp->sampptr ],
               outx
               );
#endif
        }
    
    /* Advance the sample position for next update. */
    vp->mixfrac += vp->pitch;
    
    outx = ( ( outx * envx ) >> 11 ) & ~1;
    DSPregs[ V + 9 ] = outx >> 8;

    vl = ( ( ( int )( signed char )DSPregs[ V     ] ) * outx ) >> 7;
    vr = ( ( ( int )( signed char )DSPregs[ V + 1 ] ) * outx ) >> 7;
    outl += vl;
    outr += vr;
    if( DSPregs[ 0x4D ] & m )
        {
        echol += vl;
        echor += vr;
        }
    }
outl = ( outl * ( signed char )DSPregs[ 0x0C ] ) >> 7;
outr = ( outr * ( signed char )DSPregs[ 0x1C ] ) >> 7;

#ifndef NO_ECHO
/* Perform echo.  First, read mem at current location, and put those samples
   into the FIR filter queue. */
#ifdef DBG_ECHO
fprintf(
       stderr,
       "Echo delay=%dms, feedback=%d%%\n",
       DSPregs[ 0x7D ] * 16,
       ( ( signed char )DSPregs[ 0x0D ] * 100 ) / 0x7F
       );
#endif

echo_base = ( ( DSPregs[ 0x6D ] << 8 ) + echo_ptr ) & 0xFFFF;
FIRlbuf[ FIRptr ]
  = ( signed short )LEtoME16(
                            *( unsigned short * )
                              &SPC_RAM[ echo_base ]
                            );
FIRrbuf[ FIRptr ]
  = ( signed short )LEtoME16(
                            *( unsigned short * )
                              &SPC_RAM[ echo_base + sizeof( short ) ]
                            );

/* Now, evaluate the FIR filter, and add the results into the final output. */
vl     = FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x7F ];
vr     = FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x7F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x6F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x6F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x5F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x5F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x4F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x4F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x3F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x3F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x2F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x2F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x1F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x1F ];
FIRptr = ( FIRptr + 1 ) & 7;
vl    += FIRlbuf[ FIRptr ] * ( signed char )DSPregs[ 0x0F ];
vr    += FIRrbuf[ FIRptr ] * ( signed char )DSPregs[ 0x0F ];

#ifdef DBG_ECHO
fprintf(
       stderr,
       "FIR Coefficients: %02X %02X %02X %02X %02X %02X %02X %02X\n",
       DSPregs[ 0x0F ],
       DSPregs[ 0x1F ],
       DSPregs[ 0x2F ],
       DSPregs[ 0x3F ],
       DSPregs[ 0x4F ],
       DSPregs[ 0x5F ],
       DSPregs[ 0x6F ],
       DSPregs[ 0x7F ]
       );
#endif

/* FIRptr is left in the position of the oldest sample, the one that will be
   replaced next update. */
outl += vl * ( signed char )DSPregs[ 0x2C ] >> 14;
outr += vr * ( signed char )DSPregs[ 0x3C ] >> 14;

if( !( DSPregs[ 0x6C ] & 0x20 ) )
    {
    /* Add the echo feedback back into the original result, and save that into
       memory for use later. */
    echol += vl * ( signed char )DSPregs[ 0x0D ] >> 14;
    if( echol > 32767 )
        {
        echol = 32767;
        }
    else if( echol < -32768 )
        {
        echol = -32768;
        }
    echor += vr * ( signed char )DSPregs[ 0x0D ] >> 14;
    if( echor > 32767 )
        {
        echor = 32767;
        }
    else if( echor < -32768 )
        {
        echor = -32768;
        }

#ifdef DBG_ECHO
    fprintf(
           stderr,
           "Echo: Writing %04X,%04X at location %04X\n",
           ( unsigned short )echol,
           ( unsigned short )echor,
           echo_base
           );
#endif

    *( unsigned short * )&SPC_RAM[ echo_base ]
        = MEtoLE16( ( unsigned short )echol );
    *( unsigned short * )&SPC_RAM[ echo_base + sizeof( short ) ]
        = MEtoLE16( ( unsigned short )echor );
    }

echo_ptr += 2 * sizeof( short );
if( echo_ptr >= ( ( DSPregs[ 0x7D ] & 0xF ) << 11 ) )
    {
    echo_ptr = 0;
    }
#endif                              /* !defined( NO_ECHO )          */

if( sound_ptr )
    {
    if( DSPregs[ 0x6C ] & 0x40 )
        {
        /* MUTE */
#ifdef DEBUG
        fprintf( stderr, "MUTED!\n" );
#endif

        *sound_ptr = 0;
        sound_ptr++;
        *sound_ptr = 0;
        sound_ptr++;
        }
    else
        {
        if( outl > 32767 )
            {
            *sound_ptr = 32767;
            }
        else if( outl < -32768 )
            {
            *sound_ptr = -32768;
            }
        else
            {
            *sound_ptr = outl;
            }
        sound_ptr++;
        if( outr > 32767 )
            {
            *sound_ptr = 32767;
            }
        else if( outr < -32768 )
            {
            *sound_ptr = -32768;
            }
        else
            {
            *sound_ptr = outr;
            }
        sound_ptr++;
        }
    }

}   /* DSP_Update() */


/***** AdvanceEnvelope *****/

static int AdvanceEnvelope          /* Run envelope step & retn ENVX*/
    (
    int                 v           /* Voice to process envelope for*/
    )
{
int                     envx;
int                     cnt;
int                     adsr1;
int                     t;

envx = voice_state[ v ].envx;

if( voice_state[ v ].envstate == RELEASE )
    {
    /* Docs: "When in the state of "key off". the "click" sound is prevented
       by the addition of the fixed value 1/256"  WTF???  Alright, I'm going
       to choose to interpret that this way:  When a note is keyed off, start
       the RELEASE state, which subtracts 1/256th each sample period (32kHz).
       Note there's no need for a count because it always happens every
       update. */
    envx -= 0x8;                    /* 0x8 / 0x800 = 1/256th        */
    if( envx <= 0 )
        {
        envx = 0;
        keys &= ~( 1 << v ); 
        return -1;
        }
    voice_state[ v ].envx = envx;
    DSPregs[ ( v << 4 ) + 8 ] = envx >> 8;

#ifdef DBG_ENV
    fprintf(
           stderr,
           "ENV voice %d: envx=%03X, state=RELEASE\n",
           v,
           envx
           );
#endif

    return( envx );
    }

cnt   = voice_state[ v ].envcnt;
adsr1 = DSPregs[ ( v << 4 ) + 5 ];
if( adsr1 & 0x80 )
    {
    switch( voice_state[ v ].envstate )
        {
        case ATTACK:
            /* Docs are very confusing.  "AR is multiplied by the fixed value
               1/64..."  I believe it means to add 1/64th to ENVX once every
               time ATTACK is updated, and that's what I'm going to implement.
               */
            t = adsr1 & 0xF;
            if( t == 0xF )
                {
#ifdef DBG_ENV
                fprintf( stderr, "ENV voice %d: instant attack\n", v );
#endif

                envx += 0x400;
                }
            else
                {
                cnt -= ENVCNT[ ( t << 1 ) + 1 ];
                if( cnt > 0 )
                    {
                    break;
                    }
                envx += 0x20;       /* 0x020 / 0x800 = 1/64         */
                cnt   = CNT_INIT;
                }

            if( envx > 0x7FF )
                {
                envx = 0x7FF;
                voice_state[ v ].envstate = DECAY;
                }

#ifdef DBG_ENV
            fprintf(
                   stderr,
                   "ENV voice %d: envx=%03X, state=ATTACK\n",
                   v,
                   envx
                   );
#endif

            voice_state[ v ].envx = envx;

            break;

        case DECAY:
            /* Docs: "DR... [is multiplied] by the fixed value 1-1/256."
               Well, at least that makes some sense.  Multiplying ENVX by
               255/256 every time DECAY is updated. */
            cnt -= ENVCNT[ ( ( adsr1 >> 3 ) & 0xE ) + 0x10 ];
            if( cnt <= 0 )
                {
                cnt   = CNT_INIT;
                envx -= ( ( envx - 1 ) >> 8 ) + 1;
                voice_state[ v ].envx = envx;
                }

            if( envx <= 0x100 * ( SL( v ) + 1 ) )
                {
                voice_state[ v ].envstate = SUSTAIN;
                }

#ifdef DBG_ENV
            fprintf(
                   stderr,
                   "ENV voice %d: envx=%03X, state=DECAY\n",
                   v,
                   envx
                   );
#endif

            break;

        case SUSTAIN:
            /* Docs: "SR [is multiplied] by the fixed value 1-1/256."
               Multiplying ENVX by 255/256 every time SUSTAIN is updated. */
#ifdef DBG_ENV
            if( ENVCNT[ SR( v ) ] == 0 )
                {
                fprintf(
                       stderr,
                       "ENV voice %d: envx=%03X, state=SUSTAIN, zero rate\n",
                       v,
                       envx
                       );
                }
#endif

            cnt -= ENVCNT[ SR( v ) ];
            if( cnt > 0 )
                {
                break;
                }
            cnt   = CNT_INIT;
            envx -= ( ( envx - 1 ) >> 8 ) + 1;

#ifdef DBG_ENV
            fprintf(
                   stderr,
                   "ENV voice %d: envx=%03X, state=SUSTAIN\n",
                   v,
                   envx
                   );
#endif

            voice_state[ v ].envx = envx;

            /* Note: no way out of this state except by explicit KEY OFF (or
               switch to GAIN). */
            break;

        case RELEASE:   /* Handled earlier to prevent GAIN mode from stopping
                           KEY OFF events */
            break;
        }
    }
else
    {
    /* GAIN mode is set
       Note: if the game switches between ADSR and GAIN modes partway
       through, should the count be reset, or should it continue from
       where it was?  Does the DSP actually watch for that bit to
       change, or does it just go along with whatever it sees when it
       performs the update?  I'm going to assume the latter and not
       update the count, unless I see a game that obviously wants the
       other behavior.  The effect would be pretty subtle, in any case.
       */
    t = DSPregs[ ( v << 4 ) + 7 ];
    if( t < 0x80 )
        {
        envx                  = t << 4;
        voice_state[ v ].envx = envx;

#ifdef DBG_ENV
        fprintf(
               stderr,
               "ENV voice %d: envx=%03X, state=DIRECT\n",
               v,
               envx
               );
#endif
        }
    else
        {
        switch( t >> 5 )
            {
            case 4:
                /* Docs: "Decrease (linear): Subtraction of the fixed value
                   1/64." */
                cnt -= ENVCNT[ t & 0x1F ];
                if( cnt > 0 )
                    {
                    break;
                    }
                cnt   = CNT_INIT;
                envx -= 0x020;      /* 0x020 / 0x800 = 1/64th       */
                if( envx < 0 )
                    {
                    envx = 0;
                    }

#ifdef DBG_ENV
                fprintf(
                       stderr,
                       "ENV voice %d: envx=%03X, state=DECREASE\n",
                       v,
                       envx
                       );
#endif

                voice_state[ v ].envx = envx;

                break;

            case 5:
                /* Docs: "Drecrease <sic> (exponential): Multiplication by
                   the fixed value 1-1/256." */
                cnt -= ENVCNT[ t & 0x1F ];
                if( cnt > 0 )
                    {
                    break;
                    }
                cnt = CNT_INIT;
                envx -= ( ( envx - 1 ) >> 8 ) + 1;

#ifdef DBG_ENV
                fprintf(
                       stderr,
                       "ENV voice %d: envx=%03X, state=EXP\n",
                       v,
                       envx
                       );
#endif

                voice_state[ v ].envx = envx;

                break;

            case 6:
                /* Docs: "Increase (linear): Addition of the fixed value
                   1/64." */
                cnt -= ENVCNT[ t & 0x1F ];
                if( cnt > 0 )
                {
                    break;
                }
                cnt = CNT_INIT;
                envx += 0x020;      /* 0x020 / 0x800 = 1/64th   */
                if( envx > 0x7FF )
                {
                    envx = 0x7FF;
                }

#ifdef DBG_ENV
                fprintf(
                       stderr,
                       "ENV voice %d: envx=%03X, state=INCREASE\n",
                       v,
                       envx
                       );
#endif

                voice_state[ v ].envx = envx;

                break;

            case 7:
                /* Docs: "Increase (bent line): Addition of the constant
                   1/64 up to .75 of the constaint <sic> 1/256 from .75 to
                   1." */
                cnt -= ENVCNT[ t & 0x1F ];
                if( cnt > 0 )
                    {
                    break;
                    }
                cnt = CNT_INIT;
                if( envx < 0x600 )  /* 0x600 / 0x800 = .75          */
                    {
                    envx += 0x020;  /* 0x020 / 0x800 = 1/64         */
                    }
                else
                    {
                    envx += 0x008;  /* 0x008 / 0x800 = 1/256        */
                    }
                if( envx > 0x7FF )
                    {
                    envx=0x7FF;
                    }

#ifdef DBG_ENV
                fprintf(
                       stderr,
                       "ENV voice %d: envx=%03X, state=INCREASE\n",
                       v,
                       envx
                       );
#endif

                voice_state[ v ].envx = envx;

                break;
            }
        }
    }

voice_state[ v ].envcnt   = cnt;
DSPregs[ ( v << 4 ) + 8 ] = envx >> 4;

return( envx );

}    /* AdvanceEnvelope() */
