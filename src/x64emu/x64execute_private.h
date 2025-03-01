
#ifndef __X64EXECUTE_PRIVATE_H_
#define __X64EXECUTE_PRIVATE_H_

#include <stdint.h>

/* 1 when byte has even number of set bits. */
static inline uint8_t get_byte_parity(uint8_t x) {
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return (~x) & 1;
}

/* Set SF, ZF, PF. */
#define SET_RESULT_FLAGS(x) \
    f_SF = (x) < 0; \
    f_ZF = (x) == 0; \
    f_PF = get_byte_parity(x);

/* when shift = 0, no flags are affected. */
/* CF (for shift > 0) is set to the last bit shifted out,
   OF (for shift = 1) to CF xor result MSB. */
#define ROTATE_LEFT_FLAG_IMPL(s_type, u_type, operand) \
    if (shift > 0) { \
        f_CF = (tdest >> (type_len - shift)) & 1; \
        if (shift == 1) f_OF = f_CF ^ (*(u_type *)dest >> (type_len - 1)); \
    }


/* CF (for shift > 0) is set to the last bit shifted out,
   OF (for shift = 1) to MSB xor MSB-1. */
#define ROTATE_RIGHT_FLAG_IMPL(s_type, u_type, operand) \
    if (shift > 0) { \
        f_CF = (tdest >> (shift - 1)) & 1; \
        if (shift == 1) f_OF = ((*(u_type *)dest >> (type_len - 2)) & 1) ^ \
                                (*(u_type *)dest >> (type_len - 1)); \
    }


/* Same as rotating, but sets SF, ZF, PF. */

#define SHIFT_LEFT_FLAG_IMPL(s_type, u_type, operand) \
    if (shift > 0) { \
        f_CF = (tdest >> (type_len - shift)) & 1; \
        SET_RESULT_FLAGS(*(s_type *)dest) \
        if (shift == 1) f_OF = f_CF ^ (*(u_type *)dest >> (type_len - 1)); \
    }

/* for signed shift and shift > type_len CF is set to 1? */

#define SHIFT_RIGHT_FLAG_IMPL(s_type, u_type, operand) \
    if (shift > 0) { \
        f_CF = (tdest >> (shift - 1)) & 1; \
        SET_RESULT_FLAGS(*(s_type *)dest) \
        if (shift == 1) f_OF = ((*(u_type *)dest >> (type_len - 2)) & 1) ^ \
                                (*(u_type *)dest >> (type_len - 1)); \
    }



/* Shared variables macro between rotate operations. */
#define ROTATE_PROLOGUE(s_type, u_type, operand) \
    uint8_t type_len = sizeof(u_type) * 8; \
    uint8_t shift = (operand) & (type_len - 1); \
    u_type tdest = *(u_type *)dest;



#define OP_ROTATE_LEFT(s_type, u_type, operand) { \
    ROTATE_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest = (tdest << shift) | (tdest >> (type_len - shift)); \
    ROTATE_LEFT_FLAG_IMPL(s_type, u_type, operand) \
}

#define OP_ROTATE_RIGHT(s_type, u_type, operand) { \
    ROTATE_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest = (tdest >> shift) | (tdest << (type_len - shift)); \
    ROTATE_RIGHT_FLAG_IMPL(s_type, u_type, operand) \
}


#define OP_ROTATE_LEFT_CF(s_type, u_type, operand) { \
    ROTATE_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest = (tdest << shift) | (tdest >> (type_len + 1 - shift)) | \
                      ((shift) ? (f_CF << (shift - 1)) : 0); \
    ROTATE_LEFT_FLAG_IMPL(s_type, u_type, operand) \
}

#define OP_ROTATE_RIGHT_CF(s_type, u_type, operand) { \
    ROTATE_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest = (tdest >> shift) | (tdest << (type_len + 1 - shift)) | \
                      (f_CF << (type_len - shift)); \
    ROTATE_RIGHT_FLAG_IMPL(s_type, u_type, operand) \
}



#define OP_UNSIGNED_SHIFT_LEFT(s_type, u_type, operand) { \
    ROTATE_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest <<= shift; \
    SHIFT_LEFT_FLAG_IMPL(s_type, u_type, operand) \
}

