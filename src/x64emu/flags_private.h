#ifndef __X64FLAGS_PRIVATE_H_
#define __X64FLAGS_PRIVATE_H_

#define f_CF   emu->flags.CF    /*     0  Carry Flag */
#define f_PF   emu->flags.PF    /*     2  Parity Flag */
#define f_AF   emu->flags.AF    /*     4  Auxiliary Carry Flag */
#define f_ZF   emu->flags.ZF    /*     6  Zero Flag */
#define f_SF   emu->flags.SF    /*     7  Sign Flag */
#define f_TF   emu->flags.TF    /*     8  Trap Flag */
#define f_IF   emu->flags.IF    /*     9  Interrupt Enable Flag */
#define f_DF   emu->flags.DF    /*    10  Direction Flag */
#define f_OF   emu->flags.OF    /*    11  Overflow Flag */
#define f_IOPL emu->flags.IOPL  /* 12-13  I/O Privelege Level */
#define f_NT   emu->flags.NT    /*    14  Nested Task */
#define f_RF   emu->flags.RF    /*    16  Resume Flag */
#define f_VM   emu->flags.VM    /*    17  Virtual-8086 Mode */
#define f_AC   emu->flags.AC    /*    18  Alignment Check/Access Control */
#define f_VIF  emu->flags.VIF   /*    19  Virtual Interrupt Flag */
#define f_VIP  emu->flags.VIP   /*    20  Virtual Interrupt Pending */
#define f_ID   emu->flags.ID    /*    21  ID Flag */

#endif /* __X64FLAGS_PRIVATE_H_ */