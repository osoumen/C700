/**************************************************************************

        Copyright (c) 2003-2005 Brad Martin.
        Some portions copyright (c) 1998-2005 Charles Bilyue'.

This file is part of OpenSPC.

SPCimpl.c: This file is a bridge between the OpenSPC library and the
specific SPC core implementation (in this case, SNEeSe's).  As the licensing
rights for SNEeSe are different from the rest of OpenSPC, none of the files
in this directory are LGPL.  Although this file was created by me (Brad
Martin), it contains some code derived from SNEeSe and therefore falls under
its license.  See the file 'LICENSE' in this directory for more information.

 **************************************************************************/

/*========== INCLUDES ==========*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dsp.h"
#include "SPCimpl.h"

#undef NO_ENVX
#undef CLEAR_PORTS
#undef DBG_DSP

/*========== MEMORY CONSTANTS ==========*/

static const char *     SPC_OpID[ 256 ]
  = {
    "NOP"            ,"TCALL 0"        ,"SET1 dp.0"      ,"BBS dp.0,rel"   ,
    "OR A,dp"        ,"OR A,labs"      ,"OR A,(X)"       ,"OR A,(dp+X)"    ,
    "OR A,#imm"      ,"OR dp(d),dp(s)" ,"OR1 C,mem.bit"  ,"ASL dp"         ,
    "ASL labs"       ,"PUSH PSW"       ,"TSET1 labs"     ,"BRK"            ,
                                    /* Opcode page 0X               */

    "BPL rel"        ,"TCALL 1"        ,"CLR1 dp.0"      ,"BBC dp.0,rel"   ,
    "OR A,dp+X"      ,"OR A,labs+X"    ,"OR A,labs+Y"    ,"OR A,(dp)+Y"    ,
    "OR dp,#imm"     ,"OR (X),(Y)"     ,"DECW dp"        ,"ASL dp+X"       ,
    "ASL A"          ,"DEC X"          ,"CMP X,labs"     ,"JMP (abs,x)"    ,
                                    /* Opcode page 1X               */

    "CLRP"           ,"TCALL 2"        ,"SET1 dp.1"      ,"BBS dp.1,rel"   ,
    "AND A,dp"       ,"AND A,labs"     ,"AND A,(X)"      ,"AND A,(dp+X)"   ,
    "AND A,#imm"     ,"AND dp(d),dp(s)","OR1 C,/mem.bit" ,"ROL dp"         ,
    "ROL labs"       ,"PUSH A"         ,"CBNE dp"        ,"BRA rel"        ,
                                    /* Opcode page 2X               */

    "BMI rel"        ,"TCALL 3"        ,"CLR1 dp.1"      ,"BBC dp.1,rel"   ,
    "AND A,dp+X"     ,"AND A,labs+X"   ,"AND A,labs+Y"   ,"AND A,(dp)+Y"   ,
    "AND dp,#imm"    ,"AND (X),(Y)"    ,"INCW dp"        ,"ROL dp+X"       ,
    "ROL A"          ,"INC X"          ,"CMP X,dp"       ,"CALL labs"      ,
                                    /* Opcode page 3X               */

    "SETP"           ,"TCALL 4"        ,"SET1 dp.2"      ,"BBS dp.2,rel"   ,
    "EOR A,dp"       ,"EOR A,labs"     ,"EOR A,(X)"      ,"EOR A,(dp+X)"   ,
    "EOR A,#imm"     ,"EOR dp(d),dp(s)","AND1 C,mem.bit" ,"LSR dp"         ,
    "LSR labs"       ,"PUSH X"         ,"TCLR1 labs"     ,"PCALL upage"    ,
                                    /* Opcode page 4X               */

    "BVC rel"        ,"TCALL 5"        ,"CLR1 dp.2"      ,"BBC dp.2,rel"   ,
    "EOR A,dp+X"     ,"EOR A,labs+X"   ,"EOR A,labs+Y"   ,"EOR A,(dp)+Y"   ,
    "EOR dp,#imm"    ,"EOR (X),(Y)"    ,"CMPW YA,dp"     ,"LSR dp+X"       ,
    "LSR A"          ,"MOV X,A"        ,"CMP Y,labs"     ,"JMP labs"       ,
                                    /* Opcode page 5X               */

    "CLRC"           ,"TCALL 6"        ,"SET1 dp.3"      ,"BBS dp.3,rel"   ,
    "CMP A,dp"       ,"CMP A,labs"     ,"CMP A,(X)"      ,"CMP A,(dp+X)"   ,
    "CMP A,#imm"     ,"CMP dp(d),dp(s)","AND1 C,/mem.bit","ROR dp"         ,
    "ROR labs"       ,"PUSH Y"         ,"DBNZ dp,rel"    ,"RET"            ,
                                    /* Opcode page 6X               */

    "BVS rel"        ,"TCALL 7"        ,"CLR1 dp.3"      ,"BBC dp.3,rel"   ,
    "CMP A,dp+X"     ,"CMP A,labs+X"   ,"CMP A,labs+Y"   ,"CMP A,(dp)+Y"   ,
    "CMP dp,#imm"    ,"CMP (X),(Y)"    ,"ADDW YA,dp"     ,"ROR dp+X"       ,
    "ROR A"          ,"MOV A,X"        ,"CMP Y,dp"       ,"RETI"           ,
                                    /* Opcode page 7X               */

    "SETC"           ,"TCALL 8"        ,"SET1 dp.4"      ,"BBS dp.4,rel"   ,
    "ADC A,dp"       ,"ADC A,labs"     ,"ADC A,(X)"      ,"ADC A,(dp+X)"   ,
    "ADC A,#imm"     ,"ADC dp(d),dp(s)","EOR1 C,mem.bit" ,"DEC dp"         ,
    "DEC labs"       ,"MOV Y,#imm"     ,"POP PSW"        ,"MOV dp,#imm"    ,
                                    /* Opcode page 8X               */

    "BCC rel"        ,"TCALL 9"        ,"CLR1 dp.4"      ,"BBC dp.4,rel"   ,
    "ADC A,dp+X"     ,"ADC A,labs+X"   ,"ADC A,labs+Y"   ,"ADC A,(dp)+Y"   ,
    "ADC dp,#imm"    ,"ADC (X),(Y)"    ,"SUBW YA,dp"     ,"DEC dp+X"       ,
    "DEC A"          ,"MOV X,SP"       ,"DIV YA,X"       ,"XCN A"          ,
                                    /* Opcode page 9X               */

    "EI"             ,"TCALL 10"       ,"SET1 dp.5"      ,"BBS dp.5,rel"   ,
    "SBC A,dp"       ,"SBC A,labs"     ,"SBC A,(X)"      ,"SBC A,(dp+X)"   ,
    "SBC A,#imm"     ,"SBC dp(d),dp(s)","MOV1 C,mem.bit" ,"INC dp"         ,
    "INC labs"       ,"CMP Y,#imm"     ,"POP A"          ,"MOV (X)+,A"     ,
                                    /* Opcode page AX               */

    "BCS rel"        ,"TCALL 11"       ,"CLR1 dp.5"      ,"BBC dp.5,rel"   ,
    "SBC A,dp+X"     ,"SBC A,labs+X"   ,"SBC A,labs+Y"   ,"SBC A,(dp)+Y"   ,
    "SBC dp,#imm"    ,"SBC (X),(Y)"    ,"MOVW YA,dp"     ,"INC dp+X"       ,
    "INC A"          ,"MOV SP,X"       ,"DAS A"          ,"MOV A,(X)+"     ,
                                    /* Opcode page BX               */

    "DI"             ,"TCALL 12"       ,"SET1 dp.6"      ,"BBS dp.6,rel"   ,
    "MOV dp,A"       ,"MOV labs,A"     ,"MOV (X),A"      ,"MOV (dp+X),A"   ,
    "CMP X,#imm"     ,"MOV labs,X"     ,"MOV1 mem.bit,C" ,"MOV dp,Y"       ,
    "MOV labs,Y"     ,"MOV X,#imm"     ,"POP X"          ,"MUL YA"         ,
                                    /* Opcode page CX               */

    "BNE rel"        ,"TCALL 13"       ,"CLR1 dp.6"      ,"BBC dp.6,rel"   ,
    "MOV dp+X,A"     ,"MOV labs+X,A"   ,"MOV labs+Y,A"   ,"MOV (dp)+Y,A"   ,
    "MOV dp,X"       ,"MOV dp+Y,X"     ,"MOVW dp,YA"     ,"MOV dp+X,Y"     ,
    "DEC Y"          ,"MOV A,Y"        ,"CBNE dp+X,rel"  ,"DAA A"          ,
                                    /* Opcode page DX               */

    "CLRV"           ,"TCALL 14"       ,"SET1 dp.7"      ,"BBS dp.7,rel"   ,
    "MOV A,dp"       ,"MOV A,labs"     ,"MOV A,(X)"      ,"MOV A,(dp+X)"   ,
    "MOV A,#imm"     ,"MOV X,labs"     ,"NOT1 mem.bit"   ,"MOV Y,dp"       ,
    "MOV Y,labs"     ,"NOTC"           ,"POP Y"          ,"SLEEP"          ,
                                    /* Opcode page EX               */

    "BEQ rel"        ,"TCALL 15"       ,"CLR1 dp.7"      ,"BBC dp.7,rel"   ,
    "MOV A,dp+X"     ,"MOV A,labs+X"   ,"MOV A,labs+Y"   ,"MOV A,(dp)+Y"   ,
    "MOV X,dp"       ,"MOV X,dp+Y"     ,"MOV dp(d),dp(s)","MOV Y,dp+X"     ,
    "INC Y"          ,"MOV Y,A"        ,"DBNZ Y,rel"     ,"STOP"
                                    /* Opcode page FX               */
    };

