#ifndef __X64REGS_H_
#define __X64REGS_H_

#include <stdint.h>

typedef __int128_t  int128_t;
typedef __uint128_t uint128_t;

typedef union {
    int64_t  sq[1]; /*   signed qword */
    uint64_t uq[1]; /* unsigned qword */

    int32_t  sd[2]; /*   signed dword */
    uint32_t ud[2]; /* unsigned dword */

    int16_t  sw[4]; /*   signed word */
    uint16_t uw[4]; /* unsigned word */

    int8_t   sb[8]; /*   signed byte */
    uint8_t  ub[8]; /* unsigned byte */
} reg64_t;

typedef union {
    int128_t  so[1]; /*   signed oword */
    uint128_t uo[1]; /* unsigned oword */

    int64_t  sq[2]; /*   signed qword */
    uint64_t uq[2]; /* unsigned qword */

    int32_t  sd[4]; /*   signed dword */
    uint32_t ud[4]; /* unsigned dword */

    int16_t  sw[8]; /*   signed word */
    uint16_t uw[8]; /* unsigned word */

    int8_t   sb[16]; /*   signed byte */
    uint8_t  ub[16]; /* unsigned byte */
} reg128_t;

#endif /* __X64REGS_H_ */