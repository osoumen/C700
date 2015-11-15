/*

SNEeSe, an Open Source Super NES emulator.


Copyright (c) 1998-2005, Charles Bilyue'.
Portions copyright (c) 1998-2003, Brad Martin.
Portions copyright (c) 2003-2004, Daniel Horchner.
Portions copyright (c) 2004-2005, Nach. ( http://nsrt.edgeemu.com/ )
Unzip Technology, copyright (c) 1998 Gilles Vollant.
zlib Technology ( www.gzip.org/zlib/ ), Copyright (c) 1995-2003,
 Jean-loup Gailly ( jloup* *at* *gzip.org ) and Mark Adler
 ( madler* *at* *alumni.caltech.edu ).
JMA Technology, copyright (c) 2004-2005 NSRT Team. ( http://nsrt.edgeemu.com/ )
LZMA Technology, copyright (c) 2001-4 Igor Pavlov. ( http://www.7-zip.org )
Portions copyright (c) 2002 Andrea Mazzoleni. ( http://advancemame.sf.net )

This is free software.  See 'LICENSE' for details.
You must read and accept the license prior to use.

*/

#define SNEeSe_apu_spc700_c

#include "sneese_spc.h"

SPC700_CONTEXT primary_context;

SPC700_CONTEXT *active_context = &primary_context;

/*
  SNEeSe SPC700 CPU emulation core
  Originally written by Lee Hammerton in AT&T assembly
  Maintained/rewritten/ported to NASM by Charles Bilyue'
  Maintained/ported to C by Charles Bilyue'

  This file contains:
   CPU core info
   Reset
   Execution Loop
   Invalid Opcode Handler or Dispatcher
   Variable definitions (registers, cycle counters, etc.)
   CPU opcode emulation handlers
   CPU opcode handler table
   CPU opcode timing table

  CPU core info:
     A register             - active_context->YA.b.A
     Y register             - active_context->YA.b.Y
     YA register pair       - active_context->YA.w
     X register             - active_context->X
     Stack pointer          - active_context->SP
     Program Counter        - active_context->PC
     Processor status word  - active_context->PSW
     Individual flags       - N_flag, V_flag, P_flag, B_flag,
                               H_flag, I_flag, Z_flag, C_flag


  SPC timers
   SPC700 timing is not directly related to 65c816 timing, but for
    simplicity in emulation we act as if it is. SPC700 gets 5632
    cycles for every 118125 (21.47727..MHz) 5A22 cycles. Since the
    timers run at ~8KHz and ~64KHz and the CPU core runs at
    1.024Mhz, the timers are clocked as follows:
     1.024MHz / 8KHz  = 128 cycles    (Timers 0 and 1)
     1.024MHz / 64KHz = 16  cycles    (Timer 2)
*/

/* lots of #define's! */

/* These are the bits for flag set/clr operations */
#define SPC_FLAG_C 1        /* Carry */
#define SPC_FLAG_Z 2        /* Zero result */
#define SPC_FLAG_I 4        /* Interrupt Disable */
#define SPC_FLAG_H 8        /* Half-carry */
#define SPC_FLAG_B 0x10     /* Break */
#define SPC_FLAG_P 0x20     /* Page (direct page) */
#define SPC_FLAG_V 0x40     /* Overflow */
#define SPC_FLAG_N 0x80     /* Negative result */

#define SPC_FLAG_NZ (SPC_FLAG_N | SPC_FLAG_Z)
#define SPC_FLAG_NZC (SPC_FLAG_NZ | SPC_FLAG_C)
#define SPC_FLAG_NHZC (SPC_FLAG_NZC | SPC_FLAG_H)

#define _Cycles         (active_context->Cycles)
#define _last_cycles    (active_context->last_cycles)
#define _TotalCycles    (active_context->TotalCycles)
#define _WorkCycles     (active_context->WorkCycles)

#define _PORT_R         (active_context->PORT_R)
#define _PORT0R         (_PORT_R[0])
#define _PORT1R         (_PORT_R[1])
#define _PORT2R         (_PORT_R[2])
#define _PORT3R         (_PORT_R[3])

#define _PORT_W         (active_context->PORT_W)
#define _PORT0W         (_PORT_W[0])
#define _PORT1W         (_PORT_W[1])
#define _PORT2W         (_PORT_W[2])
#define _PORT3W         (_PORT_W[3])

#define _FFC0_Address   (active_context->FFC0_Address)

#define _PC             (active_context->PC.w)
#define _PCL            (active_context->PC.b.l)
#define _PCH            (active_context->PC.b.h)
#define _YA             (active_context->YA.w)
#define _A              (active_context->YA.b.l)
#define _Y              (active_context->YA.b.h)
#define _dp             (active_context->direct_page.w)
#define _direct_page    (active_context->direct_page.b.h)
#define _SP             (active_context->SP)
#define _X              (active_context->X)
#define _PSW            (active_context->PSW)

#define _cycle          (active_context->cycle)
#define _opcode         (active_context->opcode)
#define _data           (active_context->data)
#define _data2          (active_context->data2)
#define _data16         (active_context->data16.w)
#define _offset         (active_context->offset)
#define _address        (active_context->address.w)
#define _address_l      (active_context->address.b.l)
#define _address_h      (active_context->address.b.h)
#define _address2       (active_context->address2.w)
#define _address2_l     (active_context->address2.b.l)
#define _address2_h     (active_context->address2.b.h)

#define _N_flag         (active_context->N_flag)
#define _V_flag         (active_context->V_flag)
#define _P_flag         (active_context->P_flag)
#define _B_flag         (active_context->B_flag)
#define _H_flag         (active_context->H_flag)
#define _I_flag         (active_context->I_flag)
#define _Z_flag         (active_context->Z_flag)
#define _C_flag         (active_context->C_flag)

#define _timers         (active_context->timers)

/* bits used all over the core */
void set_flag_spc(unsigned char flag)
{
  if (flag & SPC_FLAG_N)
  {
    _N_flag = 0x80;
  }
  if (flag & SPC_FLAG_V)
  {
    _V_flag = 1;
  }
  if (flag & SPC_FLAG_P)
  {
    _P_flag = 1;
    _direct_page = 0x01;
  }
  if (flag & SPC_FLAG_B)
  {
    _B_flag = 1;
  }
  if (flag & SPC_FLAG_H)
  {
    _H_flag = 1;
  }
  if (flag & SPC_FLAG_I)
  {
    _I_flag = 1;
  }
  if (flag & SPC_FLAG_Z)
  {
    _Z_flag = 0;
  }
  if (flag & SPC_FLAG_C)
  {
    _C_flag = 1;
  }
}


void clr_flag_spc(unsigned char flag)
{
  if (flag & SPC_FLAG_N)
  {
    _N_flag = 0;
  }
  if (flag & SPC_FLAG_V)
  {
    _V_flag = 0;
  }
  if (flag & SPC_FLAG_P)
  {
    _P_flag = 0;
    _direct_page = 0x00;
  }
  if (flag & SPC_FLAG_B)
  {
    _B_flag = 0;
  }
  if (flag & SPC_FLAG_H)
  {
    _H_flag = 0;
  }
  if (flag & SPC_FLAG_I)
  {
    _I_flag = 0;
  }
  if (flag & SPC_FLAG_Z)
  {
    _Z_flag = 1;
  }
  if (flag & SPC_FLAG_C)
  {
    _C_flag = 0;
  }
}


void complement_carry_spc(void)
{
  _C_flag = !_C_flag;
}

unsigned char flag_state_spc(unsigned char flag)
{
  if (flag == SPC_FLAG_N)
  {
    return _N_flag & 0x80;
  }
  else if (flag == SPC_FLAG_V)
  {
    return _V_flag;
  }
  else if (flag == SPC_FLAG_P)
  {
    return _P_flag;
  }
  else if (flag == SPC_FLAG_B)
  {
    return _B_flag;
  }
  else if (flag == SPC_FLAG_H)
  {
    return _H_flag;
  }
  else if (flag == SPC_FLAG_I)
  {
    return _I_flag;
  }
  else if (flag == SPC_FLAG_Z)
  {
    return !_Z_flag;
  }
  else if (flag == SPC_FLAG_C)
  {
    return _C_flag;
  }

  return 0;
}


void load_cycles_spc(void)
{
  _WorkCycles = _TotalCycles - _Cycles;
}

unsigned get_cycles_spc(void)
{
  return _WorkCycles + _Cycles;
}

void save_cycles_spc(void)
{
  _TotalCycles = _WorkCycles + _Cycles;
}


/* Set up the flags from our flag format to SPC flag format */
void spc_setup_flags(int B_flag)
{
    unsigned char PSW = 0;

    PSW += _N_flag & 0x80;
    PSW += _V_flag ? 0x40 : 0;
    PSW += _P_flag ? 0x20 : 0;
    PSW += B_flag ? 0x10 : 0;
    PSW += _H_flag ? 0x08 : 0;
    PSW += _I_flag ? 0x04 : 0;
    PSW += !_Z_flag ? 0x02 : 0;
    PSW += _C_flag ? 0x01 : 0;

    _PSW = PSW;
}

/* Restore the flags from SPC flag format to our flag format */
void spc_restore_flags(void)
{
  unsigned char PSW = _PSW;

  _N_flag = PSW;
  _V_flag = PSW & SPC_FLAG_V;

  if (PSW & SPC_FLAG_P) set_flag_spc(SPC_FLAG_P);
  else clr_flag_spc(SPC_FLAG_P);

  _B_flag = PSW & SPC_FLAG_B;
  _H_flag = PSW & SPC_FLAG_H;
  _I_flag = PSW & SPC_FLAG_I;
  _Z_flag = ~PSW & SPC_FLAG_Z;
  _C_flag = PSW & SPC_FLAG_C;
}


void store_flag_n(unsigned char value)
{
 _N_flag = value;
}

void store_flag_v(unsigned char value)
{
 _V_flag = value;
}

void store_flag_p(unsigned char value)
{
 _P_flag = value;
  _direct_page = value ? 0x01 : 0x00;
}

void store_flag_h(unsigned char value)
{
 _H_flag = value;
}

void store_flag_i(unsigned char value)
{
 _I_flag = value;
}

void store_flag_z(unsigned char value)
{
 _Z_flag = value;
}

void store_flag_c(unsigned char value)
{
 _C_flag = value;
}


void store_flags_nz(unsigned char value)
{
 store_flag_n(value);
 store_flag_z(value);
}

void store_flags_nzc(unsigned char nz, unsigned char c)
{
 store_flag_n(nz);
 store_flag_z(nz);
 store_flag_c(c);
}


/* bits for external access by the 5A22 core */
unsigned char SPC_READ_PORT_W(unsigned short address)
{
  return _PORT_W[address & 3];
}

void SPC_WRITE_PORT_R(unsigned short address, unsigned char data)
{
  _PORT_R[address & 3] = data;
}


/* bits for handling cycle counter overflows */
void Wrap_SPC_Cyclecounter()
{
  _TotalCycles -= 0xF0000000;
  _Cycles -= 0xF0000000;
  _timers[0].cycle_latch -= 0xF0000000;
  _timers[1].cycle_latch -= 0xF0000000;
  _timers[2].cycle_latch -= 0xF0000000;

  Wrap_SDSP_Cyclecounter();
}


