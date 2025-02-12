#ifndef __X64REGS_H_
#define __X64REGS_H_

#include <stdint.h>

typedef union {
    int64_t  sqword[1];
    uint64_t qword[1];
    int32_t  sdword[2];
    uint32_t dword[2];
    int16_t  sword[4];
    uint16_t word[4];
    int8_t   sbyte[8];
    uint8_t  byte[8];
} reg64_t;

/*typedef union {
    int32_t  sdword[1];
    uint32_t dword[1];
    int16_t  sword[2];
    uint16_t word[2];
    int8_t   sbyte[4];
    uint8_t  byte[4];
} reg32_t;*/

#endif /* __X64REGS_H_ */