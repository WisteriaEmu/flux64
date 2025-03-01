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

#define OPCODE_EXT_CASE(case_op) \
    case_op(0x0, OP_SIGNED_ADD) \
    case_op(0x1, OP_BITWISE_OR) \
    case_op(0x2, OP_SIGNED_ADC) \
    case_op(0x3, OP_SIGNED_SBB) \
    case_op(0x4, OP_BITWISE_AND) \
    case_op(0x5, OP_SIGNED_SUB) \
    case_op(0x6, OP_BITWISE_XOR) \
    case_op(0x7, OP_SIGNED_CMP)


static inline bool x64execute_80(x64emu_t *emu, x64instr_t *ins) {
#define CASE_OP(x, oper) case x: OPERATION_FIXED_S(R_M, IMM, oper, int8_t, uint8_t) break;
    switch (ins->modrm.reg) { OPCODE_EXT_CASE(CASE_OP) }
#undef CASE_OP
    return true;
}

static inline bool x64execute_81(x64emu_t *emu, x64instr_t *ins) {
#define CASE_OP(x, oper) case x: OPERATION_16_32_64(R_M, IMM, oper, S_32) break;
    switch (ins->modrm.reg) { OPCODE_EXT_CASE(CASE_OP) }
#undef CASE_OP
    return true;
}

static inline bool x64execute_83(x64emu_t *emu, x64instr_t *ins) {
#define CASE_OP(x, oper) case x: OPERATION_16_32_64(R_M, IMM, oper, S_8) break;
    switch (ins->modrm.reg) { OPCODE_EXT_CASE(CASE_OP) }
#undef CASE_OP
    return true;
}

#undef OPCODE_EXT_CASE
#define OPCODE_EXT_CASE(case_op) \
    case_op(0x0, OP_ROTATE_LEFT) \
    case_op(0x1, OP_ROTATE_RIGHT) \
    case_op(0x2, OP_ROTATE_LEFT_CF) \
    case_op(0x3, OP_ROTATE_RIGHT_CF) \
    case_op(0x4, OP_UNSIGNED_SHIFT_LEFT) \
    case_op(0x5, OP_UNSIGNED_SHIFT_RIGHT) \
    case_op(0x6, OP_UNSIGNED_SHIFT_LEFT) \
    case_op(0x7, OP_SIGNED_SHIFT_RIGHT)

static inline bool x64execute_c0(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_r_m(emu, ins);
#define CASE_OP(x, oper) case x: oper(int8_t, uint8_t, ins->imm.byte[0]) break;
    switch (ins->modrm.reg) { OPCODE_EXT_CASE(CASE_OP) }
#undef CASE_OP
    return true;
}

static inline bool x64execute_c1(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_r_m(emu, ins);
#define CASE_OP(x, oper) \
    case x: \
        DEST_OPERATION_16_32_64(oper, ins->imm.byte[0], ins->imm.byte[0], ins->imm.byte[0]) \
        break;
    switch (ins->modrm.reg) { OPCODE_EXT_CASE(CASE_OP) }
#undef CASE_OP
    return true;
}

static inline bool x64execute_d0(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_r_m(emu, ins);
#define CASE_OP(x, oper) case x: oper(int8_t, uint8_t, 1) break;
    switch (ins->modrm.reg) { OPCODE_EXT_CASE(CASE_OP) }
#undef CASE_OP
    return true;
}

static inline bool x64execute_d1(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_r_m(emu, ins);
#define CASE_OP(x, oper) case x: DEST_OPERATION_16_32_64(oper, 1, 1, 1) break;
    switch (ins->modrm.reg) { OPCODE_EXT_CASE(CASE_OP) }
#undef CASE_OP
    return true;
}

static inline bool x64execute_d2(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_r_m(emu, ins);
#define CASE_OP(x, oper) case x: oper(int8_t, uint8_t, r_cl) break;
    switch (ins->modrm.reg) { OPCODE_EXT_CASE(CASE_OP) }
#undef CASE_OP
    return true;
}

static inline bool x64execute_d3(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_r_m(emu, ins);
#define CASE_OP(x, oper) case x: DEST_OPERATION_16_32_64(oper, r_cl, r_cl, r_cl) break;
    switch (ins->modrm.reg) { OPCODE_EXT_CASE(CASE_OP) }
#undef CASE_OP
    return true;
}