/* This code should be mapped into the top of the address space */
static unsigned char SPC_ROM_CODE[64] =
{
  0xCD,0xEF,0xBD,0xE8,0x00,0xC6,0x1D,0xD0,
  0xFC,0x8F,0xAA,0xF4,0x8F,0xBB,0xF5,0x78,
  0xCC,0xF4,0xD0,0xFB,0x2F,0x19,0xEB,0xF4,
  0xD0,0xFC,0x7E,0xF4,0xD0,0x0B,0xE4,0xF5,
  0xCB,0xF4,0xD7,0x00,0xFC,0xD0,0xF3,0xAB,
  0x01,0x10,0xEF,0x7E,0xF4,0x10,0xEB,0xBA,
  0xF6,0xDA,0x00,0xBA,0xF4,0xC4,0xF4,0xDD,
  0x5D,0xD0,0xDB,0x1F,0x00,0x00,0xC0,0xFF
};


static unsigned char SPC_READ_INVALID(unsigned short address)
{
#ifdef TRAP_INVALID_READ
#ifdef DEBUG
  /* Set up address so message works */
  Map_Address = address;
  Map_Byte = 0;

  InvalidSPCHWRead();   /* Display read from invalid HW warning */
#endif
#endif
  return 0;
}

static unsigned char SPC_READ_RAM(unsigned short address)
{
  return SPCRAM[address];
}

static unsigned char SPC_READ_DSP_DATA(unsigned short address)
{
  SPC_READ_DSP();

  /* read from DSP register */
  return SPC_DSP[SPC_DSP_ADDR];
}


unsigned char SPC_READ_PORT_R(unsigned short address)
{
  return _PORT_R[address & 3];
}


/*  timer registers are write-only, actual timer clock is internal and */
/* not accessible! */
/*  counters are 4-bit, upon read/write they reset to 0 */

void Update_SPC_Timer(int timer)
{
  unsigned shift, mask, cycles, position;

  if (timer != 2)
  {
    shift = 7;
  }
  else
  {
    shift = 4;
  }
  mask = -(1 << shift);

  cycles = _TotalCycles - _timers[timer].cycle_latch;
  _timers[timer].cycle_latch += cycles & mask;

  /* nothing to do if timer turned off */
  if (!(SPC_CTRL & (1 << timer))) return;

  position = _timers[timer].position + (cycles >> shift);
  _timers[timer].position = position;
  if (position < _timers[timer].target)
  {
    return;
  }

  _timers[timer].counter += position / _timers[timer].target;
  /* 4-bit counter without saturation */
  _timers[timer].counter &= 0x0F;

  _timers[timer].position = position % _timers[timer].target;
}

static unsigned char SPC_READ_COUNTER(unsigned short address)
{
  /* 0xFD = read address for first timer's counter */
  int timer = address - 0xFD;
  unsigned char counter; 

  Update_SPC_Timer(timer);
  counter = _timers[timer].counter;
  _timers[timer].counter = 0;

  return counter;
}


/*
 | ROMEN | ----- | PC32  | PC10  | ----- |  ST2  |  ST1  |  ST0  |

 ROMEN - enable mask ROM in top 64-bytes of address space for CPU read
 PC32  - clear SPC read ports 2 & 3
 PC10  - clear SPC read ports 0 & 1
 ST2   - start timer 2 (64kHz)
 ST1   - start timer 1 (8kHz)
 ST0   - start timer 0 (8kHz)
*/

void spc_start_timer(int timer)
{
  unsigned shift, mask;

  if (timer != 2)
  {
    shift = 7;
  }
  else
  {
    shift = 4;
  }
  mask = -(1 << shift);

  _timers[timer].cycle_latch = _TotalCycles & mask;
  _timers[timer].position = 0;
  _timers[timer].counter = 0;
}

static void SPC_WRITE_INVALID(unsigned short address, unsigned char data)
{
#ifdef TRAP_INVALID_WRITE
#ifdef DEBUG
  /* Set up address so message works */
  Map_Address = address;
  Map_Byte = data;

  InvalidSPCHWWrite();  /* Display write to invalid HW warning */
#endif
#endif
}

static void SPC_WRITE_CTRL(unsigned short address, unsigned char data)
{
  /* IPL ROM enable */
  _FFC0_Address = data & 0x80 ? SPC_ROM_CODE - 0xFFC0 : SPCRAM;

  /* read ports 0/1 reset */
  if (data & 0x10)
  {
    _PORT_R[0] = 0;
    _PORT_R[1] = 0;
  }

  /* read ports 2/3 reset */
  if (data & 0x20)
  {
    _PORT_R[2] = 0;
    _PORT_R[3] = 0;
  }

  /* timer 0 control */
  if (!(SPCRAM[address] & 1) && (data & 1))
  {
    spc_start_timer(0);
  }

  /* timer 0 control */
  if (!(SPCRAM[address] & 2) && (data & 2))
  {
    spc_start_timer(1);
  }

  /* timer 2 control */
  if (!(SPCRAM[address] & 4) && (data & 4))
  {
    spc_start_timer(2);
  }

  SPC_CTRL = data;
}


static void SPC_WRITE_RAM(unsigned short address, unsigned char data)
{
  SPCRAM[address] = data;
}

static void SPC_WRITE_DSP_DATA(unsigned short address, unsigned char data)
{
  SPC_DSP_DATA = data;

  /* write to DSP register */
  SPC_WRITE_DSP();
}


void SPC_WRITE_PORT_W(unsigned short address, unsigned char data)
{
  _PORT_W[address & 3] = data;
}

static void SPC_WRITE_TIMER(unsigned short address, unsigned char data)
{
  /* 0xFA = write address for first timer's target */
  int timer = address - 0xFA;
  int target;

  if ((_timers[timer].target & 0xFF) == data)
  {
    return;
  }

  target = data ? data : 256;

  /* Timer must catch up before changing target */
  Update_SPC_Timer(timer);

  _timers[timer].target = target;

  /* does setting target for current position raise counter? assuming not */
  if (target <= _timers[timer].position)
  /* handle 'delay' where new target is set below position */
  {
    _timers[timer].position -= 256;
  }
}


/* Mappings for SPC Registers */
static unsigned char (*Read_Func_Map[16])(unsigned short address) =
{
  SPC_READ_INVALID,
  SPC_READ_INVALID,
  SPC_READ_RAM,
  SPC_READ_DSP_DATA,
  SPC_READ_PORT_R,
  SPC_READ_PORT_R,
  SPC_READ_PORT_R,
  SPC_READ_PORT_R,
  SPC_READ_RAM,
  SPC_READ_RAM,
  SPC_READ_INVALID,
  SPC_READ_INVALID,
  SPC_READ_INVALID,
  SPC_READ_COUNTER,
  SPC_READ_COUNTER,
  SPC_READ_COUNTER
};

static void (*Write_Func_Map[16])(unsigned short address, unsigned char data) =
{
  SPC_WRITE_INVALID,
  SPC_WRITE_CTRL,
  SPC_WRITE_RAM,
  SPC_WRITE_DSP_DATA,
  SPC_WRITE_PORT_W,
  SPC_WRITE_PORT_W,
  SPC_WRITE_PORT_W,
  SPC_WRITE_PORT_W,
  SPC_WRITE_RAM,
  SPC_WRITE_RAM,
  SPC_WRITE_TIMER,
  SPC_WRITE_TIMER,
  SPC_WRITE_TIMER,
  SPC_WRITE_RAM,
  SPC_WRITE_RAM,
  SPC_WRITE_RAM
};

static unsigned char offset_to_bit[8] =
{
  0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80
};

static unsigned char offset_to_not[8] =
{
  0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F
};

unsigned char get_byte_spc(unsigned short address)
{
  /*  Note: need to update sound if echo write enabled and accessing echo */
  /* region */
  if (address >= 0x0100)
  /* not zero page */
  {
    if (address >= 0xFFC0)
    /* return ROM if it's mapped in, else RAM */
    {
      return ((unsigned char *) _FFC0_Address)[address];
    }
    /* return RAM */
    return SPCRAM[address];
  }

  /* zero page */
  if (address < 0xF0)
  /* RAM */
  {
    return SPCRAM[address];
  }

  save_cycles_spc();    /* Set cycle counter */
  return Read_Func_Map[address - 0xF0](address);
}

/* -------- */

void set_byte_spc(unsigned short address, unsigned char data)
{
  /* Note: need to update sound always, since all (?) writes affect RAM */
  if (address >= 0x0100 || address < 0xF0)
  /* write to RAM */
  {
    save_cycles_spc();    /* Set cycle counter */
    update_sound();
    SPCRAM[address] = data;
  }
  else
  {
    save_cycles_spc();    /* Set cycle counter */
    Write_Func_Map[address - 0xF0](address, data);
  }
}

//#define OPCODE_TRACE_LOG
#ifdef OPCODE_TRACE_LOG
FILE *log_file = 0;
#endif

void Reset_SPC(void)
{
  int i;

#ifdef OPCODE_TRACE_LOG
  if (!log_file) log_file = fopen("h:\\spc.log", "wb");
#endif

  /* Get ROM reset vector and setup Program Counter */
  _PC = SPC_ROM_CODE[0xFFFE - 0xFFC0] +
    (SPC_ROM_CODE[0xFFFF - 0xFFC0] << 8);

  /* Reset cycle counts */
  _TotalCycles = 6;     /* 5-7 cycles before execution begins */

  _Cycles = 0;
  _last_cycles = 0;

  _cycle = 0;

  /* Reset SSMP registers */
  _dp = 0;  /* Used to save P flag check for dp addressing */
  _SP = 0xEF;
  _YA = 0;
  _X = 0;
  /* Clear flags register */
  _PSW = 0;
  clr_flag_spc(SPC_FLAG_N);
  clr_flag_spc(SPC_FLAG_V);
  clr_flag_spc(SPC_FLAG_P);
  clr_flag_spc(SPC_FLAG_B);
  clr_flag_spc(SPC_FLAG_H);
  clr_flag_spc(SPC_FLAG_I);
  clr_flag_spc(SPC_FLAG_Z);
  clr_flag_spc(SPC_FLAG_C);

  SPC_CTRL = 0x80;
  _FFC0_Address = SPC_ROM_CODE - 0xFFC0;

  /* Reset timers */
  for (i = 0; i < 3; i++)
  {
    _timers[i].cycle_latch = 0;
    _timers[i].position = 0;
    _timers[i].target = 256;
    _timers[i].counter = 0;
  }

  sound_cycle_latch = 0;

  /* Reset SPC700 input ports */
  _PORT_R[0] = 0;
  _PORT_R[1] = 0;
  _PORT_R[2] = 0;
  _PORT_R[3] = 0;

  /* Reset SPC700 output ports */
  _PORT_W[0] = 0;
  _PORT_W[1] = 0;
  _PORT_W[2] = 0;
  _PORT_W[3] = 0;

  /* Reset sound DSP port address */
  SPC_DSP_ADDR = 0;
  SPC_DSP_DATA = 0;
}