/*========== VARIABLES ==========*/

unsigned char           In_CPU;
unsigned                Map_Address;
unsigned                Map_Byte;
unsigned                SPC_CPU_cycle_divisor;
unsigned                SPC_CPU_cycle_multiplicand;
unsigned                SPC_CPU_cycles;
unsigned                SPC_CPU_cycles_mul;
unsigned char           SPC_DSP[ 256 ];
unsigned                SPC_DSP_DATA;
unsigned char           SPCRAM[ 65536 ];
unsigned                sound_cycle_latch;

/*========== PROCEDURES ==========*/


/***** SPC_SetState *****/

void SPC_SetState
    (
    int                 pc,
    int                 a,
    int                 x,
    int                 y,
    int                 p,
    int                 sp,
    void *              ram
    )
{
/* Initialize SPC memory.  Also initialize the state of the 0xFFC0 ROM being
   switched in/out. */
memcpy( SPCRAM, ram, 65536 );
if( 0 == ( SPC_CTRL & 0x80 ) )
    {
    active_context->FFC0_Address = SPCRAM;
    }

/* Initialize SPC timers to the values the saved RAM indicates were active */
active_context->timers[ 0 ].target
    = ( unsigned char )( SPCRAM[ 0xFA ] - 1 ) + 1;
active_context->timers[ 1 ].target
    = ( unsigned char )( SPCRAM[ 0xFB ] - 1 ) + 1;
active_context->timers[ 2 ].target 
    = ( unsigned char )( SPCRAM[ 0xFC ] - 1 ) + 1;
active_context->timers[ 0 ].counter = SPCRAM[ 0xFD ] & 0xF;
active_context->timers[ 1 ].counter = SPCRAM[ 0xFE ] & 0xF;
active_context->timers[ 2 ].counter = SPCRAM[ 0xFF ] & 0xF;

/* Initialize SPC <-> CPU communications registers to the values the saved RAM
   indicates were active */
active_context->PORT_R[ 0 ] = SPCRAM[ 0xF4 ];
active_context->PORT_R[ 1 ] = SPCRAM[ 0xF5 ];
active_context->PORT_R[ 2 ] = SPCRAM[ 0xF6 ];
active_context->PORT_R[ 3 ] = SPCRAM[ 0xF7 ];

/* Initialize SPC registers and associated values */
active_context->PC.w   = pc;
active_context->YA.b.l = a;
active_context->X      = x;
active_context->YA.b.h = y;
active_context->SP     = sp;

/* Now we have to set up the PSW.  Fortunately, SNEeSe now has a function to
   set its internal state up for us. */
active_context->PSW = p;
spc_restore_flags();

#ifdef CLEAR_PORTS
/* Hack: if any of the control port 'port clear' bits are set, carry out that
   clear now.  Not sure how they would get set and not have already been
   cleared, but I'm hoping this will fix an issue I'm seeing with an SPC file
   that apparently does have it set.
   Update: breaks Actraiser, so hack reverted */
if( SPC_CTRL & 0x10 )
    {
    active_context->PORT_R[ 0 ] = 0;
    active_context->PORT_R[ 1 ] = 0;
    }
if( SPC_CTRL & 0x20 )
    {
    active_context->PORT_R[ 2 ] = 0;
    active_context->PORT_R[ 3 ] = 0;
    }
#endif

}   /* SPC_SetState() */


