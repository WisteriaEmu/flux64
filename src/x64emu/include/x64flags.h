#ifndef __X64FLAGS_H_
#define __X64FLAGS_H_

#include <stdint.h>

typedef union {
    uint64_t raw;
    struct {
        uint64_t CF   : 1;  /*     0  Carry Flag */
        uint64_t      : 1;  /*     1  Reserved */
        uint64_t PF   : 1;  /*     2  Parity Flag */
        uint64_t      : 1;  /*     3  Reserved */
        uint64_t AF   : 1;  /*     4  Auxiliary Carry Flag */
        uint64_t      : 1;  /*     5  Reserved */
        uint64_t ZF   : 1;  /*     6  Zero Flag */
        uint64_t SF   : 1;  /*     7  Sign Flag */
        uint64_t TF   : 1;  /*     8  Trap Flag */
        uint64_t IF   : 1;  /*     9  Interrupt Enable Flag */
        uint64_t DF   : 1;  /*    10  Direction Flag */
        uint64_t OF   : 1;  /*    11  Overflow Flag */
        uint64_t IOPL : 2;  /* 12-13  I/O Privelege Level */
        uint64_t NT   : 1;  /*    14  Nested Task */
        uint64_t      : 1;  /*    15  Reserved */
        uint64_t RF   : 1;  /*    16  Resume Flag */
        uint64_t VM   : 1;  /*    17  Virtual-8086 Mode */
        uint64_t AC   : 1;  /*    18  Alignment Check/Access Control */
        uint64_t VIF  : 1;  /*    19  Virtual Interrupt Flag */
        uint64_t VIP  : 1;  /*    20  Virtual Interrupt Pending */
        uint64_t ID   : 1;  /*    21  ID Flag */
        uint64_t      : 42; /* 22-63  Reserved */
    };
} x64flags_t;

#endif /* __X64FLAGS_H_ */