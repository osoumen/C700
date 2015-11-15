/**************************************************************************

        Copyright (c) 2005 Brad Martin.
        Some portions copyright (c) 1998-2005 Charles Bilyue'.

This file is part of OpenSPC.

sneese_spc.h: This file defines the interface between the SNEeSe SPC700
core and the associated wrapper files.  As the licensing rights for SNEeSe
are different from the rest of OpenSPC, none of the files in this directory
are LGPL.  Although this file was created by me (Brad Martin), it contains
some code derived from SNEeSe and therefore falls under its license.  See
the file 'LICENSE' in this directory for more information.

 **************************************************************************/

#if !defined( _SNEESE_SPC_H )
#define _SNEESE_SPC_H

/*========== DEFINES ==========*/

#define SPC_CTRL        ( SPCRAM[ 0xF1 ] )
#define SPC_DSP_ADDR    ( SPCRAM[ 0xF2 ] )

/*========== TYPES ==========*/

typedef union
    {
    unsigned short      w;
    struct
        {
        unsigned char   l;
        unsigned char   h;
        } b;
    } word_2b;

typedef struct
    {
    unsigned char       B_flag;
    unsigned char       C_flag;
    unsigned char       H_flag;
    unsigned char       I_flag;
    unsigned char       N_flag;
    unsigned char       P_flag;
    unsigned char       PSW;
    unsigned char       SP;
    unsigned char       V_flag;
    unsigned char       X;
    unsigned char       Z_flag;
    unsigned char       cycle;
    unsigned char       data;
    unsigned char       data2;
    unsigned char       opcode;
    unsigned char       offset;
    
    word_2b             PC;
    word_2b             YA;
    word_2b             address;
    word_2b             address2;
    word_2b             data16;
    word_2b             direct_page;
    
    unsigned            Cycles;
    void *              FFC0_Address;
    unsigned            TotalCycles;
    int                 WorkCycles;
    unsigned            last_cycles;

    unsigned char       PORT_R[ 4 ];
    unsigned char       PORT_W[ 4 ];
    struct
        {
        unsigned char   counter;
        short           position;
        short           target;
        unsigned        cycle_latch;
        } timers[ 4 ];
    } SPC700_CONTEXT;

/*========== VARIABLES ==========*/

/* SPCimpl.c variables */
extern unsigned char    In_CPU;
extern unsigned         Map_Address;
extern unsigned         Map_Byte;
extern unsigned         SPC_CPU_cycle_divisor;
extern unsigned         SPC_CPU_cycle_multiplicand;
extern unsigned         SPC_CPU_cycles;
extern unsigned         SPC_CPU_cycles_mul;
extern unsigned char    SPC_DSP[ 256 ];
extern unsigned         SPC_DSP_DATA;
extern unsigned char    SPCRAM[ 65536 ];
extern unsigned         sound_cycle_latch;

/* spc700.c variables */
extern SPC700_CONTEXT * active_context;

/*========== MACROS ==========*/

#define Wrap_SDSP_Cyclecounter()
#define update_sound()

/*========== PROCEDURES ==========*/

/* SPCimpl.c procedures */
void DisplaySPC
    ( void );

void InvalidSPCOpcode
    ( void );

void SPC_READ_DSP
    ( void );

void SPC_WRITE_DSP
    ( void );

/* spc700.c procedures */
void Reset_SPC
    ( void );

unsigned char SPC_READ_PORT_W
    (
    unsigned short      address
    );

void SPC_START
    (
    unsigned            cycles
    );

void SPC_WRITE_PORT_R
    (
    unsigned short      address,
    unsigned char       data
    );

unsigned char get_SPC_PSW
    ( void );

void spc_restore_flags
    ( void );

#endif
