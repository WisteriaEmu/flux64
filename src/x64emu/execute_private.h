#ifndef __X64EXECUTE_PRIVATE_H_
#define __X64EXECUTE_PRIVATE_H_

#include <stdint.h>

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

/* CF (shift>0) = last bit shifted out,
   OF (shift=1) = CF xor MSB of dest. */
#define IMPL_ROL_FLAG(s_type, u_type, operand) \
    if (shift > 0) { \
        f_CF = (tdest >> (type_len - shift)) & 1; \
        if (shift == 1) f_OF = f_CF ^ (*(u_type *)dest >> (type_len - 1)); \
    }

/* CF (shift>0) = last bit shifted out,
   OF (shift=1) = MSB xor MSB-1 (of dest). */
#define IMPL_ROR_FLAG(s_type, u_type, operand) \
    if (shift > 0) { \
        f_CF = (tdest >> (shift - 1)) & 1; \
        if (shift == 1) f_OF = ((*(u_type *)dest >> (type_len - 2)) & 1) ^ \
                                (*(u_type *)dest >> (type_len - 1)); \
    }

/* Same as rotating, but sets SF, ZF, PF. */

#define IMPL_SHL_FLAG(s_type, u_type, operand) \
    if (shift > 0) { \
        f_CF = (tdest >> (type_len - shift)) & 1; \
        SET_RESULT_FLAGS(*(s_type *)dest) \
        if (shift == 1) f_OF = f_CF ^ (*(u_type *)dest >> (type_len - 1)); \
    }

#define IMPL_SHR_FLAG(s_type, u_type, operand) \
    if (shift > 0) { \
        f_CF = (tdest >> (shift - 1)) & 1; \
        SET_RESULT_FLAGS(*(s_type *)dest) \
        if (shift == 1) f_OF = ((*(u_type *)dest >> (type_len - 2)) & 1) ^ \
                                (*(u_type *)dest >> (type_len - 1)); \
    }

#define IMPL_SHIFT_PROLOGUE(s_type, u_type, operand) \
    uint8_t type_len = sizeof(u_type) * 8; \
    uint8_t shift = (operand) & (type_len - 1); \
    u_type tdest = *(u_type *)dest;


/** Rotate left. Updates CF(shift>0), OF(shift=1). Operand is unsigned. */
#define OP_ROL(s_type, u_type, operand) { \
    IMPL_SHIFT_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest = (tdest << shift) | (tdest >> (type_len - shift)); \
    IMPL_ROL_FLAG(s_type, u_type, operand) \
}

/** Rotate right. Updates CF(shift>0), OF(shift=1). Operand is unsigned. */
#define OP_ROR(s_type, u_type, operand) { \
    IMPL_SHIFT_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest = (tdest >> shift) | (tdest << (type_len - shift)); \
    IMPL_ROR_FLAG(s_type, u_type, operand) \
}

/** Rotate left with carry. Updates CF(shift>0), OF(shift=1). Operand is unsigned. */
#define OP_RCL(s_type, u_type, operand) { \
    IMPL_SHIFT_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest = (tdest << shift) | (tdest >> (type_len + 1 - shift)) | \
                      ((shift) ? (f_CF << (shift - 1)) : 0); \
    IMPL_ROL_FLAG(s_type, u_type, operand) \
}

/** Rotate right with carry. Updates CF(shift>0), OF(shift=1). Operand is unsigned */
#define OP_RCR(s_type, u_type, operand) { \
    IMPL_SHIFT_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest = (tdest >> shift) | (tdest << (type_len + 1 - shift)) | \
                      (f_CF << (type_len - shift)); \
    IMPL_ROR_FLAG(s_type, u_type, operand) \
}

/** `*dest <<= operand`; updates CF(shift>0), OF(shift=1), SF, ZF, PF; operand is unsigned. */
#define OP_SHL(s_type, u_type, operand) { \
    IMPL_SHIFT_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest <<= shift; \
    IMPL_SHL_FLAG(s_type, u_type, operand) \
}

/** `*dest >>= operand`; updates CF(shift>0), OF(shift=1), SF, ZF, PF; operand is unsigned. */
#define OP_SHR(s_type, u_type, operand) { \
    IMPL_SHIFT_PROLOGUE(s_type, u_type, operand) \
    *(u_type *)dest >>= shift; \
    IMPL_SHR_FLAG(s_type, u_type, operand) \
}

