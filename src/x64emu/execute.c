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

/* Some ugly macros for repeating opcodes and their extensions. */

#define OPCODE_EXT_CASE(case_op) \
    case_op(0x0, OP_SIGNED_ADD) \
    case_op(0x1, OP_BITWISE_OR) \
    /* ADC */ \
    /* SBB */ \
    case_op(0x4, OP_BITWISE_AND) \
    case_op(0x5, OP_SIGNED_SUB) \
    case_op(0x6, OP_BITWISE_XOR) \
    case_op(0x7, OP_SIGNED_CMP)


static inline bool x64execute_80(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_rm(emu, ins);
#define CASE_OP(x, oper) \
    case x: oper(int8_t, uint8_t, ins->imm.sbyte[0]) break;

    switch (ins->modrm.reg) {
        OPCODE_EXT_CASE(CASE_OP)
        default:
            log_err("Unimplemented opcode 80 extension %X", ins->modrm.reg);
            return false;
    }
#undef CASE_OP
    return true;
}

static inline bool x64execute_81(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_rm(emu, ins);
#define CASE_OP(x, oper) \
    case x: DEST_OPERATION_S_32(oper, &ins->imm) break;

    switch (ins->modrm.reg) {
        OPCODE_EXT_CASE(CASE_OP)
        default:
            log_err("Unimplemented opcode 81 extension %X", ins->modrm.reg);
            return false;
    }
#undef CASE_OP
    return true;
}

static inline bool x64execute_83(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_rm(emu, ins);
#define CASE_OP(x, oper) \
    case x: DEST_OPERATION_S_8(oper, &ins->imm) break;

    switch (ins->modrm.reg) {
        OPCODE_EXT_CASE(CASE_OP)
        default:
            log_err("Unimplemented opcode 83 extension %X", ins->modrm.reg);
            return false;
    }
#undef CASE_OP
    return true;
}

#undef OPCODE_EXT_CASE

/* ugly macros end. */