#define OP_UNSIGNED_SHIFT_RIGHT(s_type, u_type, operand) { \
    ROTATE_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest >>= shift; \
    SHIFT_RIGHT_FLAG_IMPL(s_type, u_type, operand) \
}


/* FIXME: signed type shifts are implementation-defined. */
/* operand is unsigned. */
#define OP_SIGNED_SHIFT_RIGHT(s_type, u_type, operand) { \
    ROTATE_PROLOGUE(s_type, s_type, operand) \
    *(s_type *)dest >>= shift; \
    SHIFT_RIGHT_FLAG_IMPL(s_type, u_type, operand) \
}




/* NOTE: state of AF is undefined. */
#define OP_BITWISE_IMPL(oper, s_type, u_type, operand) { \
    *(s_type *)dest oper ## = (operand); \
    SET_RESULT_FLAGS(*(s_type *)dest) \
    f_CF = f_OF = 0; \
}


#define OP_BITWISE_TEST_IMPL(oper, s_type, u_type, operand) { \
    s_type _res = (*(s_type *)dest) oper (operand); \
    SET_RESULT_FLAGS(_res) \
    f_CF = f_OF = 0; \
}


/* `*dest ^= operand`, updates ZF, PF, SF, operands are signed. */
#define OP_BITWISE_XOR(s_type, u_type, operand) \
    OP_BITWISE_IMPL(^, s_type, u_type, operand)


/* `*dest &= operand`, updates ZF, PF, SF, operands are signed. */
#define OP_BITWISE_AND(s_type, u_type, operand) \
    OP_BITWISE_IMPL(&, s_type, u_type, operand)


/* Update ZF, PF, SF with result of `*dest & operand`, operands are signed. */
#define OP_BITWISE_TEST_AND(s_type, u_type, operand) \
    OP_BITWISE_TEST_IMPL(&, s_type, u_type, operand)


/* `*dest |= operand`, updates ZF, PF, SF, operands are signed. */
#define OP_BITWISE_OR(s_type, u_type, operand) \
    OP_BITWISE_IMPL(|, s_type, u_type, operand)

/* `*dest = ~(*dest)` */
#define OP_BITWISE_NOT(s_type, u_type, operand) \
    *(s_type *)dest = ~(*(s_type *)dest);

/* `*dest = -(*dest)`, signed. */
#define OP_SIGNED_NEG(s_type, u_type, operand) \
    *(s_type *)dest = -(*(s_type *)dest);


#define OP_SIGNED_ADD_IMPL(s_type, u_type, operand) \
    s_type _sav = *(s_type *)dest; \
    s_type _res = _sav + (operand); \
    SET_RESULT_FLAGS(_res) \
    /* CF is set in other macro */ \
    f_AF = (uint8_t)_res < (uint8_t)_sav; \
    f_OF = ((operand) <  0 && _sav <  0 && _res >= 0) || \
           ((operand) >= 0 && _sav >= 0 && _res <  0);


/* `*dest += operand`, updates AF, OF, CF, ZF, PF, SF, operands are signed. */
#define OP_SIGNED_ADD(s_type, u_type, operand) { \
    OP_SIGNED_ADD_IMPL(s_type, u_type, operand) \
    f_CF = (u_type)_res < (u_type)_sav; \
    *(s_type *)dest = _res; \
}

/* `*dest += operand`, updates AF, OF, ZF, PF, SF, operands are signed. */
#define OP_SIGNED_INC(s_type, u_type, operand) { \
    OP_SIGNED_ADD_IMPL(s_type, u_type, operand) \
    *(s_type *)dest = _res; \
}

/* Probably bad. */

/* `*dest += CF + operand`, updates AF, OF, CF, ZF, PF, SF, operands are signed. */
#define OP_SIGNED_ADC(s_type, u_type, operand) { \
    s_type _sav; \
    s_type _res; \
    if (f_CF) { \
        _sav = *(s_type *)dest; \
        _res = _sav + 1; \
        f_CF = (u_type)_res < (u_type)_sav; \
        f_AF = (uint8_t)_res < (uint8_t)_sav; \
        f_OF = _sav >= 0 && _res < 0; \
    } \
    _sav = *(s_type *)dest; \
    _res = _sav + (operand); \
    SET_RESULT_FLAGS(_res) \
    f_CF |= (u_type)_res < (u_type)_sav; \
    f_AF |= (uint8_t)_res < (uint8_t)_sav; \
    f_OF |= ((operand) <  0 && _sav <  0 && _res >= 0) || \
            ((operand) >= 0 && _sav >= 0 && _res <  0); \
    *(s_type *)dest = _res; \
}