/* FIXME: signed type shifts are implementation-defined. */
/** Signed `*dest >>= operand`; updates CF(shift>0), OF(shift=1), SF, ZF, PF; operand is unsigned. */
#define OP_SAR(s_type, u_type, operand) { \
    IMPL_SHIFT_PROLOGUE(s_type, s_type, operand) \
    *(s_type *)dest >>= shift; \
    IMPL_SHR_FLAG(s_type, u_type, operand) \
}


/* NOTE: state of AF is undefined. */
#define IMPL_OP_S_BITWISE(oper, s_type, u_type, operand) { \
    *(s_type *)dest oper ## = (operand); \
    SET_RESULT_FLAGS(*(s_type *)dest) \
    f_CF = f_OF = 0; \
}

#define IMPL_OP_S_TEST(oper, s_type, u_type, operand) { \
    s_type _res = (*(s_type *)dest) oper (operand); \
    SET_RESULT_FLAGS(_res) \
    f_CF = f_OF = 0; \
}


/** `*dest ^= operand`. Updates ZF, PF, SF. Operands are signed. */
#define OP_S_XOR(s_type, u_type, operand) \
    IMPL_OP_S_BITWISE(^, s_type, u_type, operand)


/** `*dest &= operand`. Updates ZF, PF, SF. Operands are signed. */
#define OP_S_AND(s_type, u_type, operand) \
    IMPL_OP_S_BITWISE(&, s_type, u_type, operand)


/** Update ZF, PF, SF with result of `*dest & operand`. Operands are signed. */
#define OP_S_TEST_AND(s_type, u_type, operand) \
    IMPL_OP_S_TEST(&, s_type, u_type, operand)


/** `*dest |= operand`. Updates ZF, PF, SF. Operands are signed. */
#define OP_S_OR(s_type, u_type, operand) \
    IMPL_OP_S_BITWISE(|, s_type, u_type, operand)

/** `*dest = ~(*dest)`. */
#define OP_S_NOT(s_type, u_type, operand) \
    *(s_type *)dest = ~(*(s_type *)dest);

/** `*dest = -(*dest)`, signed. */
#define OP_S_NEG(s_type, u_type, operand) \
    *(s_type *)dest = -(*(s_type *)dest);


#define IMPL_S_ADD(s_type, u_type, operand) \
    s_type _sav = *(s_type *)dest; \
    s_type _res = _sav + (operand); \
    SET_RESULT_FLAGS(_res) \
    /* CF is set in other macro */ \
    f_AF = (uint8_t)_res < (uint8_t)_sav; \
    f_OF = ((operand) <  0 && _sav <  0 && _res >= 0) || \
           ((operand) >= 0 && _sav >= 0 && _res <  0);


/** `*dest += operand`. Updates AF, OF, CF, ZF, PF, SF. Operands are signed. */
#define OP_S_ADD(s_type, u_type, operand) { \
    IMPL_S_ADD(s_type, u_type, operand) \
    f_CF = (u_type)_res < (u_type)_sav; \
    *(s_type *)dest = _res; \
}

/** `*dest += operand`. Updates AF, OF, ZF, PF, SF. Operands are signed. */
#define OP_S_INC(s_type, u_type, operand) { \
    IMPL_S_ADD(s_type, u_type, operand) \
    *(s_type *)dest = _res; \
}

/* Probably bad. */

/** `*dest += CF + operand`. Updates AF, OF, CF, ZF, PF, SF. Operands are signed. */
#define OP_S_ADC(s_type, u_type, operand) { \
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


#define IMPL_S_CMP(s_type, u_type, operand) \
    s_type _sav = *(s_type *)dest; \
    s_type _res = _sav - (operand); \
    SET_RESULT_FLAGS(_res) \
    /* CF is set in other macro */ \
    f_AF = (uint8_t)_res > (uint8_t)_sav; \
    f_OF = ((operand) >= 0 && _sav <  0 && _res >= 0) || \
           ((operand) <  0 && _sav >= 0 && _res <  0);


/** Updates AF, OF, CF, ZF, SF, PF with result of `*dest - operand`. Operands are signed. */
#define OP_S_CMP(s_type, u_type, operand) { \
    IMPL_S_CMP(s_type, u_type, operand) \
    f_CF = (u_type)_res > (u_type)_sav; \
}

