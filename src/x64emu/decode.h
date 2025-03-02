#ifndef __X64DECODE_H_
#define __X64DECODE_H_

#include <stdint.h>
#include <stdbool.h>

#include "debug.h"
#include "x64instr.h"
#include "x64emu.h"

#include "modrm.h"
#include "regs_private.h"
#include "decode_private.h"
#include "decode_0f.h"

#ifdef HAVE_TRACE
uint8_t fetch_8(x64emu_t *emu, x64instr_t *ins) {
    uint8_t v = *(uint8_t  *)(r_rip++);
    *(uint8_t *)(ins->desc.bytes + ins->desc.bytes_len) = v;
    ins->desc.bytes_len += 1;
    return v;
}

uint16_t fetch_16(x64emu_t *emu, x64instr_t *ins) {
    uint16_t v = *(uint16_t *)(r_rip += 2, r_rip - 2);
    *(uint16_t *)(ins->desc.bytes + ins->desc.bytes_len) = v;
    ins->desc.bytes_len += 2;
    return v;
}

uint32_t fetch_32(x64emu_t *emu, x64instr_t *ins) {
    uint32_t v = *(uint32_t *)(r_rip += 4, r_rip - 4);
    *(uint32_t *)(ins->desc.bytes + ins->desc.bytes_len) = v;
    ins->desc.bytes_len += 4;
    return v;
}

uint64_t fetch_64(x64emu_t *emu, x64instr_t *ins) {
    uint64_t v = *(uint64_t *)(r_rip += 8, r_rip - 8);
    *(uint64_t *)(ins->desc.bytes + ins->desc.bytes_len) = v;
    ins->desc.bytes_len += 8;
    return v;
}
#endif /* HAVE_TRACE */

/**
 * @return Next byte of instruction that does not seem like a prefix byte.
 */
static inline uint8_t decode_prefixes(x64emu_t *emu, x64instr_t *ins) {

    while (1) {
        uint8_t byte = fetch_8(emu, ins);
        switch (byte) {
            case 0x40 ... 0x4F:   /* REX. */
                ins->rex.byte = byte;
                return fetch_8(emu, ins); /* REX must be the last prefix. */
            case 0x26:
            case 0x2E:            /* Branch not taken. */
            case 0x36:
            case 0x3E:            /* Branch taken. */
                break;
            case 0x64:
            case 0x65:            /* Segment override. */
                log_err("Unimplemented segment override: %02X", byte);
                return 6; /* Any invalid opcode. */
            case 0x66:            /* Operand-size override. */
                ins->operand_sz = true;
                break;
            case 0x67:            /* Address-size override. */
                ins->address_sz = true;
                break;
            case 0xF0:            /* LOCK */
            case 0xF2:            /* REPNE/REPNZ */
            case 0xF3:            /* REPE/REPZ */
                if (byte == 0xF0) {
                    log_err("Unimplemented LOCK prefix");
                    return 6; /* Any invalid opcode. */
                }
                ins->rep = byte;
                break;
            default:
                return byte;
        }
    }
}