static inline bool x64execute_8F(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_rm(emu, ins);
    switch (ins->modrm.reg) {
        case 0x0:            /* POP r/m16/64 */
            if (ins->operand_sz)
                *(uint16_t *)dest = pop_16(emu);
            else
                *(uint64_t *)dest = pop_64(emu);
            break;
        default:
            log_err("Unimplemented opcode 8F extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}

static inline bool x64execute_C7(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_rm(emu, ins);
    switch (ins->modrm.reg) {
        case 0x0:             /* MOV r/m16/32/64,imm16/32/32 */
            DEST_OPERATION_S_32(OP_SIGNED_MOV, &ins->imm)
            break;
        default:
            log_err("Unimplemented opcode C7 extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}

static inline bool x64execute_FE(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_rm(emu, ins);
    switch (ins->modrm.reg) {
        case 0x0:             /* INC r/m8 */
            OP_SIGNED_INC(int8_t, uint8_t, 1)
            break;
        case 0x1:             /* DEC r/m8 */
            OP_SIGNED_DEC(int8_t, uint8_t, 1)
            break;
        default:
            log_err("Unimplemented opcode FE extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}

static inline bool x64execute_FF(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_rm(emu, ins);
    switch (ins->modrm.reg) {
        case 0x0: {           /* INC r/m16/32/64 */
            int8_t dummy = 1;
            DEST_OPERATION_S_8(OP_SIGNED_INC, &dummy)
            break;
        }
        case 0x1: {           /* DEC r/m16/32/64 */
            int8_t dummy = 1;
            DEST_OPERATION_S_8(OP_SIGNED_DEC, &dummy)
            break;
        }
        case 0x2:             /* CALL r/m64 */
            if (ins->address_sz)
                push_32(emu, r_eip);
            else
                push_64(emu, r_rip);
            r_rip = *(uint64_t *)dest;
            break;
        case 0x4:             /* JMP r/m64 */
            r_rip = *(uint64_t *)dest;
            break;
        default:
            log_err("Unimplemented opcode FF extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}


bool x64execute(x64emu_t *emu, x64instr_t *ins) {
    uint8_t op = ins->opcode[0];

    /* Get ready for ugly macros... */

    switch (op) {

#define OPCODE_FAMILY(start, oper) \
        case start + 0x00:    /* r/m8,r8 */ \
            OPERATION_FIXED_S(DEST_RM_SRC_REG, oper, int8_t, uint8_t) \
            break; \
        case start + 0x01:    /* r/m16/32/64,r16/32/64 */ \
            OPERATION_16_32_64(DEST_RM_SRC_REG, oper, S_64) \
            break; \
        case start + 0x02:    /* r8,r/m8 */ \
            OPERATION_FIXED_S(DEST_REG_SRC_RM, oper, int8_t, uint8_t) \
            break; \
        case start + 0x03:    /* r16/32/64,r/m16/32/64 */ \
            OPERATION_16_32_64(DEST_REG_SRC_RM, oper, S_64) \
            break; \
        case start + 0x04: {  /* al,imm8 */ \
            void *dest = emu->regs + _rax; \
            oper(int8_t, uint8_t, ins->imm.sbyte[0]) \
            break; \
        } \
        case start + 0x05: {  /* ax/eax/rax,imm16/32/32 */ \
            void *dest = emu->regs + _rax; \
            DEST_OPERATION_S_32(oper, &ins->imm) \
            break; \
        }

        OPCODE_FAMILY(0x00, OP_SIGNED_ADD)    /* 00..05 ADD */
        OPCODE_FAMILY(0x08, OP_BITWISE_OR)    /* 08..0D OR */
        /* ADC */
        /* SBB */
        OPCODE_FAMILY(0x20, OP_BITWISE_AND)   /* 20..25 AND */
        OPCODE_FAMILY(0x28, OP_SIGNED_SUB)    /* 28..2D SUB */
        OPCODE_FAMILY(0x30, OP_BITWISE_XOR)   /* 30..35 XOR */
        OPCODE_FAMILY(0x38, OP_SIGNED_CMP)    /* 38..3D CMP */
#undef OPCODE_FAMILY

        case 0x0F:            /* Two-byte opcodes */
            if (!x64execute_0f(emu, ins))
                return false;
            break;

        case 0x50 ... 0x57: { /* PUSH+r16/64 */
            void *v = emu->regs + ((op & 7) | (ins->rex.b << 3));
            if (ins->operand_sz)
                push_16(emu, *(uint16_t *)v);
            else
                push_64(emu, *(uint64_t *)v);
            break;
        }

        case 0x58 ... 0x5F: { /* POP+r16/64 */
            void *v = emu->regs + ((op & 7) | (ins->rex.b << 3));
            if (ins->operand_sz)
                *(uint16_t *)v = pop_16(emu);
            else
                *(uint64_t *)v = pop_64(emu);
            break;
        }

        case 0x63:            /* MOVSXD r16/32/64,r/m16/32/32 */
            OPERATION_16_32_64(DEST_REG_SRC_RM, OP_SIGNED_MOV, S_32)
            break;

        case 0x68:            /* PUSH imm16/32 */
            if (ins->operand_sz)
                push_16(emu, ins->imm.sword[0]);
            else
                push_64(emu, (int64_t)ins->imm.sdword[0]);
            break;

        case 0x6A:            /* PUSH imm8 */
            if (ins->operand_sz)
                push_16(emu, (int16_t)ins->imm.sbyte[0]);
            else
                push_64(emu, (int64_t)ins->imm.sbyte[0]);
            break;

        case 0x70 ... 0x7F:   /* Jcc rel8 */
            if (x64execute_jmp_cond(emu, ins, op)) {
                r_rip += (int64_t)ins->imm.sbyte[0];
                log_dump("Jump taken");
            } else log_dump("Jump not taken");
            break;

        case 0x80:            /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m8,imm8 */
            if (!x64execute_80(emu, ins))
                return false;
            break;

        case 0x81:            /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m16/32/64,imm16/32/32 */
            if (!x64execute_81(emu, ins))
                return false;
            break;

        case 0x83:            /* ADD/OR/ADC/SBB/AND/SUB/XOR/CMP r/m16/32/64,imm8 */
            if (!x64execute_83(emu, ins))
                return false;
            break;

        case 0x84: {          /* TEST r/m8,r8 */
            OPERATION_FIXED_S(DEST_RM_SRC_REG, OP_BITWISE_TEST_AND, int8_t, uint8_t)
            break;
        }

        case 0x85:            /* TEST r/m16/32/64,r16/32/64 */
            OPERATION_16_32_64(DEST_RM_SRC_REG, OP_BITWISE_TEST_AND, S_64)
            break;

        case 0x86: {          /* XCHG r8,r/m8 */
            GET_DEST_REG_SRC_RM()
            PP_XCHG(int8_t, uint8_t, src)
            break;
        }

        case 0x87: {          /* XCHG r16/32/64,r/m16/32/64 */
            GET_DEST_REG_SRC_RM()
            PP_OPERATION(PP_XCHG, src)
            break;
        }

        case 0x88:            /* MOV r/m8,r8 */
            OPERATION_FIXED_U(DEST_RM_SRC_REG, OP_UNSIGNED_MOV, int8_t, uint8_t)
            break;

        case 0x89:            /* MOV r/m16/32/64,r16/32/64 */
            OPERATION_16_32_64(DEST_RM_SRC_REG, OP_UNSIGNED_MOV, U_64)
            break;

        case 0x8A:            /* MOV r8,r/m8 */
            OPERATION_FIXED_U(DEST_REG_SRC_RM, OP_UNSIGNED_MOV, int8_t, uint8_t)
            break;

        case 0x8B:            /* MOV r16/32/64,r/m16/32/64 */
            OPERATION_16_32_64(DEST_REG_SRC_RM, OP_UNSIGNED_MOV, U_64)
            break;

        case 0x8D: {          /* LEA r16/32/64,m */
            /* avoiding warning. */
            uintptr_t src = (uintptr_t)x64modrm_get_rm(emu, ins);
            void *dest = x64modrm_get_reg(emu, ins);
            /* src is operand, not a pointer. */
            DEST_OPERATION(OP_UNSIGNED_MOV, (uint64_t)src, (uint16_t)src, (uint32_t)src)
            break;
        }

        case 0x8F:            /* POP r/m16/64 */
            if (!x64execute_8F(emu, ins))
                return false;
            break;

        case 0x90 ... 0x97: { /* XCHG+r16/32/64 rAX */
            void *dest = emu->regs + ((op & 7) | (ins->rex.b << 3));
            PP_OPERATION(PP_XCHG, emu->regs + _rax)
            break;
        }

        case 0xAA: {          /* STOS m8 */
            uint64_t dest = (ins->address_sz) ? (uint64_t)r_edi : r_rdi;
            OP_UNSIGNED_MOV_REP(int8_t, uint8_t, r_al)
            break;
        }

        case 0xAB: {          /* STOS m16/32/64 */
            uint64_t dest = (ins->address_sz) ? (uint64_t)r_edi : r_rdi;
            DEST_OPERATION_U_64(OP_UNSIGNED_MOV_REP, emu->regs + _rax)
            break;
        }

        case 0xB8 ... 0xBF: { /* MOV+r16/32/64 imm16/32/64 */
            void *dest = emu->regs + ((op & 7) | (ins->rex.b << 3));
            DEST_OPERATION_U_64(OP_UNSIGNED_MOV, &ins->imm)
            break;
        }

        case 0xC2:            /* RET imm16 */
            if (ins->address_sz)
                r_eip = pop_32(emu);
            else
                r_rip = pop_64(emu);
            r_rsp += (uint64_t)ins->imm.word[0];
            break;

        case 0xC3:            /* RET */
            if (ins->address_sz)
                r_eip = pop_32(emu);
            else
                r_rip = pop_64(emu);
            break;

        case 0xC7:            /* MOV r/m16/32/64,imm16/32/32 */
            if (!x64execute_C7(emu, ins))
                return false;
            break;

        case 0xC9:            /* LEAVE */
            if (ins->operand_sz) {
                r_sp = r_bp;
                r_bp = pop_16(emu);
            } else {
                r_rsp = r_rbp;
                r_rbp = pop_64(emu);
            }
            break;

        case 0xE8:            /* CALL rel32 */
            if (ins->address_sz)
                push_32(emu, r_eip);
            else
                push_64(emu, r_rip);
            r_rip += (int64_t)ins->imm.sdword[0];
            break;

        case 0xE9:            /* JMP rel32 */
            r_rip += (int64_t)ins->imm.sdword[0];
            break;

        case 0xEB:            /* JMP rel8 */
            r_rip += (int64_t)ins->imm.sbyte[0];
            break;

        case 0xFE:            /* INC/DEC r/m8 */
            if (!x64execute_FE(emu, ins))
                return false;
            break;

        case 0xFF:            /* INC/DEC/CALL/CALLF/JMP/JMPF/PUSH r/m */
            if (!x64execute_FF(emu, ins))
                return false;
            break;

        default:
            log_err("Unimplemented opcode %02X", op);
            return false;
    }

    return true;
}