void SPC_SHOW_REGISTERS(void)
{
  DisplaySPC();
}

unsigned char get_SPC_PSW(void)
{
  spc_setup_flags(_B_flag);

  return _PSW;
}

#ifdef OPCODE_TRACE_LOG
static int dump_flag = 1;

#define SS_WAIT_FOR_KEY /*if ((readkey() & 0xFF) == 'g') { int i = 0; while (i++ < 49) simulate_keypress(' ' + (KEY_SPACE << 8)); }*/

#else
#define SS_WAIT_FOR_KEY

void dummy_fprintf()
{
}
#define fprintf dummy_fprintf

#endif


#ifdef OPCODE_TRACE_LOG
/* cycle #, PC, TotalCycles */
#define SINGLE_STEP_START(c) \
  if (dump_flag) fprintf(log_file, "START_CYCLE(%u) PC:%04X %u\n",  c, _PC & 0xFFFF, get_cycles_spc());

void single_step_end(void)
{
  if (!dump_flag) return;
  fprintf(log_file, "R:%02X %02X %02X %02X W:%02X %02X %02X %02X X:%02X Y:%02X A:%02X SP:%02X dp:%02X Op:%02X\n",
    _PORT0R & 0xFF, _PORT1R & 0xFF, _PORT2R & 0xFF, _PORT3R & 0xFF,
    _PORT0W & 0xFF, _PORT1W & 0xFF, _PORT2W & 0xFF, _PORT3W & 0xFF,
    _X & 0xFF, _Y & 0xFF, _A & 0xFF, _SP & 0xFF, _direct_page & 0xFF, _opcode & 0xFF);
  fprintf(log_file, "Ad%04X %04X Off%02X D%02X %02X D16 %04X NVPBHIZC %c%c%c%c%c%c%c%c",
    _address & 0xFFFF, _address2 & 0xFFFF, _offset & 0xFF, _data & 0xFF, _data2 & 0xFF,
    _data16 & 0xFFFF,
    flag_state_spc(SPC_FLAG_N) ? '1' : '0', flag_state_spc(SPC_FLAG_V) ? '1' : '0',
    flag_state_spc(SPC_FLAG_P) ? '1' : '0', '1',
    flag_state_spc(SPC_FLAG_H) ? '1' : '0', flag_state_spc(SPC_FLAG_I) ? '1' : '0',
    flag_state_spc(SPC_FLAG_Z) ? '1' : '0', flag_state_spc(SPC_FLAG_C) ? '1' : '0');
 if (_cycle == 0) fprintf(log_file, " %s\n", SPC_OpID[_opcode]);
 else fprintf(log_file, "\n");
}


/* op, R ports, W ports, X Y A */
/* address1 address2 offset data1 data2 data16 */
#define SINGLE_STEP_END single_step_end();

#else
#define SINGLE_STEP_START(c)
#define SINGLE_STEP_END
#endif


#define START_CYCLE(c) if (_cycle <= ((c) - 1)) { SINGLE_STEP_START(c)

#define END_FETCH_CYCLE() \
  _WorkCycles++; SINGLE_STEP_END \
  if (_WorkCycles >= 0) { _cycle = 1; opcode_done = 0; break; } \
}

#define END_CYCLE(c,n) \
  _WorkCycles += n; SINGLE_STEP_END \
  if (_WorkCycles >= 0) { _cycle = c; opcode_done = 0; break; } \
}

#define EXIT_OPCODE(n) { _WorkCycles += n; SINGLE_STEP_END break; }

#define END_OPCODE(n) EXIT_OPCODE(n) }

#define END_BRANCH_OPCODE(cycle, TEST) \
  TEST \
  END_CYCLE((cycle), 1) \
 \
  /* sign extend offset and add to PC */ \
  START_CYCLE((cycle) + 1) \
  _address = _PC + (((int) _offset ^ 0x80) - 0x80); \
  END_CYCLE((cycle) + 1, 1) \
 \
  START_CYCLE((cycle) + 2) \
  _PC = _address; \
  END_OPCODE(1)


#define REL_TEST_BRA ;
#define REL_TEST_BPL if (flag_state_spc(SPC_FLAG_N)) EXIT_OPCODE(1)
#define REL_TEST_BMI if (!flag_state_spc(SPC_FLAG_N)) EXIT_OPCODE(1)
#define REL_TEST_BVC if (flag_state_spc(SPC_FLAG_V)) EXIT_OPCODE(1)
#define REL_TEST_BVS if (!flag_state_spc(SPC_FLAG_V)) EXIT_OPCODE(1)
#define REL_TEST_BCC if (flag_state_spc(SPC_FLAG_C)) EXIT_OPCODE(1)
#define REL_TEST_BCS if (!flag_state_spc(SPC_FLAG_C)) EXIT_OPCODE(1)
#define REL_TEST_BNE if (flag_state_spc(SPC_FLAG_Z)) EXIT_OPCODE(1)
#define REL_TEST_BEQ if (!flag_state_spc(SPC_FLAG_Z)) EXIT_OPCODE(1)

#define OP_TCALL(vector) \
  /*  8 cycles - opcode, new PCL, new PCH, stack address load, dummy */ \
  /* cycle (PSW write in BRK?), PCH write, PCL write, */ \
  /* SP decrement */ \
  /* fetch address for PC */ \
  START_CYCLE(2) \
  _address = 0xFFC0 + ((15 - (vector)) * 2); \
  _address2 = get_byte_spc(_address); \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address2 += (get_byte_spc(_address + 1) << 8); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _address = 0x0100 + _SP; \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  /* should we write PSW to stack here? */ \
  END_CYCLE(5, 1) \
 \
  START_CYCLE(6) \
  set_byte_spc(_address, _PC >> 8); \
  _SP--; \
  _address = 0x0100 + _SP; \
  END_CYCLE(6, 1) \
 \
  START_CYCLE(7) \
  set_byte_spc(_address, _PC); \
  END_CYCLE(7, 1) \
 \
  START_CYCLE(8) \
  _PC = _address2; \
  _SP--; \
  END_OPCODE(1)


#define COND_REL(TEST) \
  /*  2 cycles - opcode, branch logic + offset; */ \
  /* +2 cycles (taken branch) add PC to offset, reload PC */ \
 \
  START_CYCLE(2) \
  _offset = get_byte_spc(_PC); \
  _PC++; \
  END_BRANCH_OPCODE(2, TEST)


#define DP_REL_TEST_BBS \
  if (!(_data & offset_to_bit[_opcode >> 5])) EXIT_OPCODE(1)

#define DP_REL_TEST_BBC \
  if ((_data & offset_to_bit[_opcode >> 5])) EXIT_OPCODE(1)

#define TEST_CBNE \
  if (_data == _A) EXIT_OPCODE(1)

#define DP_REL_TEST_DBNZ \
  --_data; \
  set_byte_spc(_address, _data); \
  if (!_data) EXIT_OPCODE(1)

#define COND_DP_REL(TEST) \
  /*  5 cycles - opcode, address, branch offset, data read, branch logic */ \
  /* and data write (DBNZ only); +2 cycles (taken branch) add PC to */ \
  /* offset, reload PC */ \
  START_CYCLE(2) \
  _address = _dp + get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _offset = get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _data = get_byte_spc(_address); \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  END_BRANCH_OPCODE(5, TEST)


#define OP_READ_DP(OP,dest) \
  /*  3 cycles - opcode, address, data read + op */ \
  START_CYCLE(2) \
  _address = _dp + get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _data = get_byte_spc(_address); \
  OP_##OP((dest), _data) \
  END_OPCODE(1)

/* xxx00100 */
#define OP_READ_DP_A(OP) OP_READ_DP(OP, _A)


#define OP_READ_ABS(OP,dest) \
  /*  4 cycles - opcode, address low, address high, data read + op */ \
  START_CYCLE(2) \
  _address = get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address += get_byte_spc(_PC) << 8; \
  _PC++; \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _data = get_byte_spc(_address); \
  OP_##OP((dest), _data) \
  END_OPCODE(1)

/* xxx00101 */
#define OP_READ_ABS_A(OP) OP_READ_ABS(OP, _A)


#define OP_READ_INDIRECT(OP,dest) \
  /*  3 cycles - opcode, address calc, data read + op */ \
  START_CYCLE(2) \
  _address = _dp + _X; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _data = get_byte_spc(_address); \
  OP_##OP(_A, _data) \
  END_OPCODE(1)

#define OP_RMW_INDIRECT(OP,NEED_OLD_DATA) \
  /*  4 cycles - opcode, address calc, data read, data write */ \
  START_CYCLE(2) \
  _address = _dp + _X; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  if (NEED_OLD_DATA) _data = get_byte_spc(_address); \
  else get_byte_spc(_address); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  OP \
  END_OPCODE(1)

/* xxx00110 */
#define OP_READ_INDIRECT_A(OP) OP_READ_INDIRECT(OP, _A)


#define OP_READ_INDEXED_INDIRECT(OP,dest) \
  /*  6 cycles - opcode, offset, address calc, address low, */ \
  /* address high, data read + op */ \
  START_CYCLE(2) \
  _address2 = get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address2 = _dp + ((_address2 + _X) & 0xFF); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _address = get_byte_spc(_address2); \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  _address += get_byte_spc(_address2 + 1) << 8; \
  END_CYCLE(5, 1) \
 \
  START_CYCLE(6) \
  _data = get_byte_spc(_address); \
  OP_##OP((dest), _data) \
  END_OPCODE(1)

#define OP_RMW_INDEXED_INDIRECT(OP,NEED_OLD_DATA) \
  /*  7 cycles - opcode, offset, address calc, address low, */ \
  /* address high, data read, data read + op */ \
  START_CYCLE(2) \
  _address2 = get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address2 = _dp + ((_address2 + _X) & 0xFF); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _address = get_byte_spc(_address2); \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  _address += get_byte_spc(_address2 + 1) << 8; \
  END_CYCLE(5, 1) \
 \
  START_CYCLE(6) \
  if (NEED_OLD_DATA) _data = get_byte_spc(_address); \
  else get_byte_spc(_address); \
  END_CYCLE(6, 1) \
 \
  START_CYCLE(7) \
  OP \
  END_OPCODE(1)

/* xxx00111 */
#define OP_READ_INDEXED_INDIRECT_A(OP) OP_READ_INDEXED_INDIRECT(OP, _A)


#define OP_READ_IMM(OP,dest) \
  /*  2 cycles - opcode, data read + op */ \
  START_CYCLE(2) \
  _data = get_byte_spc(_PC); \
  _PC++; \
  OP_##OP((dest), _data) \
  END_OPCODE(1)

/* xxx01000 */
#define OP_READ_IMM_A(OP) OP_READ_IMM(OP, _A)


#define OP_OR(dest,src) \
  { \
    (dest) |= (src); \
    store_flags_nz(dest); \
  }

#define OP_AND(dest,src) \
  { \
  (dest) &= (src); \
  store_flags_nz(dest); \
  }

#define OP_EOR(dest,src) \
  { \
  (dest) ^= (src); \
  store_flags_nz(dest); \
  }

#define OP_CMP(dest,src) \
  { \
    unsigned temp = (dest) - (src); \
 \
    store_flag_c(temp <= 0xFF); \
    store_flags_nz(temp); \
  }

