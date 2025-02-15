#include <stdint.h>
#include <stdbool.h>

#include "debug.h"
#include "x64instr.h"
#include "x64modrm.h"
#include "x64execute.h"
#include "x64emu.h"
#include "x64regs_private.h"
#include "x64flags_private.h"
#include "x64execute_private.h"
#include "x64stack.h"

SET_DEBUG_CHANNEL("X64EXECUTE")

static inline bool x64execute_83(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_rm(emu, ins);
    switch (ins->modrm.reg) {
        case 0x0:            /* ADD r/m16/32/64,imm8 */
            DEST_OPERATION_SX(OP_SIGNED_ADD, ins->imm.sbyte[0])
            break;
        case 0x1:            /* OR  r/m16/32/64,imm8 */
            DEST_OPERATION_SX(OP_BITWISE_OR, ins->imm.sbyte[0])
            break;
        /* ADC */
        /* SBB */
        case 0x4:            /* AND r/m16/32/64,imm8 */
            DEST_OPERATION_SX(OP_BITWISE_AND, ins->imm.sbyte[0])
            break;
        case 0x5:            /* SUB r/m16/32/64,imm8 */
            DEST_OPERATION_SX(OP_SIGNED_SUB, ins->imm.sbyte[0])
            break;
        case 0x6:            /* XOR r/m16/32/64,imm8 */
            DEST_OPERATION_SX(OP_BITWISE_XOR, ins->imm.sbyte[0])
            break;
        /* CMP */
        default:
            log_err("Unimplemented opcode 83 extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}

static inline bool x64execute_C7(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_rm(emu, ins);
    switch (ins->modrm.reg) {
        case 0x0:            /* MOV r/m16/32/64,imm16/32/32 */
            DEST_OPERATION(OP_SIGNED_MOV, (int64_t)ins->imm.sdword[0], ins->imm.sword[0], ins->imm.sdword[0])
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
            DEST_OPERATION(OP_BITWISE_XOR, *(int64_t *)src, *(int16_t *)src, *(int32_t *)src)
            break;
        }

        case 0x50 ... 0x57: {/* PUSH+r16/64 */
            void *v = emu->regs + ((op & 7) | (ins->rex.b << 3));
            if (ins->operand_sz)
                push_16(emu, *(uint16_t *)v);
            else
                push_64(emu, *(uint64_t *)v);
            break;
        }

        case 0x58 ... 0x5F: {/* POP+r16/64 */
            void *v = emu->regs + ((op & 7) | (ins->rex.b << 3));
            if (ins->operand_sz)
                *(uint16_t *)v = pop_16(emu);
            else
                *(uint64_t *)v = pop_64(emu);
            break;
        }

        case 0x63: {         /* MOVSXD r16/32/64,r/m16/32/32 */
            void *src  = x64modrm_get_rm(emu, ins);
            void *dest = x64modrm_get_reg(emu, ins);
            DEST_OPERATION(OP_SIGNED_MOV, (int64_t)(*(int32_t *)src), *(int16_t *)src, *(int32_t *)src)
            break;
        }

        case 0x83:           /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m16/32/64,imm8 */
            if (!x64execute_83(emu, ins))
                return false;
            break;

        case 0x89: {         /* MOV r/m16/32/64,r16/32/64 */
            void *src  = x64modrm_get_reg(emu, ins);
            void *dest = x64modrm_get_rm(emu, ins);
            DEST_OPERATION(OP_SIGNED_MOV, *(int64_t *)src, *(int16_t *)src, *(int32_t *)src)
            break;
        }

        case 0x8B: {         /* MOV r16/32/64,r/m16/32/64 */
            void *src  = x64modrm_get_rm(emu, ins);
            void *dest = x64modrm_get_reg(emu, ins);
            DEST_OPERATION(OP_SIGNED_MOV, *(int64_t *)src, *(int16_t *)src, *(int32_t *)src)
            break;
        }

        case 0x8D: {         /* LEA r16/32/64,m */
            /* avoiding warning. */
            uintptr_t src = (uintptr_t)x64modrm_get_rm(emu, ins);
            void *dest = x64modrm_get_reg(emu, ins);
            /* Treating src as operand w/o dereferencing,
               signed/unsigned does not change behavior. */
            DEST_OPERATION(OP_SIGNED_MOV, (int64_t)src, (int16_t)src, (int32_t)src)
            break;
        }

        case 0xB8 ... 0xBF: {/* MOV+r16/32/64 imm16/32/64 */
            void *dest = emu->regs + ((op & 7) | (ins->rex.b << 3));
            DEST_OPERATION(OP_SIGNED_MOV, ins->imm.sqword[0], ins->imm.sword[0], ins->imm.sbyte[0])
            break;
        }

        case 0xC7:           /* MOV r/m16/32/64,imm16/32/32 */
            if (!x64execute_C7(emu, ins))
                return false;
            break;

        case 0xE8:           /* CALL rel32 */
            if (ins->address_sz)
                push_32(emu, r_eip);
            else
                push_64(emu, r_rip);
            /* FIXME: Should be EIP in some cases. */
            r_rip += (int64_t)ins->imm.sdword[0];
            break;

        default:
            log_err("Unimplemented opcode %02X", op);
            return false;
    }

    return true;
}