static inline bool x64decode(x64emu_t *emu, x64instr_t *ins) {
    ins->opcode[0] = decode_prefixes(emu, ins);

    /* During decoding our goal is to map instruction bytes
       to fields of `x64instr_t`. */

    switch (ins->opcode[0]) {
#define OPCODE_FAMILY(start) \
        case start + 0x00:    /* r/m8,r8 */ \
        case start + 0x01:    /* r/m16/32/64,r16/32/64 */ \
        case start + 0x02:    /* r8,r/m8 */ \
        case start + 0x03:    /* r16/32/64,r/m16/32/64 */ \
            x64modrm_fetch(emu, ins); \
            break; \
        case start + 0x04:    /* al,imm8 */ \
            FETCH_IMM_8() \
            break; \
        case start + 0x05:    /* ax/eax/rax,imm16/32 */ \
            FETCH_IMM_16_32() \
            break;

        OPCODE_FAMILY(0x00)   /* 00..05 ADD */
        OPCODE_FAMILY(0x08)   /* 08..0D OR */
        OPCODE_FAMILY(0x10)   /* 10..15 ADC */
        OPCODE_FAMILY(0x18)   /* 18..1D SBB */
        OPCODE_FAMILY(0x20)   /* 20..25 AND */
        OPCODE_FAMILY(0x28)   /* 28..2D SUB */
        OPCODE_FAMILY(0x30)   /* 30..35 XOR */
        OPCODE_FAMILY(0x38)   /* 38..3D CMP */
#undef OPCODE_FAMILY

        case 0x0F:            /* Two-byte opcodes */
            if (!x64decode_0f(emu, ins))
                return false;
            break;

        case 0x50 ... 0x5F:   /* PUSH/POP+r16/32/64 */
            break;

        case 0x63:            /* MOVSXD r16/32/64,r/m16/32 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x68:            /* PUSH imm16/32 */
            FETCH_IMM_16_32()
            break;

        case 0x6A:            /* PUSH imm8 */
            FETCH_IMM_8()
            break;

        case 0x70 ... 0x7F:   /* Jcc rel8 */
            FETCH_IMM_8()
            break;

        case 0x80:            /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m8,imm8 */
            x64modrm_fetch(emu, ins);
            FETCH_IMM_8()
            break;

        case 0x81:            /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m16/32/64,imm16/32 */
            x64modrm_fetch(emu, ins);
            FETCH_IMM_16_32()
            break;

        case 0x83:            /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m16/32/64,imm8 */
            x64modrm_fetch(emu, ins);
            FETCH_IMM_8()
            break;

        case 0x84:            /* TEST r/m8,r8 */
        case 0x85:            /* TEST r/m16/32/64,r16/32/64 */
        case 0x86:            /* XCHG r8,r/m8 */
        case 0x87:            /* XCHG r16/32/64,r/m16/32/64 */
        case 0x88:            /* MOV r/m8,r8 */
        case 0x89:            /* MOV r/m16/32/64,r16/32/64 */
        case 0x8A:            /* MOV r8,r/m8 */
        case 0x8B:            /* MOV r16/32/64,r/m16/32/64 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x8D:            /* LEA r16/32/64,m */
            x64modrm_fetch(emu, ins);
            break;

        case 0x8F:            /* POP r/m16/64 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x90 ... 0x97:   /* XCHG+r16/32/64 rAX */
        case 0x98:            /* CBW/CWDE/CDQE */
        case 0x99:            /* CWD/CDQ/CQO */
        case 0x9C:            /* PUSGF/PUSGFQ */
        case 0x9D:            /* POPF/POPFQ */
        case 0x9E:            /* SAHF */
        case 0x9F:            /* LAHF */
            break;

        case 0xA8:            /* TEST imm8 */
            FETCH_IMM_8()
            break;

        case 0xA9:            /* TEST imm16/32 */
            FETCH_IMM_16_32()
            break;

        case 0xAA ... 0xAB:   /* STOS m8/16/32/64 */
            break;

        case 0xB0 ... 0xB7:   /* MOV+r8 imm8 */
            FETCH_IMM_8()
            break;

        case 0xB8 ... 0xBF:   /* MOV+r16/32/64 imm16/32/64 */
            FETCH_IMM_16_32_64()
            break;

        case 0xC0 ... 0xC1:   /* rotate/shift r/m8/16/32/64,imm8 */
            x64modrm_fetch(emu, ins);
            FETCH_IMM_8()
            break;

        case 0xC2:            /* RET imm16 */
            FETCH_IMM_16()
            break;

        case 0xC3:            /* RET */
            break;

        case 0xC6:            /* MOV r/m8,imm8 */
            x64modrm_fetch(emu, ins);
            FETCH_IMM_8()
            break;

        case 0xC7:            /* MOV r/m16/32/64,imm16/32 */
            x64modrm_fetch(emu, ins);
            FETCH_IMM_16_32()
            break;

        case 0xC9:            /* LEAVE */
            break;

        case 0xD0 ... 0xD3:   /* rotate/shift r/m,1/CL */
            x64modrm_fetch(emu, ins);
            break;

        case 0xE8 ... 0xE9:   /* CALL/JMP rel32 */
            FETCH_IMM_32()
            break;

        case 0xEB:            /* JMP rel8 */
            FETCH_IMM_8()
            break;

        case 0xF5:            /* CMC */
            break;

        case 0xF6:            /* TEST/NOT/NEG/MUL/IMUL/DIV/IDIV r/m8,imm8 */
            x64modrm_fetch(emu, ins);
            if (ins->modrm.reg <= 1) { FETCH_IMM_8() }
            break;

        case 0xF7:            /* TEST/NOT/NEG/MUL/IMUL/DIV/IDIV r/m16/32/64,imm16/32 */
            x64modrm_fetch(emu, ins);
            if (ins->modrm.reg <= 1) { FETCH_IMM_16_32() }
            break;

        case 0xF8 ... 0xFD:   /* CLC/STC/CLI/STI/CLD/STD */
            break;

        case 0xFE:            /* INC/DEC r/m8 */
        case 0xFF:            /* INC/DEC/CALL/CALLF/JMP/JMPF/PUSH r/m16/32/64 */
            x64modrm_fetch(emu, ins);
            break;

        default:
            log_err("Unhandled opcode %02X", ins->opcode[0]);
            return false;
    }

    return true;
}

#endif /* __X64DECODE_H_ */