/* These are to be called from SNEeSe SPC core only, not from the rest of the
   library. */

/***** DisplaySPC *****/

void DisplaySPC
    ( void )
{
int                     c;
char                    Message[ 9 ];

fprintf( stderr, "\nSPC registers\n" );
fprintf(
       stderr,
       "PC:%04X  SP:%04X  NVPBHIZC\n",
       active_context->PC.w,
       active_context->SP
       );

active_context->PSW = get_SPC_PSW();
for( c = 0; c < 8; c++ )
    {
    Message[ 7 - c ] = ( active_context->PSW & ( 1 << c ) ) ? '1' : '0';
    }
Message[ 8 ] = 0;

fprintf(
       stderr,
       "A:%02X  X:%02X  Y:%02X  %s\n",
       ( unsigned )active_context->YA.b.l,
       ( unsigned )active_context->X,
       ( unsigned )active_context->YA.b.h,
       Message
       );
fprintf(
       stderr,
       "SPC R  0:%02X  1:%02X  2:%02X  3:%02X\n",
       ( unsigned )active_context->PORT_R[ 0 ],
       ( unsigned )active_context->PORT_R[ 1 ],
       ( unsigned )active_context->PORT_R[ 2 ],
       ( unsigned )active_context->PORT_R[ 3 ]
       );
fprintf(
       stderr,
       "SPC W  0:%02X  1:%02X  2:%02X  3:%02X\n",
       ( unsigned )active_context->PORT_W[ 0 ],
       ( unsigned )active_context->PORT_W[ 1 ],
       ( unsigned )active_context->PORT_W[ 2 ],
       ( unsigned )active_context->PORT_W[ 3 ]
       );
fprintf(
       stderr,
       "SPC counters:%1X %1X %1X targets:%02X %02X %02X CTRL:%02X\n",
       active_context->timers[ 0 ].counter,
       active_context->timers[ 1 ].counter,
       active_context->timers[ 2 ].counter,
       active_context->timers[ 0 ].target & 0xFF,
       active_context->timers[ 1 ].target & 0xFF,
       active_context->timers[ 2 ].target & 0xFF,
       SPC_CTRL
       );
fprintf(
       stderr,
       "Op: %02X (%02X %02X)\n",
       SPCRAM[ active_context->PC.w     ],
       SPCRAM[ active_context->PC.w + 1 ],
       SPCRAM[ active_context->PC.w + 2 ]
       );

}   /* DisplaySPC() */


