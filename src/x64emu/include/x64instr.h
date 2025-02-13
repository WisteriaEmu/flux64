#ifndef __X64INSTR_H_
#define __X64INSTR_H_

#include <stdbool.h>
#include <stdint.h>

#include "x64regs.h"

/**
 * Decoded x86_64 REX prefix.
 */
typedef union {
    uint8_t byte;
    struct {
        uint8_t b : 1;
        uint8_t x : 1;
        uint8_t r : 1;
        uint8_t w : 1;
        uint8_t   : 4;
    };
} x64rex_t;

/**
 * Decoded x86_64 SIB byte.
 */
typedef union {
    uint8_t byte;
    struct {
        uint8_t base  : 3;
        uint8_t index : 3;
        uint8_t scale : 2;
    };
} x64sib_t;

/**
 * Decoded x86_64 ModR/M byte.
 */
typedef union {
    uint8_t byte;
    struct {
        /* 3 bits, direct or indirect operand, optionally with displacement. */
        uint8_t rm  : 3;

        /* 3 bits, opcode extension or source/destination. */
        uint8_t reg : 3;

        /* 2 bits, usually specifies addressing mode. */
        uint8_t mod : 2;
    };
} x64modrm_t;

/* Some instructions can have up to 8 bytes of
   immediate or displacement data. */

/**
 * Decoded x86_64 instruction.
 */
typedef struct {
    uint8_t    rep;            /* REP/LOCK prefix. */

    x64rex_t   rex;

    /* Operand-size override prefix 0x66 presence.
       When set to `true` makes some opcodes use
       16 bit data. */
    bool       operand_sz;

    /* Address-size override prefix 0x67 presence.
       Changes behaviour of memory addressing for some opcodes
       (16 bit addresses). */
    bool       address_sz;

    uint8_t    opcode[3];

    x64modrm_t modrm;          /* ModR/M byte. */
    x64sib_t   sib;            /* SIB byte. */
    reg64_t    displ;          /* Address displacement. */
    reg64_t    imm;            /* Immediate data. */
} x64instr_t;

/* fetch N bits of instruction. */
#define fetch_8()  *(uint8_t  *)(r_rip++)
#define fetch_16() *(uint16_t *)(r_rip += 2, r_rip - 2)
#define fetch_32() *(uint32_t *)(r_rip += 4, r_rip - 4)
#define fetch_64() *(uint64_t *)(r_rip += 8, r_rip - 8)

#endif /* __X64INSTR_H_ */