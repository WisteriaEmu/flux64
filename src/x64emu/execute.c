#include <stdint.h>
#include <stdbool.h>

#include "debug.h"
#include "x64instr.h"
#include "x64modrm.h"
#include "x64execute.h"
#include "x64emu.h"
#include "x64regs_private.h"
#include "x64stack.h"

SET_DEBUG_CHANNEL("X64EXECUTE")

#define BASE_OPERATION(operator, dest_type, src_type) \
    *(dest_type *)dest operator ## = *(src_type *)src;

/* FIXME: Handle flags. */

/* Sign-extended operation on dest and src. */
#define SX_OPERATION(operator, src64, src16, src32) \
    if (!src || !dest) return false; \
    if      (ins->rex.w)      BASE_OPERATION(operator, int64_t, src64) \
    else if (ins->operand_sz) BASE_OPERATION(operator, int16_t, src16) \
    else                      BASE_OPERATION(operator, int32_t, src32)

static inline bool x64execute_83(x64emu_t *emu, x64instr_t *ins) {
    void *src  = ins->imm.sqword;
    void *dest = x64modrm_get_rm(emu, ins);
    switch (ins->modrm.reg) {
        case 0x0:            /* ADD r/m16/32/64,imm8 */
            SX_OPERATION(+, int8_t, int8_t, int8_t)
            break;
        case 0x1:            /* OR  r/m16/32/64,imm8 */
            SX_OPERATION(|, int8_t, int8_t, int8_t)
            break;
        /* ADC */
        /* SBB */
        case 0x4:            /* AND r/m16/32/64,imm8 */
            SX_OPERATION(&, int8_t, int8_t, int8_t)
            break;
        case 0x5:            /* SUB r/m16/32/64,imm8 */
            SX_OPERATION(-, int8_t, int8_t, int8_t)
            break;
        case 0x6:            /* XOR r/m16/32/64,imm8 */
            SX_OPERATION(^, int8_t, int8_t, int8_t)
            break;
        /* CMP */
        default:
            log_err("Unimplemented opcode 83 extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}

static inline bool x64execute_C7(x64emu_t *emu, x64instr_t *ins) {
    void *src  = ins->imm.sqword;
    void *dest = x64modrm_get_rm(emu, ins);
    switch (ins->modrm.reg) {
        case 0x0:            /* MOV r/m16/32/64,imm16/32/32 */
            SX_OPERATION(, int32_t, int16_t, int32_t)
            break;
        default:
            log_err("Unimplemented opcode C7 extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}

bool x64execute(x64emu_t *emu, x64instr_t *ins) {
    uint8_t op = ins->opcode[0];

    switch (op) {
        case 0x0F:           /* Two-byte opcodes */
            if (!x64execute_0f(emu, ins))
                return false;
            break;

        case 0x31: {         /* XOR r/m16/32/64,r16/32/64 */
            void *src  = x64modrm_get_reg(emu, ins);
            void *dest = x64modrm_get_rm(emu, ins);
            SX_OPERATION(^, int64_t, int16_t, int32_t)
            break;
        }

        case 0x50 ... 0x57:  /* PUSH+r16/32/64 */
            /* FIXME: Implement 16 and 32 bit versions. */
            push_64(emu, r_reg64((op & 7) | (ins->rex.b << 3)));
            break;

        case 0x58 ... 0x5F:  /* POP+r16/32/64 */
            /* FIXME: Implement 16 and 32 bit versions. */
            r_reg64((op & 7) | (ins->rex.b << 3)) = pop_64(emu);
            break;

        case 0x63: {         /* MOVSXD r16/32/64,r/m16/32/32 */
            void *src  = x64modrm_get_rm(emu, ins);
            void *dest = x64modrm_get_reg(emu, ins);
            SX_OPERATION(, int32_t, int16_t, int32_t)
            break;
        }

        case 0x83:           /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m16/32/64,imm8 */
            if (!x64execute_83(emu, ins))
                return false;
            break;

        case 0x89: {         /* MOV r/m16/32/64,r16/32/64 */
            void *src  = x64modrm_get_reg(emu, ins);
            void *dest = x64modrm_get_rm(emu, ins);
            SX_OPERATION(, int64_t, int16_t, int32_t)
            break;
        }

        case 0xB8 ... 0xBF:  /* MOV+r16/32/64 imm16/32/64 */
            /* FIXME: Implement 16 bit version. */
            /* No need to zero-extend. */
            r_reg64((op & 7) | (ins->rex.b << 3)) = ins->imm.qword[0];
            break;

        case 0xC7:           /* MOV r/m16/32/64,imm16/32/32 */
            if (!x64execute_C7(emu, ins))
                return false;
            break;

        case 0xE8:           /* CALL rel32 */
            if (ins->address_sz)
                push_32(emu, r_eip);
            else
                push_64(emu, r_rip);
            r_rip += (int64_t)ins->imm.sdword[0];
            break;

        default:
            log_err("Unimplemented opcode %02X", op);
            return false;
    }

    return true;
}

#undef BASE_OPERATION
#undef SX_OPERATION