/** `*dest -= operand`. Updates AF, OF, CF, ZF, SF, PF. Operands are signed. */
#define OP_S_SUB(s_type, u_type, operand) { \
    IMPL_S_CMP(s_type, u_type, operand) \
    f_CF = (u_type)_res > (u_type)_sav; \
    *(s_type *)dest = _res; \
}

/** `*dest -= operand`. Updates AF, OF, ZF, SF, PF. Operands are signed. */
#define OP_S_DEC(s_type, u_type, operand) { \
    IMPL_S_CMP(s_type, u_type, operand) \
    *(s_type *)dest = _res; \
}

/** `*dest -= CF + operand`. Updates AF, OF, CF, ZF, PF, SF. Operands are signed. */
#define OP_S_SBB(s_type, u_type, operand) { \
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

/** `*dest = operand`. Operands are signed. */
#define OP_S_MOV(s_type, u_type, operand) { \
    *(s_type *)dest = operand; \
}


/** `*dest = operand`. Operands are unsigned. */
#define OP_U_MOV(s_type, u_type, operand) { \
    *(u_type *)dest = operand; \
}


/** `*dest = operand`, RCX/ECX times if rep specified. */
#define OP_U_MOV_REP(s_type, u_type, operand) { \
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

/* Get the first src operand for operation. */
#define GET_OP_SRC1_REG      void *src = x64modrm_get_reg(emu, ins);
#define GET_OP_SRC1_R_M      void *src = x64modrm_get_r_m(emu, ins);
#define GET_OP_SRC1_XMM      void *src = x64modrm_get_xmm(emu, ins);
#define GET_OP_SRC1_XMM_M    void *src = x64modrm_get_xmm_m(emu, ins);
#define GET_OP_SRC1_GPR(reg) void *src = emu->regs + (reg);
#define GET_OP_SRC1_IMM      void *src = &ins->imm;
#define GET_OP_SRC1_NULL
/* Get the destination operand for operation. */
#define GET_OP_DEST_REG      void *dest = x64modrm_get_reg(emu, ins);
#define GET_OP_DEST_R_M      void *dest = x64modrm_get_r_m(emu, ins);
#define GET_OP_DEST_XMM      void *dest = x64modrm_get_xmm(emu, ins);
#define GET_OP_DEST_XMM_M    void *dest = x64modrm_get_xmm_m(emu, ins);
#define GET_OP_DEST_GPR(reg) void *dest = emu->regs + (reg);
#define GET_OP_DEST_IMM      void *dest = &ins->imm;
#define GET_OP_DEST_NULL

/* Perform specified 2 operand operation on dest. */
#define DEST_OP2_16_32_64(operation, src64, src16, src32) \
    if      (ins->rex.w)      operation(int64_t, uint64_t, src64) \
    else if (ins->operand_sz) operation(int16_t, uint16_t, src16) \
    else                      operation(int32_t, uint32_t, src32)

/* Perform signed operation on dest with src operand, sign extending when needed. */
#define DEST_OP2_16_32_64_S_8(operation, src) \
    DEST_OP2_16_32_64(operation, (int64_t)(*(int8_t *)(src)), (int16_t)(*(int8_t *)(src)), (int32_t)(*(int8_t *)(src)))

#define DEST_OP2_16_32_64_S_16(operation, src) \
    DEST_OP2_16_32_64(operation, (int64_t)(*(int16_t *)(src)), *(int16_t *)(src), (int32_t)(*(int16_t *)(src)))

#define DEST_OP2_16_32_64_S_32(operation, src) \
    DEST_OP2_16_32_64(operation, (int64_t)(*(int32_t *)(src)), *(int16_t *)(src), *(int32_t *)(src))

#define DEST_OP2_16_32_64_S_64(operation, src) \
    DEST_OP2_16_32_64(operation, *(int64_t *)(src), *(int16_t *)(src), *(int32_t *)(src))

/* Perform unsigned operation on dest with src operand, zero-extending when needed. */
#define DEST_OP2_16_32_64_U_8(operation, src) \
    DEST_OP2_16_32_64(operation, (uint64_t)(*(uint8_t *)(src)), (uint16_t)(*(uint8_t *)(src)), (uint32_t)(*(uint8_t *)(src)))

#define DEST_OP2_16_32_64_U_16(operation, src) \
    DEST_OP2_16_32_64(operation, (uint64_t)(*(uint16_t *)(src)), *(uint16_t *)(src), (uint32_t)(*(uint16_t *)(src)))