#define OP_SIGNED_CMP_IMPL(s_type, u_type, operand) \
    s_type _sav = *(s_type *)dest; \
    s_type _res = _sav - (operand); \
    SET_RESULT_FLAGS(_res) \
    /* CF is set in other macro */ \
    f_AF = (uint8_t)_res > (uint8_t)_sav; \
    f_OF = ((operand) >= 0 && _sav <  0 && _res >= 0) || \
           ((operand) <  0 && _sav >= 0 && _res <  0);


/* Updates AF, OF, CF, ZF, SF, PF with result of `*dest - operand`. */
#define OP_SIGNED_CMP(s_type, u_type, operand) { \
    OP_SIGNED_CMP_IMPL(s_type, u_type, operand) \
    f_CF = (u_type)_res > (u_type)_sav; \
}

/* `*dest -= operand`, updates AF, OF, CF, ZF, SF, PF. */
#define OP_SIGNED_SUB(s_type, u_type, operand) { \
    OP_SIGNED_CMP_IMPL(s_type, u_type, operand) \
    f_CF = (u_type)_res > (u_type)_sav; \
    *(s_type *)dest = _res; \
}

/* `*dest -= operand`, updates AF, OF, ZF, SF, PF. */
#define OP_SIGNED_DEC(s_type, u_type, operand) { \
    OP_SIGNED_CMP_IMPL(s_type, u_type, operand) \
    *(s_type *)dest = _res; \
}

/* `*dest -= CF + operand`, updates AF, OF, CF, ZF, PF, SF, operands are signed. */
#define OP_SIGNED_SBB(s_type, u_type, operand) { \
    s_type _sav; \
    s_type _res; \
    if (f_CF) { \
        _sav = *(s_type *)dest; \
        _res = _sav - 1; \
        f_CF = (u_type)_res > (u_type)_sav; \
        f_AF = (uint8_t)_res > (uint8_t)_sav; \
        f_OF = _sav <  0 && _res >= 0; \
    } \
    _sav = *(s_type *)dest; \
    _res = _sav - (operand); \
    SET_RESULT_FLAGS(_res) \
    f_CF |= (u_type)_res > (u_type)_sav; \
    f_AF |= (uint8_t)_res > (uint8_t)_sav; \
    f_OF |= ((operand) >= 0 && _sav <  0 && _res >= 0) || \
           ((operand) <  0 && _sav >= 0 && _res <  0); \
    *(s_type *)dest = _res; \
}

/* `*dest = operand` */
#define OP_SIGNED_MOV(s_type, u_type, operand) { \
    *(s_type *)dest = operand; \
}


/* `*dest = operand` */
#define OP_UNSIGNED_MOV(s_type, u_type, operand) { \
    *(u_type *)dest = operand; \
}


/* `*dest = operand`, RCX/ECX times if rep specified. */
#define OP_UNSIGNED_MOV_REP(s_type, u_type, operand) { \
    u_type *tdest = (u_type *)dest; \
    if (!ins->rep) *tdest = operand; \
    else if (ins->address_sz) \
        for (; r_ecx; r_ecx--) \
            if (f_DF) *(tdest--) = operand; \
            else      *(tdest++) = operand; \
    else \
        for (; r_rcx; r_rcx--) \
            if (f_DF) *(tdest--) = operand; \
            else      *(tdest++) = operand; \
}

#define GET_OPERAND_SRC_REG      void *src = x64modrm_get_reg(emu, ins);
#define GET_OPERAND_SRC_R_M      void *src = x64modrm_get_r_m(emu, ins);
#define GET_OPERAND_SRC_XMM      void *src = x64modrm_get_xmm(emu, ins);
#define GET_OPERAND_SRC_XMM_M    void *src = x64modrm_get_xmm_m(emu, ins);
#define GET_OPERAND_SRC_GPR(reg) void *src = emu->regs + (reg);
#define GET_OPERAND_SRC_IMM      void *src = &ins->imm;
#define GET_OPERAND_SRC_NULL