#define OP_ADC(dest,src) \
  { \
    unsigned result = (dest) + (src) + (flag_state_spc(SPC_FLAG_C) ? 1 : 0); \
 \
    store_flag_h(((unsigned) (((dest) & 0x0F) + ((src) & 0x0F) + \
      (flag_state_spc(SPC_FLAG_C) ? 1 : 0))) > 0x0F ? 1 : 0); \
    store_flag_c(result > 0xFF); \
    store_flag_v((~((dest) ^ (src))) & (((dest) ^ result) & 0x80)); \
    store_flags_nz(result); \
    (dest) = result; \
  }

#define OP_SBC(dest,src) \
  { \
    unsigned result = (dest) - (src) - (flag_state_spc(SPC_FLAG_C) ? 0 : 1); \
 \
    store_flag_h(((unsigned) (((dest) & 0x0F) - ((src) & 0x0F) - \
      (flag_state_spc(SPC_FLAG_C) ? 0 : 1))) > 0x0F ? 0 : 1); \
    store_flag_c(result <= 0xFF); \
    store_flag_v(((dest) ^ (src)) & (((dest) ^ result) & 0x80)); \
    store_flags_nz(result); \
    (dest) = result; \
  }

#define OP_MOV_READ_NOFLAGS(dest,src) \
  { \
    (dest) = (src); \
  }

#define OP_MOV_READ(dest,src) \
  { \
    (dest) = (src); \
    store_flags_nz(src); \
  }


#define OP_ADDW(dest,src) \
  { \
    unsigned temp_low, carry_low, temp_high, result; \
 \
    temp_low = ((dest) & 0xFF) + ((src) & 0xFF); \
    carry_low = temp_low > 0xFF ? 1 : 0; \
 \
    store_flag_h(((unsigned) ((((dest) >> 8) & 0x0F) + \
      (((src) >> 8) & 0x0F) + carry_low)) > 0x0F ? 1 : 0); \
 \
    temp_high = ((dest) >> 8) + ((src) >> 8) + carry_low; \
    store_flag_c(temp_high > 0xFF); \
    result = ((temp_low & 0xFF) + (temp_high << 8)) & 0xFFFF; \
 \
    store_flag_v(((~((dest) ^ (src))) & (((dest) ^ result) & 0x8000)) >> 8); \
    store_flag_n(result >> 8); store_flag_z(result != 0); \
    (dest) = result; \
  }

#define OP_SUBW(dest,src) \
  { \
    unsigned temp_low, carry_low, temp_high, result; \
 \
    temp_low = ((dest) & 0xFF) - ((src) & 0xFF); \
    carry_low = temp_low > 0xFF ? 1 : 0; \
 \
    store_flag_h(((unsigned) ((((dest) >> 8) & 0x0F) - \
      (((src) >> 8) & 0x0F) - carry_low)) > 0x0F ? 0 : 1); \
 \
    temp_high = ((dest) >> 8) - ((src) >> 8) - carry_low; \
    store_flag_c(temp_high <= 0xFF); \
    result = ((temp_low & 0xFF) + (temp_high << 8)) & 0xFFFF; \
 \
    store_flag_v((((dest) ^ (src)) & (((dest) ^ result) & 0x8000)) >> 8); \
    store_flag_n(result >> 8); store_flag_z(result != 0); \
    (dest) = result; \
  }

#define OP_MOVW_READ(dest,src) \
  { \
    (dest) = (src); \
    store_flag_n((dest) >> 8); \
    store_flag_z((dest) != 0); \
  }


#define OP_ASL(var) \
  { \
    store_flag_c((var) & 0x80); \
    (var) <<= 1; \
    store_flags_nz(var); \
  }

#define OP_ROL(var) \
  { \
    int c = flag_state_spc(SPC_FLAG_C) ? 1 : 0; \
 \
    store_flag_c((var) & 0x80); \
    (var) = ((var) << 1) + c; \
    store_flags_nz(var); \
  }

#define OP_LSR(var) \
  { \
    store_flag_c((var) & 1); \
    (var) >>= 1; \
    store_flags_nz(var); \
  }

#define OP_ROR(var) \
  { \
    int c = flag_state_spc(SPC_FLAG_C) ? 0x80 : 0; \
 \
    store_flag_c((var) & 1); \
    (var) = ((var) >> 1) + c; \
    store_flags_nz(var); \
  }


#define OP_DECW(var) \
  { \
    (var)--; \
    store_flag_n(var >> 8); \
    store_flag_z((var & 0xFFFF) != 0); \
  }

#define OP_INCW(var) \
  { \
    (var)++; \
    store_flag_n(var >> 8); \
    store_flag_z((var & 0xFFFF) != 0); \
  }


#define OP_DEC(var) \
  { \
    (var)--; \
    store_flags_nz(var); \
  }

#define OP_INC(var) \
  { \
    (var)++; \
    store_flags_nz(var); \
  }


#define OP_SET1(var) \
  { \
    (var) |= offset_to_bit[_opcode >> 5]; \
  }
#define OP_CLR1(var) \
  { \
    (var) &= offset_to_not[_opcode >> 5]; \
  }

#define WRITE_OP(OP) OP_##OP(_data); set_byte_spc(_address, _data);
#define WRITE_MOV(var) set_byte_spc(_address, (var));


#define WRITE_ASL \
    OP_ASL(_data); set_byte_spc(_address, _data);
#define WRITE_LSR \
    OP_LSR(_data); set_byte_spc(_address, _data);
#define WRITE_ROL \
    OP_ROL(_data); set_byte_spc(_address, _data);
#define WRITE_ROR \
    OP_ROR(_data); set_byte_spc(_address, _data);

#define WRITE_DEC \
    OP_DEC(_data); set_byte_spc(_address, _data);
#define WRITE_INC \
    OP_INC(_data); set_byte_spc(_address, _data);


/* xxx01011 */
#define OP_RMW_DP(OP,NEED_OLD_DATA) \
  /*  4 cycles - opcode, address, data read, op + data write */ \
  START_CYCLE(2) \
  _address = _dp + get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  if (NEED_OLD_DATA) _data = get_byte_spc(_address); \
  else get_byte_spc(_address); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  OP \
  END_OPCODE(1)

/* xxx01001 */
#define OP_RMW_DP_DP(OP) \
  /*  6 cycles - opcode, src address, dest address, src read, */ \
  /* dest read + op, dest write */ \
  START_CYCLE(2) \
  _address2 = _dp + get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address = _dp + get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _data2 = get_byte_spc(_address2); \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  _data = get_byte_spc(_address); \
  OP_##OP(_data, _data2) \
  END_CYCLE(5, 1) \
 \
  START_CYCLE(6) \
  set_byte_spc(_address, _data); \
  END_OPCODE(1)

/* xxx01100 */
#define OP_RMW_ABS(OP,NEED_OLD_DATA) \
  /*  5 cycles - opcode, address low, address high, data read, */ \
  /* op + data write */ \
  START_CYCLE(2) \
  _address = get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address += get_byte_spc(_PC) << 8; \
  _PC++; \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  if (NEED_OLD_DATA) _data = get_byte_spc(_address); \
  else get_byte_spc(_address); \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  OP \
  END_OPCODE(1)


/* 0xx01101 */
#define OP_PUSH(src) \
  { \
    /*  4 cycles - opcode, address load, data write, SP decrement */ \
    START_CYCLE(2) \
    _address = 0x0100 + _SP; \
    END_CYCLE(2, 1) \
 \
    START_CYCLE(3) \
    set_byte_spc(_address, src); \
    END_CYCLE(3, 1) \
 \
    START_CYCLE(4) \
    _SP--; \
    END_OPCODE(1) \
  }

/* 1xx01110 */
#define OP_POP(dest) \
  { \
    /*  4 cycles - opcode, SP increment, address load, data read */ \
    START_CYCLE(2) \
    _SP++; \
    END_CYCLE(2, 1) \
 \
    START_CYCLE(3) \
    _address = 0x0100 + _SP; \
    END_CYCLE(3, 1) \
 \
    START_CYCLE(4) \
    (dest) = get_byte_spc(_address); \
    END_OPCODE(1) \
  }


#define OP_READ_DP_reg_INDEXED(OP,dest,index) \
  /*  4 cycles - opcode, address, address calc, data read + op */ \
  START_CYCLE(2) \
  _address = get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address = _dp + ((_address + index) & 0xFF); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _data = get_byte_spc(_address); \
  OP_##OP((dest), _data) \
  END_OPCODE(1)

#define OP_RMW_DP_reg_INDEXED(OP,NEED_OLD_DATA,index) \
  /*  5 cycles - opcode, address, address calc, data read, */ \
  /* data write */ \
  START_CYCLE(2) \
  _address = get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address = _dp + ((_address + index) & 0xFF); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  if (NEED_OLD_DATA) _data = get_byte_spc(_address); \
  else get_byte_spc(_address); \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  OP \
  END_OPCODE(1)


/* xxx10100 */
#define OP_READ_DP_X_INDEXED(OP,dest) OP_READ_DP_reg_INDEXED(OP,dest, _X)
#define OP_READ_DP_X_INDEXED_A(OP) OP_READ_DP_X_INDEXED(OP, _A)
/* xxx11011 */
#define OP_RMW_DP_X_INDEXED(OP,NEED_OLD_DATA) \
  OP_RMW_DP_reg_INDEXED(OP,NEED_OLD_DATA, _X)

/* xxx11001 */
#define OP_READ_DP_Y_INDEXED(OP,dest) OP_READ_DP_reg_INDEXED(OP,dest, _Y)
#define OP_RMW_DP_Y_INDEXED(OP,NEED_OLD_DATA) \
  OP_RMW_DP_reg_INDEXED(OP,NEED_OLD_DATA, _Y)


#define OP_READ_ABS_reg_INDEXED(OP,dest,index) \
  /*  5 cycles - opcode, address low, address high, address calc, */ \
  /* data read + op */ \
  START_CYCLE(2) \
  _address = get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address += get_byte_spc(_PC) << 8; \
  _PC++; \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _address += index; \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  _data = get_byte_spc(_address); \
  OP_##OP((dest), _data) \
  END_OPCODE(1)

#define OP_RMW_ABS_reg_INDEXED(OP,NEED_OLD_DATA,index) \
  /*  6 cycles - opcode, address low, address high, address calc, */ \
  /* data read, data write */ \
  START_CYCLE(2) \
  _address = get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address += get_byte_spc(_PC) << 8; \
  _PC++; \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _address += index; \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  if (NEED_OLD_DATA) _data = get_byte_spc(_address); \
  else get_byte_spc(_address); \
  END_CYCLE(5, 1) \
 \
  START_CYCLE(6) \
  OP \
  END_OPCODE(1)

/* xxx10101 */
#define OP_READ_ABS_X_INDEXED_A(OP) OP_READ_ABS_reg_INDEXED(OP, _A, _X)
#define OP_RMW_ABS_X_INDEXED(OP,NEED_OLD_DATA) \
  OP_RMW_ABS_reg_INDEXED(OP,NEED_OLD_DATA, _X)