#define DEST_OP2_16_32_64_U_64(operation, src) \
    DEST_OP2_16_32_64(operation, *(uint64_t *)(src), *(uint16_t *)(src), *(uint32_t *)(src))

#define DEST_OP2_16_32_64_U_32(operation, src) \
    DEST_OP2_16_32_64(operation, (uint64_t)(*(uint32_t *)(src)), *(uint16_t *)(src), *(uint32_t *)(src))


/* Combined macros */

/* Operation that uses 16, 32 or 64 bit destination size, and `ext` source operand type. */
#define OP2_16_32_64(desttype, srctype, operation, ext) { \
    GET_OP_SRC1_ ## srctype \
    GET_OP_DEST_ ## desttype \
    DEST_OP2_16_32_64_ ## ext(operation, src) \
}

/* Operation that uses fixed type of destination and source operands. */
#define OP2_FIXED_S(desttype, srctype, operation, s_type, u_type) { \
    GET_OP_SRC1_ ## srctype \
    GET_OP_DEST_ ## desttype \
    operation(s_type, u_type, *(s_type *)(src)) \
}

/* Operation that uses fixed type of destination and source operands. */
#define OP2_FIXED_U(desttype, srctype, operation, s_type, u_type) { \
    GET_OP_SRC1_ ## srctype \
    GET_OP_DEST_ ## desttype \
    operation(s_type, u_type, *(u_type *)(src)) \
}

/* Operations modifying both src and dest.
   Cannot be used with previous macros,
   src operand is a pointer. */

#define OP_XCHG(s_type, u_type) { \
    u_type _sav = *(u_type *)dest; \
    *(u_type *)dest = *(u_type *)src; \
    *(u_type *)src = _sav; \
}

#define PP_OP2_16_32_64(desttype, srctype, operation) { \
    GET_OP_SRC1_ ## srctype \
    GET_OP_DEST_ ## desttype \
    if      (ins->rex.w)      operation(int64_t, uint64_t) \
    else if (ins->operand_sz) operation(int16_t, uint16_t) \
    else                      operation(int32_t, uint32_t) \
}

#define PP_OP2_FIXED(desttype, srctype, operation, s_type, u_type) { \
    GET_OP_SRC1_ ## srctype \
    GET_OP_DEST_ ## desttype \
    operation(s_type, u_type) \
}


/* XMM/MMX */


#define DEST_OP2_32_64(operation, src64, src32) \
    if (ins->rex.w) operation(int64_t, uint64_t, src64) \
    else            operation(int32_t, uint32_t, src32)


#define DEST_OP2_32_64_S_8(operation, src) \
    DEST_OP2_32_64(operation, (int64_t)(*(int8_t *)(src)), (int32_t)(*(int8_t *)(src)))

#define DEST_OP2_32_64_S_16(operation, src) \
    DEST_OP2_32_64(operation, (int64_t)(*(int16_t *)(src)), (int32_t)(*(int16_t *)(src)))

#define DEST_OP2_32_64_S_32(operation, src) \
    DEST_OP2_32_64(operation, (int64_t)(*(int32_t *)(src)), *(int32_t *)(src))

#define DEST_OP2_32_64_S_64(operation, src) \
    DEST_OP2_32_64(operation, *(int64_t *)(src), *(int32_t *)(src))

#define DEST_OP2_32_64_U_8(operation, src) \
    DEST_OP2_32_64(operation, (uint64_t)(*(uint8_t *)(src)), (uint32_t)(*(uint8_t *)(src)))

#define DEST_OP2_32_64_U_16(operation, src) \
    DEST_OP2_32_64(operation, (uint64_t)(*(uint16_t *)(src)), (uint32_t)(*(uint16_t *)(src)))

#define DEST_OP2_32_64_U_32(operation, src) \
    DEST_OP2_32_64(operation, (uint64_t)(*(uint32_t *)(src)), *(uint32_t *)(src))

#define DEST_OP2_32_64_U_64(operation, src) \
    DEST_OP2_32_64(operation, *(uint64_t *)(src), *(uint32_t *)(src))


#define OP_32_64(desttype, srctype, operation, ext) { \
    GET_OP_SRC1_ ## srctype \
    GET_OP_DEST_ ## desttype \
    DEST_OP2_32_64_ ## ext(operation, src) \
}


#include <stdbool.h>

#include "x64emu.h"
#include "x64instr.h"

#include "flags_private.h"

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