#define GET_OPERAND_DEST_REG      void *dest = x64modrm_get_reg(emu, ins);
#define GET_OPERAND_DEST_R_M      void *dest = x64modrm_get_r_m(emu, ins);
#define GET_OPERAND_DEST_XMM      void *dest = x64modrm_get_xmm(emu, ins);
#define GET_OPERAND_DEST_XMM_M    void *dest = x64modrm_get_xmm_m(emu, ins);
#define GET_OPERAND_DEST_GPR(reg) void *dest = emu->regs + (reg);
#define GET_OPERAND_DEST_IMM      void *dest = &ins->imm;
#define GET_OPERAND_DEST_NULL

/* Perform specified operation on dest using src operands. */
#define DEST_OPERATION_16_32_64(operation, src64, src16, src32) \
    if      (ins->rex.w)      operation(int64_t, uint64_t, src64) \
    else if (ins->operand_sz) operation(int16_t, uint16_t, src16) \
    else                      operation(int32_t, uint32_t, src32)

/* Perform signed operation on dest with src operand, sign extending when needed. */
#define DEST_OPERATION_16_32_64_S_8(operation, src) \
    DEST_OPERATION_16_32_64(operation, (int64_t)(*(int8_t *)(src)), (int16_t)(*(int8_t *)(src)), (int32_t)(*(int8_t *)(src)))

#define DEST_OPERATION_16_32_64_S_16(operation, src) \
    DEST_OPERATION_16_32_64(operation, (int64_t)(*(int16_t *)(src)), *(int16_t *)(src), (int32_t)(*(int16_t *)(src)))

#define DEST_OPERATION_16_32_64_S_32(operation, src) \
    DEST_OPERATION_16_32_64(operation, (int64_t)(*(int32_t *)(src)), *(int16_t *)(src), *(int32_t *)(src))

#define DEST_OPERATION_16_32_64_S_64(operation, src) \
    DEST_OPERATION_16_32_64(operation, *(int64_t *)(src), *(int16_t *)(src), *(int32_t *)(src))

/* Perform unsigned operation on dest with src operand, zero-extending when needed. */
#define DEST_OPERATION_16_32_64_U_8(operation, src) \
    DEST_OPERATION_16_32_64(operation, (uint64_t)(*(uint8_t *)(src)), (uint16_t)(*(uint8_t *)(src)), (uint32_t)(*(uint8_t *)(src)))

#define DEST_OPERATION_16_32_64_U_16(operation, src) \
    DEST_OPERATION_16_32_64(operation, (uint64_t)(*(uint16_t *)(src)), *(uint16_t *)(src), (uint32_t)(*(uint16_t *)(src)))

#define DEST_OPERATION_16_32_64_U_64(operation, src) \
    DEST_OPERATION_16_32_64(operation, *(uint64_t *)(src), *(uint16_t *)(src), *(uint32_t *)(src))

#define DEST_OPERATION_16_32_64_U_32(operation, src) \
    DEST_OPERATION_16_32_64(operation, (uint64_t)(*(uint32_t *)(src)), *(uint16_t *)(src), *(uint32_t *)(src))


/* Combined macros */

/* Operation that uses 16, 32 or 64 bit destination size, and `ext` source operand type. */
#define OPERATION_16_32_64(desttype, srctype, operation, ext) { \
    GET_OPERAND_SRC_ ## srctype \
    GET_OPERAND_DEST_ ## desttype \
    DEST_OPERATION_16_32_64_ ## ext(operation, src) \
}

/* Operation that uses fixed type of destination and source operands. */
#define OPERATION_FIXED_S(desttype, srctype, operation, s_type, u_type) { \
    GET_OPERAND_SRC_ ## srctype \
    GET_OPERAND_DEST_ ## desttype \
    operation(s_type, u_type, *(s_type *)(src)) \
}

/* Operation that uses fixed type of destination and source operands. */
#define OPERATION_FIXED_U(desttype, srctype, operation, s_type, u_type) { \
    GET_OPERAND_SRC_ ## srctype \
    GET_OPERAND_DEST_ ## desttype \
    operation(s_type, u_type, *(u_type *)(src)) \
}

/* Operations modifying both src and dest.
   Cannot be used with previous macros,
   src operand is a pointer. */