/* xxx10110 */
#define OP_READ_ABS_Y_INDEXED_A(OP) OP_READ_ABS_reg_INDEXED(OP, _A, _Y)
#define OP_RMW_ABS_Y_INDEXED(OP,NEED_OLD_DATA) \
  OP_RMW_ABS_reg_INDEXED(OP,NEED_OLD_DATA, _Y)


#define OP_READ_INDIRECT_INDEXED(OP,dest) \
  /*  6 cycles - opcode, offset, address low, address high, */ \
  /* address calc, data read + op */ \
  START_CYCLE(2) \
  _address2 = _dp + get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address = get_byte_spc(_address2); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _address += get_byte_spc(_address2 + 1) << 8; \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  _address += _Y; \
  END_CYCLE(5, 1) \
 \
  START_CYCLE(6) \
  _data = get_byte_spc(_address); \
  OP_##OP((dest), _data) \
  END_OPCODE(1)

#define OP_RMW_INDIRECT_INDEXED(OP,NEED_OLD_DATA) \
  /*  7 cycles - opcode, offset, address low, address high, */ \
  /* address calc, data read + op, data write */ \
  START_CYCLE(2) \
  _address2 = _dp + get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address = get_byte_spc(_address2); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _address += get_byte_spc(_address2 + 1) << 8; \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  _address += _Y; \
  END_CYCLE(5, 1) \
 \
  START_CYCLE(6) \
  if (NEED_OLD_DATA) _data = get_byte_spc(_address); \
  else get_byte_spc(_address); \
  END_CYCLE(6, 1) \
 \
  START_CYCLE(7) \
  OP \
  END_OPCODE(1)

/* xxx10111 */
#define OP_READ_INDIRECT_INDEXED_A(OP) OP_READ_INDIRECT_INDEXED(OP, _A)


/* xxx11000 */
#define OP_RMW_DP_IMM(OP) \
  /*  5 cycles - opcode, src data, dest address, dest read + op, */ \
  /* dest write */ \
  START_CYCLE(2) \
  _data2 = get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _address = _dp + get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _data = get_byte_spc(_address); \
  OP_##OP(_data, _data2) \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  set_byte_spc(_address, _data); \
  END_OPCODE(1)


/* xxx11001 */
#define OP_RMW_INDIRECT_INDIRECT(OP) \
  /*  5 cycles - opcode, address calc, src read, dest read + op, */ \
  /* dest write */ \
  START_CYCLE(2) \
  _address = _dp + _Y; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _data2 = get_byte_spc(_address); \
  _address = _dp + _X; \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _data = get_byte_spc(_address); \
  OP_##OP(_data, _data2) \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  set_byte_spc(_address, _data); \
  END_OPCODE(1)


/*  note - timing on all 16-bit opcodes may be off: RMW could be instead: */
/* data low read, write, data high read, write; or data low read, */
/* data high read, data low write, data high write */

/* xxx11010 */
#define OP_READ16_YA_DP(OP) \
  /*  5 cycles - opcode, address, data low read, data high read + op, */ \
  /* (?) */ \
  START_CYCLE(2) \
  _address = _dp + get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _data16 = get_byte_spc(_address); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _data16 += get_byte_spc(_address + 1) << 8; \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  OP_##OP(_YA, _data16) \
  END_OPCODE(1)

#define OP_RMW16_DP(OP) \
  /*  6 cycles - opcode, address, data low read, data high read + op, */ \
  /* data low (?) write, data high write */ \
  START_CYCLE(2) \
  _address = _dp + get_byte_spc(_PC); \
  _PC++; \
  END_CYCLE(2, 1) \
 \
  START_CYCLE(3) \
  _data16 = get_byte_spc(_address); \
  END_CYCLE(3, 1) \
 \
  START_CYCLE(4) \
  _data16 += get_byte_spc(_address + 1) << 8; \
  END_CYCLE(4, 1) \
 \
  START_CYCLE(5) \
  OP_##OP(_data16) \
  set_byte_spc(_address, _data16); \
  END_CYCLE(5, 1) \
 \
  START_CYCLE(6) \
  set_byte_spc(_address + 1, _data16 >> 8); \
  END_OPCODE(1)


/* xxx11100 */
#define OP_RMW_IMPLIED(OP,reg) \
  /*  2 cycles - opcode, op */ \
  START_CYCLE(2) \
  OP_##OP(reg); \
  END_OPCODE(1)


/* xxx11101 */
#define OP_MOV_IMPLIED(dest,src) \
  /*  2 cycles - opcode, op */ \
  START_CYCLE(2) \
  OP_MOV_READ(dest,src) \
  END_OPCODE(1)

#define OP_MOV_IMPLIED_NO_FLAGS(dest,src) \
  /*  2 cycles - opcode, op */ \
  START_CYCLE(2) \
  (dest) = (src); \
  END_OPCODE(1)


