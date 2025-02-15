#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "x64instr.h"
#include "x64decode.h"
#include "x64emu.h"
#include "x64modrm.h"
#include "x64regs_private.h"

SET_DEBUG_CHANNEL("X64DECODE")

uint8_t fetch_8(x64emu_t *emu, x64instr_t *ins) {
    uint8_t v = *(uint8_t  *)(r_rip++);
    OPCODE_APPEND("%02x ", v)
    return v;
}

uint16_t fetch_16(x64emu_t *emu, x64instr_t *ins) {
    uint16_t v = *(uint16_t *)(r_rip += 2, r_rip - 2);
    OPCODE_APPEND("%04x ", v)
    return v;
}

uint32_t fetch_32(x64emu_t *emu, x64instr_t *ins) {
    uint32_t v = *(uint32_t *)(r_rip += 4, r_rip - 4);
    OPCODE_APPEND("%08x ", v)
    return v;
}

uint64_t fetch_64(x64emu_t *emu, x64instr_t *ins) {
    uint64_t v = *(uint64_t *)(r_rip += 8, r_rip - 8);
    OPCODE_APPEND("%016lx ", v)
    return v;
}

/**
 * @return Next byte of instruction that does not seem like a prefix byte.
 */
static inline uint8_t decode_prefixes(x64emu_t *emu, x64instr_t *ins) {

    while (1) {
        uint8_t byte = fetch_8(emu, ins);
        switch (byte) {
            case 0x40 ... 0x4F:   /* REX prefix. */
                ins->rex.byte = byte;
                break;
            case 0x2E:            /* Branch not taken. */
            case 0x36:
            case 0x3E:            /* Branch taken. */
            case 0x26:
            case 0x64:
            case 0x65:            /* Segment override prefixes. */
                log_err("Unimplemented segment override: %02X", byte);
                return false;
            case 0x66:            /* Operand-size override prefix. */
                ins->operand_sz = true;
                break;
            case 0x67:            /* Address-size override prefix. */
                ins->address_sz = true;
                break;
            case 0xF0:
            case 0xF2:
            case 0xF3:            /* REP/LOCK prefix. */
                if (byte == 0xF0) {
                    log_err("Unimplemented LOCK prefix");
                    return false;
                }
                ins->rep = byte;
                break;
            default:
                return byte;
        }
    }
}

static inline void fetch_imm_16_32_32(x64emu_t *emu, x64instr_t *ins) {
    if (ins->operand_sz)
        ins->imm.word[0]  = fetch_16(emu, ins);
    else
        ins->imm.dword[0] = fetch_32(emu, ins);
}

static inline void fetch_imm_16_32_64(x64emu_t *emu, x64instr_t *ins) {
    if (ins->rex.w)
        ins->imm.qword[0] = fetch_64(emu, ins);
    else if (ins->operand_sz)
        ins->imm.word[0]  = fetch_16(emu, ins);
    else
        ins->imm.dword[0] = fetch_32(emu, ins);
}

bool x64decode(x64emu_t *emu, x64instr_t *ins) {
    ins->opcode[0] = decode_prefixes(emu, ins);

    /* During decoding our goal is to map instruction bytes
       to fields of `x64instr_t`. */

    /* Get ready for ugly macros... */

    switch (ins->opcode[0]) {
#define OPCODE_FAMILY(start) \
        case start + 0x00:    /* r/m8,r8 */ \
        case start + 0x01:    /* r/m16/32/64,r16/32/64 */ \
        case start + 0x02:    /* r8,r/m8 */ \
        case start + 0x03:    /* r16/32/64,r/m16/32/64 */ \
            x64modrm_fetch(emu, ins); \
            break; \
        case start + 0x04:    /* al,imm8 */ \
            ins->imm.byte[0] = fetch_8(emu, ins); \
            break; \
        case start + 0x05:    /* ax/eax/rax,imm16/32/32 */ \
            fetch_imm_16_32_32(emu, ins); \
            break;

        OPCODE_FAMILY(0x00)   /* 00..05 ADD */

        OPCODE_FAMILY(0x08)   /* 08..0D OR */

        /* ADC */

        /* SBB */

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

        case 0x63:            /* MOVSXD r16/32/64,r/m16/32/32 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x70 ... 0x7F:   /* Jcc rel8 */
            ins->imm.byte[0] = fetch_8(emu, ins);
            break;

        case 0x81:            /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m16/32/64,imm16/32/32 */
            x64modrm_fetch(emu, ins);
            fetch_imm_16_32_32(emu, ins);
            break;

        case 0x83:            /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m16/32/64,imm8 */
            x64modrm_fetch(emu, ins);
            ins->imm.byte[0] = fetch_8(emu, ins);
            break;

        case 0x85:            /* TEST r/m16/32/64,r16/32/64 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x89:            /* MOV r/m16/32/64,r16/32/64 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x8B:            /* MOV r16/32/64,r/m16/32/64 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x8D:            /* LEA r16/32/64,m */
            x64modrm_fetch(emu, ins);
            break;

        case 0xAB:            /* STOS m16/32/64 */
            break;

        case 0xB8 ... 0xBF:   /* MOV+r16/32/64 imm16/32/64 */
            fetch_imm_16_32_64(emu, ins);
            break;

        case 0xC7:            /* MOV r/m16/32/64,imm16/32/32 */
            x64modrm_fetch(emu, ins);
            fetch_imm_16_32_32(emu, ins);
            break;

        case 0xC9:            /* LEAVE */
            break;

        case 0xE8:            /* CALL rel32 */
            /* rel32 is immediate data */
            ins->imm.dword[0] = fetch_32(emu, ins);
            break;

        default:
            log_err("Unhandled opcode %02X", ins->opcode[0]);
            return false;
    }

    return true;
}
