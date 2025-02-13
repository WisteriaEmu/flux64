#include <stdint.h>
#include <stdbool.h>

#include "debug.h"
#include "x64instr.h"
#include "x64modrm.h"
#include "x64execute.h"
#include "x64emu.h"
#include "x64regs_private.h"
#include "x64stack_private.h"

SET_DEBUG_CHANNEL("X64EXECUTE")

static inline bool x64execute_83(x64emu_t *emu, x64instr_t *ins) {
    /* FIXME: Handle flags. */
    /* imm8 is sign-extended. */
#define OPERATION(operator, t) \
    *(t *)dest = *(t *)dest operator (t)ins->imm.sbyte[0];
#define EXTENSION(operator) \
    if (ins->rex.w)           OPERATION(operator, int64_t) \
    else if (ins->operand_sz) OPERATION(operator, int16_t) \
    else                      OPERATION(operator, int32_t) \
    break;

    void *dest = x64modrm_get_rm(emu, ins);
    if (!dest) return false;
    switch (ins->modrm.reg) {
        case 0x0: EXTENSION(+) /* ADD r/m16/32/64,imm8 */
        case 0x1: EXTENSION(|) /* OR  r/m16/32/64,imm8 */
        /* ADC */
        /* SBB */
        case 0x4: EXTENSION(&) /* AND r/m16/32/64,imm8 */
        case 0x5: EXTENSION(-) /* SUB r/m16/32/64,imm8 */
        case 0x6: EXTENSION(^) /* XOR r/m16/32/64,imm8 */
        /* CMP */
        default:
            log_err("Unimplemented opcode 83 extension %X", ins->modrm.reg);
            return false;
    }
    return true;
#undef EXTENSION
#undef OPERATION
}

static inline bool x64execute_C7(x64emu_t *emu, x64instr_t *ins) {
    switch (ins->modrm.reg) {
        case 0x0: {          /* MOV r/m16/32/64,imm16/32/32 */
            void *dest = x64modrm_get_rm(emu, ins);
            if (!dest) return false;
            /* imm32 is sign-extended */
            if (ins->rex.w)
                *(int64_t *)dest = (int64_t)ins->imm.sdword[0];
            else if (ins->operand_sz)
                *(int16_t *)dest = ins->imm.sword[0];
            else
                *(int32_t *)dest = ins->imm.sdword[0];
            break;
        }
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
            /* FIXME: Handle flags. */
            void *src  = x64modrm_get_reg(emu, ins);
            void *dest = x64modrm_get_rm(emu, ins);
            if (!src || !dest) return false;
            if (ins->rex.w)
                *(uint64_t *)dest ^= *(uint64_t *)src;
            else if (ins->operand_sz)
                *(uint16_t *)dest ^= *(uint16_t *)src;
            else
                *(uint32_t *)dest ^= *(uint32_t *)src;
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
            if (!src || !dest) return false;
            if (ins->rex.w)
                *(int64_t *)dest = *(int32_t *)src;
            else if (ins->operand_sz)
                *(int16_t *)dest = *(int16_t *)src;
            else
                *(int32_t *)dest = *(int32_t *)src;
            break;
        }

        case 0x83:           /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m16/32/64,imm8 */
            if (!x64execute_83(emu, ins))
                return false;
            break;

        case 0x89: {         /* MOV r/m16/32/64,r16/32/64 */
            void *src  = x64modrm_get_reg(emu, ins);
            void *dest = x64modrm_get_rm(emu, ins);
            if (!src || !dest) return false;
            if (ins->rex.w)
                *(uint64_t *)dest = *(uint64_t *)src;
            else if (ins->operand_sz)
                *(uint16_t *)dest = *(uint16_t *)src;
            else
                *(uint32_t *)dest = *(uint32_t *)src;
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
            push_64(emu, r_rip);
            r_rip += (int64_t)ins->displ.sdword[0];
            break;

        default:
            log_err("Unimplemented opcode %02X", op);
            return false;
    }

    return true;
}
