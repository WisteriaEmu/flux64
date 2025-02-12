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
    switch (ins->modrm.reg) {
        case 0x4: {          /* AND r/m,imm8 */
            /* FIXME: Implement 16 bit version. Handle flags. */
            void *dest = x64modrm_dest_rm(emu, ins);
            if (!dest) return false;
            /* imm8 is sign-extended. */
            if (ins->rex.w)
                *(int64_t *)dest &= (int64_t)ins->imm.sbyte[0];
            else
                *(int32_t *)dest &= (int32_t)ins->imm.sbyte[0];
            break;
        }
        default:
            log_err("Unimplemented opcode 83 extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}

static inline bool x64execute_C7(x64emu_t *emu, x64instr_t *ins) {
    switch (ins->modrm.reg) {
        case 0x0: {          /* MOV r/m16/32/64,imm16/32/32 */
            /* FIXME: Implement 16 bit version. */
            void *dest = x64modrm_dest_rm(emu, ins);
            if (!dest) return false;
            /* imm32 is sign-extended */
            if (ins->rex.w)
                *(int64_t *)dest = (int64_t)ins->imm.sdword[0];
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
            /* FIXME: Implement 16 bit version. Handle flags. */
            void *src  = x64modrm_src_reg(emu, ins);
            void *dest = x64modrm_dest_rm(emu, ins);
            if (!src || !dest) return false;
            if (ins->rex.w)
                *(uint64_t *)dest ^= *(uint64_t *)src;
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

        case 0x83:           /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m,imm8 */
            if (!x64execute_83(emu, ins))
                return false;
            break;

        case 0x89: {         /* MOV r/m16/32/64,r16/32/64 */
            /* FIXME: Implement 16 bit version. */
            void *src  = x64modrm_src_reg(emu, ins);
            void *dest = x64modrm_dest_rm(emu, ins);
            if (!src || !dest) return false;
            if (ins->rex.w)
                *(uint64_t *)dest = (*(uint64_t *)src);
            else
                *(uint32_t *)dest = (*(uint32_t *)src);
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

        default:
            log_err("Unimplemented opcode %02X", op);
            return false;
    }

    return true;
}
