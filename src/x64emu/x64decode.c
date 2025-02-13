#include <stdint.h>
#include <stdbool.h>

#include "debug.h"
#include "x64instr.h"
#include "x64decode.h"
#include "x64emu.h"
#include "x64modrm.h"
#include "x64regs_private.h"

SET_DEBUG_CHANNEL("X64DECODE")

/**
 * @return Next byte of instruction that does not seem like a prefix byte.
 */
static inline uint8_t decode_prefixes(x64emu_t *emu, x64instr_t *ins) {
    while (1) {
        uint8_t byte = fetch_8();
        switch (byte) {
            case 0x40 ... 0x4F:  /* REX prefix. */
                ins->rex.byte = byte;
                break;
            case 0x66:           /* operand-size override prefix. */
                ins->operand_sz = true;
                break;
            case 0x67:           /* address-size override prefix. */
                ins->address_sz = true;
                break;
            case 0xF0 ... 0xF3:  /* REP/LOCK prefix. */
                if (byte == 0xF0) {
                    log_err("Unimplemented LOCK prefix");
                    return false;
                }
                log_dump("REP %02X", byte);
                ins->rep = byte;
                break;
            default:
                return byte;
        }
    }
}

bool x64decode(x64emu_t *emu, x64instr_t *ins) {
    ins->opcode[0] = decode_prefixes(emu, ins);

    /* During decoding our goal is to map instruction bytes
       to fields of `x64instr_t`. */

    switch (ins->opcode[0]) {
        case 0x0F:           /* Two-byte opcodes */
            if (!x64decode_0f(emu, ins))
                return false;
            break;

        case 0x31:           /* XOR r/m16/32/64,r16/32/64 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x50 ... 0x5F:  /* PUSH/POP+r16/32/64 */
            break;

        case 0x63:           /* MOVSXD r16/32/64,r/m16/32/32 */
            x64modrm_fetch(emu, ins);
            break;

        case 0x83:           /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m16/32/64,imm8 */
            x64modrm_fetch(emu, ins);
            ins->imm.byte[0] = fetch_8();
            break;

        case 0x89:           /* MOV r/m16/32/64,r16/32/64 */
            x64modrm_fetch(emu, ins);
            break;

        case 0xB8 ... 0xBF:  /* MOV+r16/32/64 imm16/32/64 */
            if (ins->rex.w)
                ins->imm.qword[0] = fetch_64();
            else if (ins->operand_sz)
                ins->imm.word[0]  = fetch_16();
            else
                ins->imm.dword[0] = fetch_32();
            break;

        case 0xC7:           /* MOV r/m16/32/64,imm16/32/32 */
            x64modrm_fetch(emu, ins);
            if (ins->operand_sz)
                ins->imm.word[0]  = fetch_16();
            else
                ins->imm.dword[0] = fetch_32();
            break;

        case 0xE8:           /* CALL rel32 */
            /* rel32 is immediate data */
            ins->imm.dword[0] = fetch_32();
            break;

        default:
            log_err("Unhandled opcode %02X", ins->opcode[0]);
            return false;
    }

    return true;
}
