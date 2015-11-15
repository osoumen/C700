/**************************************************************************

        Copyright (c) 2003-2005 Brad Martin.

This file is part of OpenSPC.

SPCimpl.h: This file is a bridge between the OpenSPC library and the
specific SPC core implementation (in this case, SNEeSe's).  As the licensing
rights for SNEeSe are different from the rest of OpenSPC, none of the files
in this directory are LGPL.  Although this file was created by me (Brad
Martin), it contains some code derived from SNEeSe and therefore falls under
its license.  See the file 'LICENSE' in this directory for more information.

 **************************************************************************/

#if !defined( _SPCIMPL_H )
#define _SPCIMPL_H

/*========== INCLUDES ==========*/

#include "sneese_spc.h"

/*========== DEFINES ==========*/

#define DSPregs         SPC_DSP
#define SPC_RAM         SPCRAM

/*========== MACROS ==========*/

#define ReadPort0()     SPC_READ_PORT_W( 0 )
#define ReadPort1()     SPC_READ_PORT_W( 1 )
#define ReadPort2()     SPC_READ_PORT_W( 2 )
#define ReadPort3()     SPC_READ_PORT_W( 3 )

#define SPC_Reset()                                                 \
    {                                                               \
    Reset_SPC();                                                    \
    SPC_CPU_cycle_multiplicand = 1;                                 \
    SPC_CPU_cycle_divisor      = 1;                                 \
    SPC_CPU_cycles_mul         = 0;                                 \
    }

#define SPC_Run( c )    SPC_START( c )

#define WritePort0( x ) SPC_WRITE_PORT_R( 0, x )
#define WritePort1( x ) SPC_WRITE_PORT_R( 1, x )
#define WritePort2( x ) SPC_WRITE_PORT_R( 2, x )
#define WritePort3( x ) SPC_WRITE_PORT_R( 3, x )

/*========== PROCEDURES ==========*/

void SPC_SetState
    (
    int                 pc,
    int                 a,
    int                 x,
    int                 y,
    int                 p,
    int                 sp,
    void *              ram
    );

#endif /* _SPCIMPL_H */