static void Execute_SPC(void)
{
  unsigned char was_in_cpu = In_CPU;
  In_CPU = 0;

  load_cycles_spc();
  
  while (_WorkCycles < 0)
  {
    int opcode_done = 1;

    START_CYCLE(1)
      /* fetch opcode */
      _opcode = get_byte_spc(_PC);
      _PC++;
    END_FETCH_CYCLE()

    switch (_opcode)
    {
    /* xxx00000 */
    case 0x00:  /* NOP */
      {
        START_CYCLE(2)
        END_OPCODE(1)
      }

    case 0x20:  /* CLRP */
      {
        START_CYCLE(2)
        clr_flag_spc(SPC_FLAG_P);
        END_OPCODE(1)
      }

    case 0x40:  /* SETP */
      {
        START_CYCLE(2)
        set_flag_spc(SPC_FLAG_P);
        END_OPCODE(1)
      }

    case 0x60:  /* CLRC */
      {
        START_CYCLE(2)
        clr_flag_spc(SPC_FLAG_C);
        END_OPCODE(1)
      }

    case 0x80:  /* SETC */
      {
        START_CYCLE(2)
        set_flag_spc(SPC_FLAG_C);
        END_OPCODE(1)
      }

    case 0xA0:  /* EI */
      {
        START_CYCLE(2)
        set_flag_spc(SPC_FLAG_I);
        END_OPCODE(1)
      }

    case 0xC0:  /* DI */
      {
        START_CYCLE(2)
        clr_flag_spc(SPC_FLAG_I);
        END_OPCODE(1)
      }

    case 0xE0:  /* CLRV */
      {
        START_CYCLE(2)
        clr_flag_spc(SPC_FLAG_H | SPC_FLAG_V);
        END_OPCODE(1)
      }


    /* xxxx0001 */
#define opcode_TCALL(vector) (((vector) << 4) + 0x01)
    case opcode_TCALL( 0): case opcode_TCALL( 1):
    case opcode_TCALL( 2): case opcode_TCALL( 3):
    case opcode_TCALL( 4): case opcode_TCALL( 5):
    case opcode_TCALL( 6): case opcode_TCALL( 7):
    case opcode_TCALL( 8): case opcode_TCALL( 9):
    case opcode_TCALL(10): case opcode_TCALL(11):
    case opcode_TCALL(12): case opcode_TCALL(13):
    case opcode_TCALL(14): case opcode_TCALL(15):
      {
        OP_TCALL(_opcode >> 4)
      }


    /* xxx00010 */
#define opcode_SET1(bit) (((bit) << 5) + 0x02)
    case opcode_SET1(0): case opcode_SET1(1):
    case opcode_SET1(2): case opcode_SET1(3):
    case opcode_SET1(4): case opcode_SET1(5):
    case opcode_SET1(6): case opcode_SET1(7):
      {
        OP_RMW_DP(WRITE_OP(SET1), 1)
      }

    /* xxx00011 */
#define opcode_BBS(bit) (((bit) << 5) + 0x03)
    case opcode_BBS(0): case opcode_BBS(1):
    case opcode_BBS(2): case opcode_BBS(3):
    case opcode_BBS(4): case opcode_BBS(5):
    case opcode_BBS(6): case opcode_BBS(7):
      {
        COND_DP_REL(DP_REL_TEST_BBS)
      }

    /* xxx00100 */
    case 0x04:  /* OR A,dp */
      {
        OP_READ_DP_A(OR)
      }

    case 0x24:  /* AND A,dp */
      {
        OP_READ_DP_A(AND)
      }

    case 0x44:  /* EOR A,dp */
      {
        OP_READ_DP_A(EOR)
      }

    case 0x64:  /* CMP A,dp */
      {
        OP_READ_DP_A(CMP)
      }

    case 0x84:  /* ADC A,dp */
      {
        OP_READ_DP_A(ADC)
      }

    case 0xA4:  /* SBC A,dp */
      {
        OP_READ_DP_A(SBC)
      }

    case 0xC4:  /* MOV dp,A */
      {
        OP_RMW_DP(WRITE_MOV(_A), 0)
      }

    case 0xE4:  /* MOV A,dp */
      {
        OP_READ_DP_A(MOV_READ)
      }


    /* xxx00101 */
    case 0x05:  /* OR A,abs */
      {
        OP_READ_ABS_A(OR)
      }

    case 0x25:  /* AND A,abs */
      {
        OP_READ_ABS_A(AND)
      }

    case 0x45:  /* EOR A,abs */
      {
        OP_READ_ABS_A(EOR)
      }

    case 0x65:  /* CMP A,abs */
      {
        OP_READ_ABS_A(CMP)
      }

    case 0x85:  /* ADC A,abs */
      {
        OP_READ_ABS_A(ADC)
      }

    case 0xA5:  /* SBC A,abs */
      {
        OP_READ_ABS_A(SBC)
      }

    case 0xC5:  /* MOV abs,A */
      {
        OP_RMW_ABS(WRITE_MOV(_A), 0)
      }

    case 0xE5:  /* MOV A,abs */
      {
        OP_READ_ABS_A(MOV_READ)
      }


    /* xxx00110 */
    case 0x06:  /* OR A,(X) */
      {
        OP_READ_INDIRECT_A(OR)
      }

    case 0x26:  /* AND A,(X) */
      {
        OP_READ_INDIRECT_A(AND)
      }

    case 0x46:  /* EOR A,(X) */
      {
        OP_READ_INDIRECT_A(EOR)
      }

    case 0x66:  /* CMP A,(X) */
      {
        OP_READ_INDIRECT_A(CMP)
      }

    case 0x86:  /* ADC A,(X) */
      {
        OP_READ_INDIRECT_A(ADC)
      }

    case 0xA6:  /* SBC A,(X) */
      {
        OP_READ_INDIRECT_A(SBC)
      }

    case 0xC6:  /* MOV (X),A */
      {
        OP_RMW_INDIRECT(WRITE_MOV(_A), 0)
      }

    case 0xE6:  /* MOV A,(X) */
      {
        OP_READ_INDIRECT_A(MOV_READ)
      }


    /* xxx00111 */
    case 0x07:  /* OR A,(dp+X) */
      {
        OP_READ_INDEXED_INDIRECT_A(OR)
      }

    case 0x27:  /* AND A,(dp+X) */
      {
        OP_READ_INDEXED_INDIRECT_A(AND)
      }

    case 0x47:  /* EOR A,(dp+X) */
      {
        OP_READ_INDEXED_INDIRECT_A(EOR)
      }

    case 0x67:  /* CMP A,(dp+X) */
      {
        OP_READ_INDEXED_INDIRECT_A(CMP)
      }

    case 0x87:  /* ADC A,(dp+X) */
      {
        OP_READ_INDEXED_INDIRECT_A(ADC)
      }

    case 0xA7:  /* SBC A,(dp+X) */
      {
        OP_READ_INDEXED_INDIRECT_A(SBC)
      }

    case 0xC7:  /* MOV (dp+X),A */
      {
        OP_RMW_INDEXED_INDIRECT(WRITE_MOV(_A), 0)
      }

    case 0xE7:  /* MOV A,(dp+X) */
      {
        OP_READ_INDEXED_INDIRECT_A(MOV_READ)
      }


    /* xxx01000 */
    case 0x08:  /* OR A,#imm */
      {
        OP_READ_IMM_A(OR)
      }

    case 0x28:  /* AND A,#imm */
      {
        OP_READ_IMM_A(AND)
      }

    case 0x48:  /* EOR A,#imm */
      {
        OP_READ_IMM_A(EOR)
      }

    case 0x68:  /* CMP A,#imm */
      {
        OP_READ_IMM_A(CMP)
      }

    case 0x88:  /* ADC A,#imm */
      {
        OP_READ_IMM_A(ADC)
      }

    case 0xA8:  /* SBC A,#imm */
      {
        OP_READ_IMM_A(SBC)
      }

    case 0xC8:  /* CMP X,#imm */
      {
        OP_READ_IMM(CMP, _X)
      }

    case 0xE8:  /* MOV A,#imm */
      {
        OP_READ_IMM_A(MOV_READ)
      }


    /* xxx01001 */
    case 0x09:  /* OR dp(d),dp(s) */
      {
        OP_RMW_DP_DP(OR)
      }

    case 0x29:  /* AND dp(d),dp(s) */
      {
        OP_RMW_DP_DP(AND)
      }

    case 0x49:  /* EOR dp(d),dp(s) */
      {
        OP_RMW_DP_DP(EOR)
      }

    case 0x69:  /* CMP dp(d),dp(s) */
      {
        /*  6 cycles - opcode, src address, dest address, src read, */
        /* dest read + op, dummy cycle */
        START_CYCLE(2)
        _address2 = _dp + get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address = _dp + get_byte_spc(_PC);
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data2 = get_byte_spc(_address2);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        _data = get_byte_spc(_address);
        OP_CMP(_data, _data2)
        END_OPCODE(2)
      }

    case 0x89:  /* ADC dp(d),dp(s) */
      {
        OP_RMW_DP_DP(ADC)
      }

    case 0xA9:  /* SBC dp(d),dp(s) */
      {
        OP_RMW_DP_DP(SBC)
      }

    case 0xC9:  /* MOV abs,X */
      {
        OP_RMW_ABS(WRITE_MOV(_X), 0)
      }

    case 0xE9:  /* MOV X,abs */
      {
        OP_READ_ABS(MOV_READ, _X)
      }


    /* xxx01010 */
    case 0x0A:  /* OR1 C,mem.bit */
      {
        /*  5 cycles - opcode, address low, address high, data read, op */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        /* separate bit number, address */
        _offset = _address >> 13;
        _address &= 0x1FFF;
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        if (_data & offset_to_bit[_offset]) set_flag_spc(SPC_FLAG_C);
        END_OPCODE(1)
      }

    case 0x2A:  /* OR1 C,/mem.bit */
      {
        /*  5 cycles - opcode, address low, address high, data read, op */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        /* separate bit number, address */
        _offset = _address >> 13;
        _address &= 0x1FFF;
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        if (!(_data & offset_to_bit[_offset])) set_flag_spc(SPC_FLAG_C);
        END_OPCODE(1)
      }

    case 0x4A:  /* AND1 C,mem.bit */
      {
        /*  4 cycles - opcode, address low, address high, data read + op */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        /* separate bit number, address */
        _offset = _address >> 13;
        _address &= 0x1FFF;
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        if (!(_data & offset_to_bit[_offset])) clr_flag_spc(SPC_FLAG_C);
        END_OPCODE(1)
      }

    case 0x6A:  /* AND1 C,/mem.bit */
      {
        /*  4 cycles - opcode, address low, address high, data read + op */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        /* separate bit number, address */
        _offset = _address >> 13;
        _address &= 0x1FFF;
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        if (_data & offset_to_bit[_offset]) clr_flag_spc(SPC_FLAG_C);
        END_OPCODE(1)
      }

    case 0x8A:  /* EOR1 C,mem.bit */
      {
        /*  5 cycles - opcode, address low, address high, data read, op */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        /* separate bit number, address */
        _offset = _address >> 13;
        _address &= 0x1FFF;
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        if (_data & offset_to_bit[_offset]) complement_carry_spc();
        END_OPCODE(1)
      }

    case 0xAA:  /* MOV1 C,mem.bit */
      {
        /*  4 cycles - opcode, address low, address high, data read */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        /* separate bit number, address */
        _offset = _address >> 13;
        _address &= 0x1FFF;
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        store_flag_c(_data & offset_to_bit[_offset]);
        END_OPCODE(1)
      }

    case 0xCA:  /* MOV1 mem.bit,C */
      {
        /*  6 cycles - opcode, address low, address high, data read, op, */
        /* data write */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        /* separate bit number, address */
        _offset = _address >> 13;
        _address &= 0x1FFF;
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        if (flag_state_spc(SPC_FLAG_C)) _data |= offset_to_bit[_offset];
        else _data &= offset_to_not[_offset];
        END_CYCLE(5, 1)

        START_CYCLE(6)
        set_byte_spc(_address, _data);
        END_OPCODE(1)
      }

    case 0xEA:  /* NOT1 mem.bit */
      {
        /*  5 cycles - opcode, address low, address high, data read, */
        /* op + data write */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        /* separate bit number, address */
        _offset = _address >> 13;
        _address &= 0x1FFF;
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        _data ^= offset_to_bit[_offset];
        set_byte_spc(_address, _data);
        END_OPCODE(1)
      }


    /* xxx01011 */
    case 0x0B:  /* ASL dp */
      {
        OP_RMW_DP(WRITE_OP(ASL), 1)
      }

    case 0x2B:  /* ROL dp */
      {
        OP_RMW_DP(WRITE_OP(ROL), 1)
      }

    case 0x4B:  /* LSR dp */
      {
        OP_RMW_DP(WRITE_OP(LSR), 1)
      }

    case 0x6B:  /* ROR dp */
      {
        OP_RMW_DP(WRITE_OP(ROR), 1)
      }

    case 0x8B:  /* DEC dp */
      {
        OP_RMW_DP(WRITE_OP(DEC), 1)
      }

    case 0xAB:  /* INC dp */
      {
        OP_RMW_DP(WRITE_OP(INC), 1)
      }

    case 0xCB:  /* MOV dp,Y */
      {
        OP_RMW_DP(WRITE_MOV(_Y), 0)
      }

    case 0xEB:  /* MOV Y,dp */
      {
        OP_READ_DP(MOV_READ, _Y)
      }


    /* xxx01100 */
    case 0x0C:  /* ASL abs */
      {
        OP_RMW_ABS(WRITE_OP(ASL), 1)
      }

    case 0x2C:  /* ROL abs */
      {
        OP_RMW_ABS(WRITE_OP(ROL), 1)
      }

    case 0x4C:  /* LSR abs */
      {
        OP_RMW_ABS(WRITE_OP(LSR), 1)
      }

    case 0x6C:  /* ROR abs */
      {
        OP_RMW_ABS(WRITE_OP(ROR), 1)
      }

    case 0x8C:  /* DEC abs */
      {
        OP_RMW_ABS(WRITE_OP(DEC), 1)
      }

    case 0xAC:  /* INC abs */
      {
        OP_RMW_ABS(WRITE_OP(INC), 1)
      }

    case 0xCC:  /* MOV abs,Y */
      {
        OP_RMW_ABS(WRITE_MOV(_Y), 0)
      }

    case 0xEC:  /* MOV Y,abs */
      {
        OP_READ_ABS(MOV_READ, _Y)
      }


    /* xxx01101 */
    case 0x0D:  /* PUSH PSW */
      {
        /*  4 cycles - opcode, address load, data write, SP decrement */
        START_CYCLE(2)
        _address = 0x0100 + _SP;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        spc_setup_flags(_B_flag);
        set_byte_spc(_address, _PSW);
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _SP--;
        END_OPCODE(1)
      }

    case 0x2D:  /* PUSH A */
      {
        OP_PUSH(_A)
      }

    case 0x4D:  /* PUSH X */
      {
        OP_PUSH(_X)
      }

    case 0x6D:  /* PUSH Y */
      {
        OP_PUSH(_Y)
      }

    case 0x8D:  /* MOV Y,#imm */
      {
        OP_READ_IMM(MOV_READ, _Y)
      }

    case 0xAD:  /* CMP Y,#imm */
      {
        OP_READ_IMM(CMP, _Y)
      }

    case 0xCD:  /* MOV X,#imm */
      {
        OP_READ_IMM(MOV_READ, _X)
      }

    case 0xED:  /* NOTC */
      {
        /*  2 cycles - opcode, op */
        START_CYCLE(2)
        complement_carry_spc();
        END_OPCODE(1)
      }


    /* xxx01110 */
    case 0x0E:  /* TSET1 abs */
      {
        /*  6 cycles - opcode, address low, address high, data read, */
        /* test for flags, op + data write */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        store_flags_nz(_data & _A);
        END_CYCLE(5, 1)

        START_CYCLE(6)
        _data |= _A;
        set_byte_spc(_address, _data);
        END_OPCODE(1)
      }

    case 0x2E:  /* CBNE dp,rel */
      {
        COND_DP_REL(TEST_CBNE)
      }

    case 0x4E:  /* TCLR1 abs */
      {
        /*  6 cycles - opcode, address low, address high, data read, */
        /* test for flags, op + data write */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        store_flags_nz(_data & _A);
        END_CYCLE(5, 1)

        START_CYCLE(6)
        _data &= ~_A;
        set_byte_spc(_address, _data);
        END_OPCODE(1)
      }

    case 0x6E:  /* DBNZ dp,rel */
      {
        COND_DP_REL(DP_REL_TEST_DBNZ)
      }

    case 0x8E:  /* POP PSW */
      {
        /*  4 cycles - opcode, SP increment, address load, data read */
        START_CYCLE(2)
        _SP++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address = 0x0100 + _SP;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _PSW = get_byte_spc(_address);
        spc_restore_flags();
        END_OPCODE(1)
      }

    case 0xAE:  /* POP A */
      {
        OP_POP(_A)
      }

    case 0xCE:  /* POP X */
      {
        OP_POP(_X)
      }

    case 0xEE:  /* POP Y */
      {
        OP_POP(_Y)
      }


    /* xxx01111 */
    case 0x2F:  /* BRA rel */
      {
        COND_REL(REL_TEST_BRA)
      }

    case 0x4F:  /* PCALL upage */
      {
        /*  6 cycles - opcode, new PCL, stack address load, PCH write, */
        /* PCL write, SP decrement */
        /* fetch address for PC */
        START_CYCLE(2)
        _address2 = 0xFF00 + get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address = 0x0100 + _SP;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        set_byte_spc(_address, _PC >> 8);
        _SP--;
        _address = 0x0100 + _SP;
        END_CYCLE(4, 1)

        START_CYCLE(5)
        set_byte_spc(_address, _PC);
        END_CYCLE(5, 1)

        START_CYCLE(6)
        _PC = _address2;
        _SP--;
        END_OPCODE(1)
      }

    case 0x6F:  /* RET */
      {
        /*  4 cycles - opcode, SP increment, address load, new PCL, new PCH */
        /* pop address to PC */
        START_CYCLE(2)
        _SP++;
        END_CYCLE(2, 1)

        START_CYCLE(3) \
        _address = 0x0100 + _SP; \
        END_CYCLE(3, 1) \

        START_CYCLE(4) \
        _address2 = get_byte_spc(_address);
        _SP++;
        _address = 0x0100 + _SP;
        END_CYCLE(4, 1)

        START_CYCLE(5)
        _PC = (get_byte_spc(_address) << 8) + _address2;
        END_OPCODE(1)
      }

    case 0x8F:  /* MOV dp,#imm */
      {
        OP_RMW_DP_IMM(MOV_READ_NOFLAGS)
      }

    case 0xAF:  /* MOV (X)+,A */
      {
        /*  4 cycles - opcode, address load, data write, X increment */
        START_CYCLE(2)
        _address = _dp + _X;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        set_byte_spc(_address, _A);
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _X++;
        END_OPCODE(1)
      }

    case 0xCF:  /* MUL YA */
      {
        /*  9 cycles - opcode, 8(op) */
        START_CYCLE(2)
        _YA = (unsigned) _Y * _A;
        store_flags_nz(_Y);
        END_OPCODE(8)
      }

    /* xxx10000 */
    case 0x10:  /* BPL rel */
      {
        COND_REL(REL_TEST_BPL)
      }

    case 0x30:  /* BMI rel */
      {
        COND_REL(REL_TEST_BMI)
      }

    case 0x50:  /* BVC rel */
      {
        COND_REL(REL_TEST_BVC)
      }

    case 0x70:  /* BVS rel */
      {
        COND_REL(REL_TEST_BVS)
      }

    case 0x90:  /* BCC rel */
      {
        COND_REL(REL_TEST_BCC)
      }

    case 0xB0:  /* BCS rel */
      {
        COND_REL(REL_TEST_BCS)
      }

    case 0xD0:  /* BNE rel */
      {
        COND_REL(REL_TEST_BNE)
      }

    case 0xF0:  /* BEQ rel */
      {
        COND_REL(REL_TEST_BEQ)
      }


    /* xxx10010 */
#define opcode_CLR1(bit) (((bit) << 5) + 0x12)
    case opcode_CLR1(0): case opcode_CLR1(1):
    case opcode_CLR1(2): case opcode_CLR1(3):
    case opcode_CLR1(4): case opcode_CLR1(5):
    case opcode_CLR1(6): case opcode_CLR1(7):
      {
        OP_RMW_DP(WRITE_OP(CLR1), 1)
      }


    /* xxx10011 */
#define opcode_BBC(bit) (((bit) << 5) + 0x13)
    case opcode_BBC(0): case opcode_BBC(1):
    case opcode_BBC(2): case opcode_BBC(3):
    case opcode_BBC(4): case opcode_BBC(5):
    case opcode_BBC(6): case opcode_BBC(7):
      {
        COND_DP_REL(DP_REL_TEST_BBC)
      }


    /* xxx10100 */
    case 0x14:  /* OR A,dp+X */
      {
        OP_READ_DP_X_INDEXED_A(OR)
      }

    case 0x34:  /* AND A,dp+X */
      {
        OP_READ_DP_X_INDEXED_A(AND)
      }

    case 0x54:  /* EOR A,dp+X */
      {
        OP_READ_DP_X_INDEXED_A(EOR)
      }

    case 0x74:  /* CMP A,dp+X */
      {
        OP_READ_DP_X_INDEXED_A(CMP)
      }

    case 0x94:  /* ADC A,dp+X */
      {
        OP_READ_DP_X_INDEXED_A(ADC)
      }

    case 0xB4:  /* SBC A,dp+X */
      {
        OP_READ_DP_X_INDEXED_A(SBC)
      }

    case 0xD4:  /* MOV dp+X,A */
      {
        OP_RMW_DP_X_INDEXED(WRITE_MOV(_A), 0)
      }

    case 0xF4:  /* MOV A,dp+X */
      {
        OP_READ_DP_X_INDEXED_A(MOV_READ)
      }


    /* xxx10101 */
    case 0x15:  /* OR A,abs+X */
      {
        OP_READ_ABS_X_INDEXED_A(OR)
      }

    case 0x35:  /* AND A,abs+X */
      {
        OP_READ_ABS_X_INDEXED_A(AND)
      }

    case 0x55:  /* EOR A,abs+X */
      {
        OP_READ_ABS_X_INDEXED_A(EOR)
      }

    case 0x75:  /* CMP A,abs+X */
      {
        OP_READ_ABS_X_INDEXED_A(CMP)
      }

    case 0x95:  /* ADC A,abs+X */
      {
        OP_READ_ABS_X_INDEXED_A(ADC)
      }

    case 0xB5:  /* SBC A,abs+X */
      {
        OP_READ_ABS_X_INDEXED_A(SBC)
      }

    case 0xD5:  /* MOV abs+X,A */
      {
        OP_RMW_ABS_X_INDEXED(WRITE_MOV(_A), 0)
      }

    case 0xF5:  /* MOV A,abs+X */
      {
        OP_READ_ABS_X_INDEXED_A(MOV_READ)
      }


    /* xxx10110 */
    case 0x16:  /* OR A,abs+Y */
      {
        OP_READ_ABS_Y_INDEXED_A(OR)
      }

    case 0x36:  /* AND A,abs+Y */
      {
        OP_READ_ABS_Y_INDEXED_A(AND)
      }

    case 0x56:  /* EOR A,abs+Y */
      {
        OP_READ_ABS_Y_INDEXED_A(EOR)
      }

    case 0x76:  /* CMP A,abs+Y */
      {
        OP_READ_ABS_Y_INDEXED_A(CMP)
      }

    case 0x96:  /* ADC A,abs+Y */
      {
        OP_READ_ABS_Y_INDEXED_A(ADC)
      }

    case 0xB6:  /* SBC A,abs+Y */
      {
        OP_READ_ABS_Y_INDEXED_A(SBC)
      }

    case 0xD6:  /* MOV abs+Y,A */
      {
        OP_RMW_ABS_Y_INDEXED(WRITE_MOV(_A), 0)
      }

    case 0xF6:  /* MOV A,abs+Y */
      {
        OP_READ_ABS_Y_INDEXED_A(MOV_READ)
      }


    /* xxx10111 */
    case 0x17:  /* OR A,(dp)+Y */
      {
        OP_READ_INDIRECT_INDEXED_A(OR)
      }

    case 0x37:  /* AND A,(dp)+Y */
      {
        OP_READ_INDIRECT_INDEXED_A(AND)
      }

    case 0x57:  /* EOR A,(dp)+Y */
      {
        OP_READ_INDIRECT_INDEXED_A(EOR)
      }

    case 0x77:  /* CMP A,(dp)+Y */
      {
        OP_READ_INDIRECT_INDEXED_A(CMP)
      }

    case 0x97:  /* ADC A,(dp)+Y */
      {
        OP_READ_INDIRECT_INDEXED_A(ADC)
      }

    case 0xB7:  /* SBC A,(dp)+Y */
      {
        OP_READ_INDIRECT_INDEXED_A(SBC)
      }

    case 0xD7:  /* MOV (dp)+Y,A */
      {
        OP_RMW_INDIRECT_INDEXED(WRITE_MOV(_A), 0)
      }

    case 0xF7:  /* MOV A,(dp)+Y */
      {
        OP_READ_INDIRECT_INDEXED_A(MOV_READ)
      }


    /* xxx11000 */
    case 0x18:  /* OR dp,#imm */
      {
        OP_RMW_DP_IMM(OR)
      }

    case 0x38:  /* AND dp,#imm */
      {
        OP_RMW_DP_IMM(AND)
      }

    case 0x58:  /* EOR dp,#imm */
      {
        OP_RMW_DP_IMM(EOR)
      }

    case 0x78:  /* CMP dp,#imm */
      {
        /*  5 cycles - opcode, src data, dest address, dest read + op, */
        /* dummy cycle */
        START_CYCLE(2)
        _data2 = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address = _dp + get_byte_spc(_PC);
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        OP_CMP(_data, _data2)
        END_OPCODE(2)
      }

    case 0x98:  /* ADC dp,#imm */
      {
        OP_RMW_DP_IMM(ADC)
      }

    case 0xB8:  /* SBC dp,#imm */
      {
        OP_RMW_DP_IMM(SBC)
      }

    case 0xD8:  /* MOV dp,X */
      {
        OP_RMW_DP(WRITE_MOV(_X), 0)
      }

    case 0xF8:  /* MOV X,dp */
      {
        OP_READ_DP(MOV_READ, _X)
      }


    /* xxx11001 */
    case 0x19:  /* OR (X),(Y) */
      {
        OP_RMW_INDIRECT_INDIRECT(OR)
      }

    case 0x39:  /* AND (X),(Y) */
      {
        OP_RMW_INDIRECT_INDIRECT(AND)
      }

    case 0x59:  /* EOR (X),(Y) */
      {
        OP_RMW_INDIRECT_INDIRECT(EOR)
      }

    case 0x79:  /* CMP (X),(Y) */
      {
        /*  5 cycles - opcode, address calc, src read, dest read + op, */
        /* dummy cycle */
        START_CYCLE(2)
        _address = _dp + _Y;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _data2 = get_byte_spc(_address);
        _address = _dp + _X;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address);
        OP_CMP(_data, _data2)
        END_OPCODE(2)
      }

    case 0x99:  /* ADC (X),(Y) */
      {
        OP_RMW_INDIRECT_INDIRECT(ADC)
      }

    case 0xB9:  /* SBC (X),(Y) */
      {
        OP_RMW_INDIRECT_INDIRECT(SBC)
      }

    case 0xD9:  /* MOV dp+Y,X */
      {
        OP_RMW_DP_Y_INDEXED(WRITE_MOV(_X), 0)
      }

    case 0xF9:  /* MOV X,dp+Y */
      {
        OP_READ_DP_Y_INDEXED(MOV_READ, _X)
      }


    /* xxx11010 */
    case 0x1A:  /* DECW dp */
      {
        OP_RMW16_DP(DECW)
      }

    case 0x3A:  /* INCW dp */
      {
        OP_RMW16_DP(INCW)
      }

    case 0x5A:  /* CMPW YA,dp */
      {
        unsigned temp;

        /*  4 cycles - opcode, address, data low read, data high read + op */
        START_CYCLE(2)
        _address = _dp + get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _data16 = get_byte_spc(_address);
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data16 += get_byte_spc(_address + 1) << 8;
        temp = _YA - _data16;
        store_flag_c(temp <= 0xFFFF);
        store_flag_n(temp >> 8);
        store_flag_z(temp != 0);
        END_OPCODE(1)
      }

    case 0x7A:  /* ADDW YA,dp */
      {
        OP_READ16_YA_DP(ADDW)
      }

    case 0x9A:  /* SUBW YA,dp */
      {
        OP_READ16_YA_DP(SUBW)
      }

    case 0xBA:  /* MOVW YA,dp */
      {
        OP_READ16_YA_DP(MOVW_READ)
      }

    case 0xDA:  /* MOVW dp,YA */
      {
        /*  5 cycles - opcode, address, (?), data low write, data high write, */
        START_CYCLE(2)
        _address = _dp + get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        get_byte_spc(_address);
        END_CYCLE(3, 1)

        START_CYCLE(4)
        set_byte_spc(_address, _A);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        set_byte_spc(_address + 1, _Y);
        END_OPCODE(1)
      }

    case 0xFA:  /* MOV dp(d),dp(s) */
      {
        /*  5 cycles - opcode, src address, dest address, src read, */
        /* dest write */
        START_CYCLE(2)
        _address2 = _dp + get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address = _dp + get_byte_spc(_PC);
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data = get_byte_spc(_address2);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        set_byte_spc(_address, _data);
        END_OPCODE(1)
      }

    /* xxx11011 */
    case 0x1B:  /* ASL dp+X */
      {
        OP_RMW_DP_X_INDEXED(WRITE_OP(ASL), 1)
      }

    case 0x3B:  /* ROL dp+X */
      {
        OP_RMW_DP_X_INDEXED(WRITE_OP(ROL), 1)
      }

    case 0x5B:  /* LSR dp+X */
      {
        OP_RMW_DP_X_INDEXED(WRITE_OP(LSR), 1)
      }

    case 0x7B:  /* ROR dp+X */
      {
        OP_RMW_DP_X_INDEXED(WRITE_OP(ROR), 1)
      }

    case 0x9B:  /* DEC dp+X */
      {
        OP_RMW_DP_X_INDEXED(WRITE_OP(DEC), 1)
      }

    case 0xBB:  /* INC dp+X */
      {
        OP_RMW_DP_X_INDEXED(WRITE_OP(INC), 1)
      }

    case 0xDB:  /* MOV dp+X,Y */
      {
        OP_RMW_DP_X_INDEXED(WRITE_MOV(_Y), 0)
      }

    case 0xFB:  /* MOV Y,dp+X */
      {
        OP_READ_DP_X_INDEXED(MOV_READ, _Y)
      }


    /* xxx11100 */
    case 0x1C:  /* ASL A */
      {
        OP_RMW_IMPLIED(ASL, _A)
      }

    case 0x3C:  /* ROL A */
      {
        OP_RMW_IMPLIED(ROL, _A)
      }

    case 0x5C:  /* LSR A */
      {
        OP_RMW_IMPLIED(LSR, _A)
      }

    case 0x7C:  /* ROR A */
      {
        OP_RMW_IMPLIED(ROR, _A)
      }

    case 0x9C:  /* DEC A */
      {
        OP_RMW_IMPLIED(DEC, _A)
      }

    case 0xBC:  /* INC A */
      {
        OP_RMW_IMPLIED(INC, _A)
      }

    case 0xDC:  /* DEC Y */
      {
        OP_RMW_IMPLIED(DEC, _Y)
      }

    case 0xFC:  /* INC Y */
      {
        OP_RMW_IMPLIED(INC, _Y)
      }


    /* xxx11101 */
    case 0x1D:  /* DEC X */
      {
        OP_RMW_IMPLIED(DEC, _X)
      }

    case 0x3D:  /* INC X */
      {
        OP_RMW_IMPLIED(INC, _X)
      }

    case 0x5D:  /* MOV X,A */
      {
        OP_MOV_IMPLIED(_X, _A)
      }

    case 0x7D:  /* MOV A,X */
      {
        OP_MOV_IMPLIED(_A, _X)
      }

    case 0x9D:  /* MOV X,SP */
      {
        OP_MOV_IMPLIED(_X, _SP)
      }

    case 0xBD:  /* MOV SP,X */
      {
        OP_MOV_IMPLIED_NO_FLAGS(_SP, _X)
      }

    case 0xDD:  /* MOV A,Y */
      {
        OP_MOV_IMPLIED(_A, _Y)
      }

    case 0xFD:  /* MOV Y,A */
      {
        OP_MOV_IMPLIED(_Y, _A)
      }


    /* xxx11110 */
    case 0x1E:  /* CMP X,abs */
      {
        OP_READ_ABS(CMP, _X)
      }

    case 0x3E:  /* CMP X,dp */
      {
        OP_READ_DP(CMP, _X)
      }

    case 0x5E:  /* CMP Y,abs */
      {
        OP_READ_ABS(CMP, _Y)
      }

    case 0x7E:  /* CMP Y,dp */
      {
        OP_READ_DP(CMP, _Y)
      }

    case 0x9E:  /* DIV YA,X */
      {
        unsigned temp = _YA;
        int overflow = 0;

        /*  12 cycles - opcode, 11(op) */
        /* timing and logic of operations completely wrong here */
        START_CYCLE(2)
        if (_X == 0) overflow = 1;
        else overflow = (temp / _X) > 0xFF;
        if (overflow)
        {
          set_flag_spc(SPC_FLAG_N | SPC_FLAG_V);
          clr_flag_spc(SPC_FLAG_Z);
          _YA = 0xFFFF;
        }
        else
        {
          _Y = temp % _X;
          _A = temp / _X;
          clr_flag_spc(SPC_FLAG_V);
          store_flags_nz(_A);
        }
        END_OPCODE(11)
      }

    case 0xDE:  /* CBNE dp+X,rel */
      {
        /*  6 cycles - opcode, address, branch offset, address index */
        /* (add X), data read, branch logic; */
        /* +2 cycles (taken branch) add PC to offset, reload PC */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _offset = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _address = _dp + ((_address + _X) & 0xFF);
        END_CYCLE(4, 1)

        START_CYCLE(5)
        _data = get_byte_spc(_address);
        END_CYCLE(5, 1)

        START_CYCLE(6)
        END_BRANCH_OPCODE(6, TEST_CBNE)
      }

    case 0xFE:  /* DBNZ Y,rel */
      {
        /*  4 cycles - opcode, branch offset, decrement Y, branch logic; */
        /* +2 cycles (taken branch) add PC to offset, reload PC */

        START_CYCLE(2)
        _offset = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _Y--;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        END_BRANCH_OPCODE(4, if (!_Y) EXIT_OPCODE(1))
      }


    /* xxx11111 */
    case 0x1F:  /* JMP (abs+X) */
      {
        /*  6 cycles - opcode, address low, address high */
        /* address index (add X), new PCL, new PCH */
        /* fetch base adderss */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address += get_byte_spc(_PC) << 8;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _address += _X;
        END_CYCLE(4, 1)

        START_CYCLE(5)
        _offset = get_byte_spc(_address);
        END_CYCLE(5, 1)

        START_CYCLE(6)
        _PC = (get_byte_spc(_address + 1) << 8) + _offset;
        END_OPCODE(1)
      }

    case 0x3F:  /* CALL abs */
      {
        /*  8 cycles - opcode, new PCL, new PCH, stack address load, dummy */
        /* cycle (PSW write in BRK?), PCH write, PCL write, */
        /* SP decrement */
        /* fetch address for PC */
        START_CYCLE(2)
        _address2 = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _address2 += (get_byte_spc(_PC) << 8);
        _PC++;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _address = 0x0100 + _SP;
        END_CYCLE(4, 1)

        START_CYCLE(5)
        /* should we write PSW to stack here? */
        END_CYCLE(5, 1)

        START_CYCLE(6)
        set_byte_spc(_address, _PC >> 8);
        _SP--;
        _address = 0x0100 + _SP;
        END_CYCLE(6, 1)

        START_CYCLE(7)
        set_byte_spc(_address, _PC);
        END_CYCLE(7, 1)

        START_CYCLE(8)
        _PC = _address2;
        _SP--;
        END_OPCODE(1)
      }

    case 0x5F:  /* JMP abs */
      {
        /*  3 cycles - opcode, new PCL, new PCH */
        /* fetch address to PC */
        START_CYCLE(2)
        _address = get_byte_spc(_PC);
        _PC++;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _PC = (get_byte_spc(_PC) << 8) + _address;
        END_OPCODE(1)
      }

    case 0x9F:  /* XCN A */
      {
        /*  5 cycles - opcode, 4(op) */
        /* timing of operations may be off here */
        START_CYCLE(2)
        _data = _A;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _A <<= 4;
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _data >>= 4;
        END_CYCLE(4, 1)

        START_CYCLE(5)
        OP_OR(_A, _data);
        END_OPCODE(1)
      }

    case 0xBF:  /* MOV A,(X)+ */
      {
        /*  4 cycles - opcode, address load, data read, X increment */
        START_CYCLE(2)
        _address = _dp + _X;
        END_CYCLE(2, 1)

        START_CYCLE(3)
        _data = get_byte_spc(_address);
        OP_MOV_READ(_A, _data)
        END_CYCLE(3, 1)

        START_CYCLE(4)
        _X++;
        END_OPCODE(1)
      }


    /* handle unhandled or invalid opcodes */
    case 0x0F:  /* BRK */
    case 0x7F:  /* RETI */
    case 0xBE:  /* DAS */
    case 0xDF:  /* DAA */
    case 0xEF:  /* SLEEP */
    case 0xFF:  /* STOP */
    default:
      {
        /* set up address (PC) and opcode for display */
        Map_Byte = _opcode;
        /* Adjust address to correct for increment */
        Map_Address = (_PC - 1) & 0xFFFF;
        save_cycles_spc();    /* Set cycle counter */
        InvalidSPCOpcode();   /* This exits.. aviods conflict with other things! */
        load_cycles_spc();
        break;
      }
    }
    if (opcode_done) _cycle = 0;
  }

  save_cycles_spc();    /* Set cycle counter */

#ifdef INDEPENDENT_SPC
  /* update SPC700 timers to prevent overflow */
  Update_SPC_Timer(0);
  Update_SPC_Timer(1);
  Update_SPC_Timer(2);
#endif

  In_CPU = was_in_cpu;
}

void SPC_START(unsigned cycles)
{
 unsigned long long temp = cycles;
 temp = (temp * SPC_CPU_cycle_multiplicand) + SPC_CPU_cycles_mul;

 /* save remainder */
 SPC_CPU_cycles_mul = temp % SPC_CPU_cycle_divisor;

 cycles = temp / SPC_CPU_cycle_divisor;
 SPC_CPU_cycles = 0;

 /* Add new balance of SPC cycles */
 _Cycles += cycles;
 if (_Cycles <= _TotalCycles)
 {
  if ((int) _Cycles < 0) return;
  if ((int) _TotalCycles >= 0) return;
  Wrap_SPC_Cyclecounter();
 }

 Execute_SPC();
}