/***** InvalidSPCOpcode *****/

void InvalidSPCOpcode
    ( void )
{
DisplaySPC();
fprintf(
       stderr,
       "Unemulated SPC opcode 0x%02X (%s)\n",
       ( unsigned )Map_Byte,
       SPC_OpID[ Map_Byte ]
       );
fprintf(
       stderr,
       "At address 0x%04X\n",
       ( unsigned )( Map_Address & 0xFFFF )
       );
exit( 1 );

}   /* InvalidSPCOpcode() */


/***** SPC_READ_DSP *****/

void SPC_READ_DSP
    ( void )
{

#ifdef DBG_DSP
fprintf(
       stderr,
       "Reading 0x%02X from DSP register 0x%02X\n",
       SPC_DSP[ SPCRAM[ 0xF2 ] ],
       SPCRAM[ 0xF2 ]
       );
#endif

#ifdef NO_ENVX
if( 8 == ( SPCRAM[ 0xF2 ] & 0xF ) )
    {
    SPC_DSP[ SPCRAM[ 0xF2 ] ] = 0;
    }
#endif

}   /* SPC_READ_DSP() */


/***** SPC_WRITE_DSP *****/

void SPC_WRITE_DSP
    ( void )
{
int                     addr;

addr = SPCRAM[ 0xF2 ];

#ifdef DBG_DSP
fprintf(
       stderr,
       "Writing 0x%02X to DSP register 0x%02X\n",
       SPC_DSP_DATA,
       addr
       );
#endif

if( 0x7C == addr )
    {
    DSP_WRITE_7C( SPC_DSP_DATA );
    }
else
    {
    SPC_DSP[ addr ] = SPC_DSP_DATA;
    }

}   /* _SPC_WRITE_DSP() */