#define PP_XCHG(s_type, u_type) { \
    u_type _sav = *(u_type *)dest; \
    *(u_type *)dest = *(u_type *)src; \
    *(u_type *)src = _sav; \
}

#define PP_OPERATION(desttype, srctype, operation) { \
    GET_OPERAND_SRC_ ## srctype \
    GET_OPERAND_DEST_ ## desttype \
    if      (ins->rex.w)      operation(int64_t, uint64_t) \
    else if (ins->operand_sz) operation(int16_t, uint16_t) \
    else                      operation(int32_t, uint32_t) \
}

#define PP_OPERATION_FIXED(desttype, srctype, operation, s_type, u_type) { \
    GET_OPERAND_SRC_ ## srctype \
    GET_OPERAND_DEST_ ## desttype \
    operation(s_type, u_type) \
}


/* XMM/MMX */


#define DEST_OPERATION_32_64(operation, src64, src32) \
    if (ins->rex.w) operation(int64_t, uint64_t, src64) \
    else            operation(int32_t, uint32_t, src32)


#define DEST_OPERATION_32_64_S_8(operation, src) \
    DEST_OPERATION_32_64(operation, (int64_t)(*(int8_t *)(src)), (int32_t)(*(int8_t *)(src)))

#define DEST_OPERATION_32_64_S_16(operation, src) \
    DEST_OPERATION_32_64(operation, (int64_t)(*(int16_t *)(src)), (int32_t)(*(int16_t *)(src)))

#define DEST_OPERATION_32_64_S_32(operation, src) \
    DEST_OPERATION_32_64(operation, (int64_t)(*(int32_t *)(src)), *(int32_t *)(src))

#define DEST_OPERATION_32_64_S_64(operation, src) \
    DEST_OPERATION_32_64(operation, *(int64_t *)(src), *(int32_t *)(src))

#define DEST_OPERATION_32_64_U_8(operation, src) \
    DEST_OPERATION_32_64(operation, (uint64_t)(*(uint8_t *)(src)), (uint32_t)(*(uint8_t *)(src)))

#define DEST_OPERATION_32_64_U_16(operation, src) \
    DEST_OPERATION_32_64(operation, (uint64_t)(*(uint16_t *)(src)), (uint32_t)(*(uint16_t *)(src)))

#define DEST_OPERATION_32_64_U_32(operation, src) \
    DEST_OPERATION_32_64(operation, (uint64_t)(*(uint32_t *)(src)), *(uint32_t *)(src))

#define DEST_OPERATION_32_64_U_64(operation, src) \
    DEST_OPERATION_32_64(operation, *(uint64_t *)(src), *(uint32_t *)(src))


#define OPERATION_32_64(desttype, srctype, operation, ext) { \
    GET_OPERAND_SRC_ ## srctype \
    GET_OPERAND_DEST_ ## desttype \
    DEST_OPERATION_32_64_ ## ext(operation, src) \
}


#include <stdbool.h>

#include "x64emu.h"
#include "x64instr.h"
#include "x64flags_private.h"

static inline bool x64execute_jmp_cond(x64emu_t *emu, x64instr_t *ins, uint8_t op) {
    bool ret = false;
    switch (op & 0xF) {
        case 0x0: ret =  f_OF;                 break;
        case 0x1: ret = !f_OF;                 break;
        case 0x2: ret =  f_CF;                 break;
        case 0x3: ret = !f_CF;                 break;
        case 0x4: ret =  f_ZF;                 break;
        case 0x5: ret = !f_ZF;                 break;
        case 0x6: ret =  f_CF ||  f_ZF;        break;
        case 0x7: ret = !f_CF && !f_ZF;        break;
        case 0x8: ret =  f_SF;                 break;
        case 0x9: ret = !f_SF;                 break;
        case 0xA: ret =  f_PF;                 break;
        case 0xB: ret = !f_PF;                 break;
        case 0xC: ret =  f_SF != f_OF;         break;
        case 0xD: ret =  f_SF == f_OF;         break;
        case 0xE: ret =  f_ZF || f_SF != f_OF; break;
        case 0xF: ret = !f_ZF && f_SF == f_OF; break;
    }
    return ret;
}

#endif /* __X64EXECUTE_PRIVATE_H_ */