/************************************************************************

        Copyright (c) 2003 Brad Martin.

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



main.c: implements functions intended for external use of the libopenspc
library.

 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "openspc.h"
#include "dsp.h"
#include "SPCimpl.h"

#undef NO_CLEAR_ECHO

static int mix_left;

/**** Internal (static) functions ****/

static int Load_SPC(void *buf,size_t size)
{
    const char ident[]="SNES-SPC700 Sound File Data";
    struct SPC_FILE
    {
        unsigned char ident[37],
                      PCl,PCh,
                      A,
                      X,
                      Y,
                      P,
                      SP,
                      junk[212],
                      RAM[65536],
                      DSP[128];
    } *spc_file;
    if(size<sizeof(spc_file))
        return 1;
    spc_file=(struct SPC_FILE *)buf;
            
    if(memcmp(buf,ident,strlen(ident)))
        return 1;
    SPC_SetState(((int)spc_file->PCh<<8)+spc_file->PCl,spc_file->A,
     spc_file->X,spc_file->Y,spc_file->P,0x100+spc_file->SP,spc_file->RAM);
    memcpy(DSPregs,spc_file->DSP,128);
    return 0;
}

/**** Exported library interfaces ****/

int OSPC_Init(void *buf, size_t size)
{
    int ret;
#ifndef NO_CLEAR_ECHO
    int start,len;
#endif
    mix_left=0;
    SPC_Reset();
    DSP_Reset();
    ret=Load_SPC(buf,size);

/* New file formats could go on from here, for example:
    if(ret==1)
        ret=Load_FOO(buf,size);
    ...
*/
#ifndef NO_CLEAR_ECHO
    /* Because the emulator that generated the SPC file most likely did
       not correctly support echo, it is probably necessary to zero out
       the echo region of memory to prevent pops and clicks as playback
       begins. */
    if(!(DSPregs[0x6C]&0x20))
    {
        start=(unsigned char)DSPregs[0x6D]<<8;
        len=(unsigned char)DSPregs[0x7D]<<11;
        if(start+len>0x10000)
            len=0x10000-start;
        memset(&SPC_RAM[start],0,len);
    }
#endif
    return ret;
}

int OSPC_Run(int cyc, short *s_buf, int s_size)
{
    int i,buf_inc=s_buf?2:0;
    
    if((cyc<0)||((s_buf!=NULL)&&(cyc>=(s_size>>2)*TS_CYC+mix_left)))
    {   /* Buffer size is the limiting factor */
        s_size&=~3;
        if(mix_left)
            SPC_Run(mix_left);
        for(i=0;i<s_size;i+=4,s_buf+=buf_inc)
        {
            DSP_Update(s_buf);
            SPC_Run(TS_CYC);
        }
        mix_left=0;
        return s_size;
    }
    
    /* Otherwise, use the cycle count */
    if(cyc<mix_left)
    {
        SPC_Run(cyc);
        mix_left-=cyc;
        return 0;
    }
    if(mix_left)
    {
        SPC_Run(mix_left);
        cyc-=mix_left;
    }
    for(i=0;cyc>=TS_CYC;i+=4,cyc-=TS_CYC,s_buf+=buf_inc)
    {
        DSP_Update(s_buf);
        SPC_Run(TS_CYC);
    }
    if(cyc)
    {
        DSP_Update(s_buf);
        SPC_Run(cyc);
        mix_left=TS_CYC-cyc;
        i+=4;
    }
    return i;
}

void OSPC_WritePort0(char data)
{
    WritePort0(data);
}

void OSPC_WritePort1(char data)
{
    WritePort1(data);
}

void OSPC_WritePort2(char data)
{
    WritePort2(data);
}

void OSPC_WritePort3(char data)
{
    WritePort3(data);
}

char OSPC_ReadPort0(void)
{
    return ReadPort0();
}

char OSPC_ReadPort1(void)
{
    return ReadPort1();
}

char OSPC_ReadPort2(void)
{
    return ReadPort2();
}

char OSPC_ReadPort3(void)
{
    return ReadPort3();
}