#undef OPCODE_EXT_CASE

static inline bool x64execute_8f(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_r_m(emu, ins);
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

static inline bool x64execute_c6(x64emu_t *emu, x64instr_t *ins) {
    switch (ins->modrm.reg) {
        case 0x0:             /* MOV r/m8,imm8 */
            OPERATION_FIXED_S(R_M, IMM, OP_SIGNED_MOV, int8_t, uint8_t)
            break;
        default:
            log_err("Unimplemented opcode C6 extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}

static inline bool x64execute_c7(x64emu_t *emu, x64instr_t *ins) {
    switch (ins->modrm.reg) {
        case 0x0:             /* MOV r/m16/32/64,imm16/32/32 */
            OPERATION_16_32_64(R_M, IMM, OP_SIGNED_MOV, S_32)
            break;
        default:
            log_err("Unimplemented opcode C7 extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}

static inline bool x64execute_f6(x64emu_t *emu, x64instr_t *ins) {
    switch (ins->modrm.reg) {
        case 0x0 ... 0x1:    /* TEST r/m8,imm8 */
            OPERATION_FIXED_S(R_M, IMM, OP_BITWISE_TEST_AND, int8_t, uint8_t)
            break;
        case 0x2:            /* NOT r/m8 */
            OPERATION_FIXED_S(R_M, NULL, OP_BITWISE_NOT, int8_t, uint8_t)
            break;
        case 0x3:            /* NEG r/m8 */
            OPERATION_FIXED_S(R_M, NULL, OP_SIGNED_NEG, int8_t, uint8_t)
            break;
        default:
            log_err("Unimplemented opcode F6 extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}

static inline bool x64execute_f7(x64emu_t *emu, x64instr_t *ins) {
    switch (ins->modrm.reg) {
        case 0x0 ... 0x1:    /* TEST r/m16/32/64,imm16/32/32 */
            OPERATION_16_32_64(R_M, IMM, OP_BITWISE_TEST_AND, S_32)
            break;
        case 0x2:            /* NOT r/m16/32/64 */
            OPERATION_16_32_64(R_M, NULL, OP_BITWISE_NOT, S_32)
            break;
        case 0x3:            /* NEG r/m16/32/64 */
            OPERATION_16_32_64(R_M, NULL, OP_SIGNED_NEG, S_32)
            break;
        default:
            log_err("Unimplemented opcode F7 extension %X", ins->modrm.reg);
            return false;
    }
    return true;
}


static inline bool x64execute_fe(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_r_m(emu, ins);
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

static inline bool x64execute_ff(x64emu_t *emu, x64instr_t *ins) {
    void *dest = x64modrm_get_r_m(emu, ins);
    switch (ins->modrm.reg) {
        case 0x0:             /* INC r/m16/32/64 */
            DEST_OPERATION_16_32_64(OP_SIGNED_INC, 1, 1, 1)
            break;
        case 0x1:             /* DEC r/m16/32/64 */
            DEST_OPERATION_16_32_64(OP_SIGNED_DEC, 1, 1, 1)
            break;
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

    switch (op) {

#define OPCODE_FAMILY(start, oper) \
        case start + 0x00:    /* r/m8,r8 */ \
            OPERATION_FIXED_S(R_M, REG, oper, int8_t, uint8_t) \
            break; \
        case start + 0x01:    /* r/m16/32/64,r16/32/64 */ \
            OPERATION_16_32_64(R_M, REG, oper, S_64) \
            break; \
        case start + 0x02:    /* r8,r/m8 */ \
            OPERATION_FIXED_S(REG, R_M, oper, int8_t, uint8_t) \
            break; \
        case start + 0x03:    /* r16/32/64,r/m16/32/64 */ \
            OPERATION_16_32_64(REG, R_M, oper, S_64) \
            break; \
        case start + 0x04:    /* al,imm8 */ \
            OPERATION_FIXED_S(GPR(_rax), IMM, oper, int8_t, uint8_t) \
            break; \
        case start + 0x05:    /* ax/eax/rax,imm16/32/32 */ \
            OPERATION_16_32_64(GPR(_rax), IMM, oper, S_32) \
            break;

        OPCODE_FAMILY(0x00, OP_SIGNED_ADD)    /* 00..05 ADD */
        OPCODE_FAMILY(0x08, OP_BITWISE_OR)    /* 08..0D OR */
        OPCODE_FAMILY(0x10, OP_SIGNED_ADC)    /* 10..15 ADC */
        OPCODE_FAMILY(0x18, OP_SIGNED_SBB)    /* 18..1D SBB */
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
            OPERATION_16_32_64(REG, R_M, OP_SIGNED_MOV, S_32)
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
            if (x64execute_jmp_cond(emu, ins, op))
                r_rip += (int64_t)ins->imm.sbyte[0];
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

        case 0x84:            /* TEST r/m8,r8 */
            OPERATION_FIXED_S(R_M, REG, OP_BITWISE_TEST_AND, int8_t, uint8_t)
            break;

        case 0x85:            /* TEST r/m16/32/64,r16/32/64 */
            OPERATION_16_32_64(R_M, REG, OP_BITWISE_TEST_AND, S_64)
            break;

        case 0x86:            /* XCHG r8,r/m8 */
            PP_OPERATION_FIXED(REG, R_M, PP_XCHG, int8_t, uint8_t)
            break;

        case 0x87:            /* XCHG r16/32/64,r/m16/32/64 */
            PP_OPERATION(REG, R_M, PP_XCHG)
            break;

        case 0x88:            /* MOV r/m8,r8 */
            OPERATION_FIXED_U(R_M, REG, OP_UNSIGNED_MOV, int8_t, uint8_t)
            break;

        case 0x89:            /* MOV r/m16/32/64,r16/32/64 */
            OPERATION_16_32_64(R_M, REG, OP_UNSIGNED_MOV, U_64)
            break;

        case 0x8A:            /* MOV r8,r/m8 */
            OPERATION_FIXED_U(REG, R_M, OP_UNSIGNED_MOV, int8_t, uint8_t)
            break;

        case 0x8B:            /* MOV r16/32/64,r/m16/32/64 */
            OPERATION_16_32_64(REG, R_M, OP_UNSIGNED_MOV, U_64)
            break;

        case 0x8D: {          /* LEA r16/32/64,m */
            /* FIXME: this is not r/m... */
            /* avoiding warning. */
            uintptr_t src = (uintptr_t)x64modrm_get_r_m(emu, ins);
            void *dest = x64modrm_get_reg(emu, ins);
            /* src is operand, not a pointer. */
            DEST_OPERATION_16_32_64(OP_UNSIGNED_MOV, (uint64_t)src, (uint16_t)src, (uint32_t)src)
            break;
        }

        case 0x8F:            /* POP r/m16/64 */
            if (!x64execute_8f(emu, ins))
                return false;
            break;

        case 0x90 ... 0x97:   /* XCHG+r16/32/64 rAX */
            PP_OPERATION(GPR((op & 7) | (ins->rex.b << 3)), GPR(_rax), PP_XCHG)
            break;

        case 0x98: {          /* CBW/CWDE/CDQE */
            void *dest = emu->regs + _rax;
            DEST_OPERATION_16_32_64(OP_SIGNED_MOV, (int64_t)s_eax, (int16_t)s_al, (int32_t)s_ax)
            break;
        }

        case 0x99: {          /* CWD/CDQ/CQO */
            /* uncertain about dest. */
            void *dest = emu->regs + _rdx;
            DEST_OPERATION_16_32_64(OP_SIGNED_MOV, (int64_t)s_eax, (int16_t)s_al, (int32_t)s_ax)
            break;
        }

        case 0x9C:            /* PUSHF/PUSHFQ */
            if (ins->operand_sz)
                push_16(emu, (uint16_t)r_flags);
            else
                push_64(emu, r_flags & 0xFCFFFF);
            break;

        case 0x9D:            /* POPF/POPFQ */
            /* FIXME: Privilege levels. (currently always 3) */
            if (ins->operand_sz)
                //    00100111111010101  to be updated from the stack
                //        4   F   D   5
                // ..101011000000101010  to be preserved
                //    E   B   0   2   A
                r_eflags = (r_eflags & 0xfffeb02a) | ((uint32_t)pop_16(emu) & 0x4fd5);
            else
                //     1001000100111111010101  to be updated from the stack
                //      2   4   4   F   D   5
                // ..110110101011000000101010  to be preserved
                //      D   A   B   0   2   A
                r_eflags = (r_eflags & 0xffdab02a) | ((uint32_t)pop_64(emu) & 0x244fd5);
            break;

        case 0x9E:            /* SAHF */
            // 11010101 to be updated
            /* FIXME: May be an invalid instruction. */
            r_eflags = (r_eflags & (~0xD5)) | (r_ah & 0xD5);
            break;

        case 0x9F:            /* LAHF */
            /* FIXME: May be an invalid instruction. */
            r_ah = ((uint8_t)r_eflags & 0xD5) | 2;
            break;

        case 0xA8:            /* TEST imm8 */
            OPERATION_FIXED_S(GPR(_rax), IMM, OP_BITWISE_TEST_AND, int8_t, uint8_t)
            break;

        case 0xA9:            /* TEST imm16/32 */
            OPERATION_16_32_64(GPR(_rax), IMM, OP_BITWISE_TEST_AND, S_32)
            break;

        case 0xAA: {          /* STOS m8 */
            uint64_t dest = (ins->address_sz) ? (uint64_t)r_edi : r_rdi;
            OP_UNSIGNED_MOV_REP(int8_t, uint8_t, r_al)
            break;
        }

        case 0xAB: {          /* STOS m16/32/64 */
            uint64_t dest = (ins->address_sz) ? (uint64_t)r_edi : r_rdi;
            DEST_OPERATION_16_32_64_U_64(OP_UNSIGNED_MOV_REP, emu->regs + _rax)
            break;
        }

        case 0xB0 ... 0xB7:   /* MOV+r8 imm8 */
            OPERATION_FIXED_U(GPR((op & 7) | (ins->rex.b << 3)), IMM, OP_UNSIGNED_MOV, int8_t, uint8_t)
            break;

        case 0xB8 ... 0xBF:   /* MOV+r16/32/64 imm16/32/64 */
            OPERATION_16_32_64(GPR((op & 7) | (ins->rex.b << 3)), IMM, OP_UNSIGNED_MOV, U_64)
            break;

        case 0xC0:            /* rotate/shift r/m8,imm8 */
            if (!x64execute_c0(emu, ins))
                return false;
            break;
        case 0xC1:            /* rotate/shift r/m16/32/64,imm8 */
            if (!x64execute_c1(emu, ins))
                return false;
            break;

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

        case 0xC6:            /* MOV r/m8,imm8 */
            if (!x64execute_c6(emu, ins))
                return false;
            break;

        case 0xC7:            /* MOV r/m16/32/64,imm16/32/32 */
            if (!x64execute_c7(emu, ins))
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

        case 0xD0:            /* rotate/shift r/m8,1 */
            if (!x64execute_d0(emu, ins))
                return false;
            break;

        case 0xD1:            /* rotate/shift r/m16/32/64,1 */
            if (!x64execute_d1(emu, ins))
                return false;
            break;

        case 0xD2:            /* rotate/shift r/m8,CL */
            if (!x64execute_d2(emu, ins))
                return false;
            break;

        case 0xD3:            /* rotate/shift r/m16/32/64,CL */
            if (!x64execute_d3(emu, ins))
                return false;
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

        case 0xF5:            /* CMC */
            f_CF = !f_CF;
            break;

        case 0xF6:            /* TEST/NOT/NEG/MUL/IMUL/DIV/IDIV r/m8,imm8 */
            if (!x64execute_f6(emu, ins))
                return false;
            break;

        case 0xF7:            /* TEST/NOT/NEG/MUL/IMUL/DIV/IDIV r/m16/32/64,imm16/32/32 */
            if (!x64execute_f7(emu, ins))
                return false;
            break;

        case 0xF8 ... 0xF9:   /* CLC/STC */
            f_CF = op & 1;
            break;

        case 0xFA ... 0xFB:   /* CLI/STI */
            f_IF = op & 1;
            break;

        case 0xFC ... 0xFD:   /* CLD/STD */
            f_DF = op & 1;
            break;

        case 0xFE:            /* INC/DEC r/m8 */
            if (!x64execute_fe(emu, ins))
                return false;
            break;

        case 0xFF:            /* INC/DEC/CALL/CALLF/JMP/JMPF/PUSH r/m */
            if (!x64execute_ff(emu, ins))
                return false;
            break;

        default:
            log_err("Unimplemented opcode %02X", op);
            return false;
    }

    return true;
}
