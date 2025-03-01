#ifndef __X64REGS_H_
#define __X64REGS_H_

#include <stdint.h>

typedef __int128_t  int128_t;
typedef __uint128_t uint128_t;

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

typedef union {
    int128_t  soword[1];
    uint128_t oword[1];
    int64_t   sqword[2];
    uint64_t  qword[2];
    int32_t   sdword[4];
    uint32_t  dword[4];
    int16_t   sword[8];
    uint16_t  word[8];
    int8_t    sbyte[16];
    uint8_t   byte[16];
} reg128_t;

/*typedef union {
    int32_t  sdword[1];
    uint32_t dword[1];
    int16_t  sword[2];
    uint16_t word[2];
    int8_t   sbyte[4];
    uint8_t  byte[4];
} reg32_t;*/

#endif /* __X64REGS_H_ */