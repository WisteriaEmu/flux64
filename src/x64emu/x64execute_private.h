
#ifndef __EXECUTE_PRIVATE_H_
#define __EXECUTE_PRIVATE_H_

/* Set SF, ZF, PF. */
#define SET_RESULT_FLAGS(x) \
    f_SF = (x) < 0; \
    f_ZF = (x) == 0; \
    f_PF = !__builtin_parity((uint8_t)(x));


/* Perform bitwise operation and update flags. */
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


/* `*dest ^= operand`, update flags. */
#define OP_BITWISE_XOR(s_type, u_type, operand) \
    OP_BITWISE_IMPL(^, s_type, u_type, operand)


/* `*dest &= operand`, update flags. */
#define OP_BITWISE_AND(s_type, u_type, operand) \
    OP_BITWISE_IMPL(&, s_type, u_type, operand)


/* Same as AND, but discarding result. */
#define OP_BITWISE_TEST_AND(s_type, u_type, operand) \
    OP_BITWISE_TEST_IMPL(&, s_type, u_type, operand)


/* `*dest |= operand`, update flags. */
#define OP_BITWISE_OR(s_type, u_type, operand) \
    OP_BITWISE_IMPL(|, s_type, u_type, operand)


#define OP_SIGNED_ADD_IMPL(s_type, u_type, operand) \
    s_type _sav = *(s_type *)dest; \
    s_type _res = _sav + (operand); \
    SET_RESULT_FLAGS(_res) \
    /* CF is set in other macro */ \
    f_AF = (uint8_t)_res < (uint8_t)_sav; \
    f_OF = ((operand) <  0 && _sav <  0 && _res >= 0) || \
            ((operand) >= 0 && _sav >= 0 && _res <  0);


/* `*dest += operand`, update flags.
   CF, AF as unsigned, OF as signed.
   operand should be signed type. */
#define OP_SIGNED_ADD(s_type, u_type, operand) { \
    OP_SIGNED_ADD_IMPL(s_type, u_type, operand) \
    f_CF = (u_type)_res < (u_type)_sav; \
    *(s_type *)dest = _res; \
}

/* Same as add, but preserves CF. */
#define OP_SIGNED_INC(s_type, u_type, operand) { \
    OP_SIGNED_ADD_IMPL(s_type, u_type, operand) \
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


/* Same as SUB, but discarding result. */
#define OP_SIGNED_CMP(s_type, u_type, operand) { \
    OP_SIGNED_CMP_IMPL(s_type, u_type, operand) \
    f_CF = (u_type)_res > (u_type)_sav; \
}


/* `*dest -= operand`, update flags.
   CF, AF as unsigned, OF as signed.
   operand should be signed type. */
#define OP_SIGNED_SUB(s_type, u_type, operand) { \
    OP_SIGNED_CMP_IMPL(s_type, u_type, operand) \
    f_CF = (u_type)_res > (u_type)_sav; \
    *(s_type *)dest = _res; \
}

/* Same as sub, but preserves CF. */
#define OP_SIGNED_DEC(s_type, u_type, operand) { \
    OP_SIGNED_CMP_IMPL(s_type, u_type, operand) \
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


/* dest: r/m, src: reg, from ModR/M byte fields. */
#define GET_DEST_RM_SRC_REG() \
    void *src  = x64modrm_get_reg(emu, ins); \
    void *dest = x64modrm_get_rm(emu, ins);

/* dest: reg, src: r/m, from ModR/M byte fields. */
#define GET_DEST_REG_SRC_RM() \
    void *src  = x64modrm_get_rm(emu, ins); \
    void *dest = x64modrm_get_reg(emu, ins);

/* Perform specified operation on dest using src operands. */
#define DEST_OPERATION(operation, src64, src16, src32) \
    if      (ins->rex.w)      operation(int64_t, uint64_t, src64) \
    else if (ins->operand_sz) operation(int16_t, uint16_t, src16) \
    else                      operation(int32_t, uint32_t, src32)


/* Perform signed operation on dest with src operand, always sign-extending.
    NOTE: src is a pointer */
#define DEST_OPERATION_S_8(operation, src) \
    DEST_OPERATION(operation, (int64_t)(*(int8_t *)(src)), (int16_t)(*(int8_t *)(src)), (int32_t)(*(int8_t *)(src)))


/* Perform unsigned operation on dest with src operand, always sign-extending.
   NOTE: src is a pointer */
#define DEST_OPERATION_U_8(operation, src) \
    DEST_OPERATION(operation, (uint64_t)(*(uint8_t *)(src)), (uint16_t)(*(uint8_t *)(src)), (uint32_t)(*(uint8_t *)(src)))


/* Perform signed operation on dest with src operand, always sign-extending.
    NOTE: src is a pointer */
#define DEST_OPERATION_S_16(operation, src) \
    DEST_OPERATION(operation, (int64_t)(*(int16_t *)(src)), *(int16_t *)(src), (int32_t)(*(int16_t *)(src)))


/* Perform unsigned operation on dest with src operand, always sign-extending.
   NOTE: src is a pointer */
#define DEST_OPERATION_U_16(operation, src) \
    DEST_OPERATION(operation, (uint64_t)(*(uint16_t *)(src)), *(uint16_t *)(src), (uint32_t)(*(uint16_t *)(src)))


/* Perform signed operation on dest with src operand, without extension.
   NOTE: src is a pointer */
#define DEST_OPERATION_S_64(operation, src) \
    DEST_OPERATION(operation, *(int64_t *)(src), *(int16_t *)(src), *(int32_t *)(src))

/* Perform unsigned operation on dest with src operand, without extension.
   NOTE: src is a pointer */
#define DEST_OPERATION_U_64(operation, src) \
    DEST_OPERATION(operation, *(uint64_t *)(src), *(uint16_t *)(src), *(uint32_t *)(src))


/* Perform signed operation on dest with src operand, sign-extending when 64 bit dest.
   NOTE: src is a pointer */
#define DEST_OPERATION_S_32(operation, src) \
    DEST_OPERATION(operation, (int64_t)(*(int32_t *)(src)), *(int16_t *)(src), *(int32_t *)(src))

/* Perform unsigned operation on dest with src operand, zero-extending when 64 bit dest.
   NOTE: src is a pointer */
#define DEST_OPERATION_U_32(operation, src) \
    DEST_OPERATION(operation, (uint64_t)(*(uint32_t *)(src)), *(uint16_t *)(src), *(uint32_t *)(src))


/* Combined macros */

/* Operation that uses 16, 32 or 64 bit destination size, and `ext` source operand size. */
#define OPERATION_16_32_64(operandtype, operation, ext) { \
    GET_ ## operandtype() \
    DEST_OPERATION_ ## ext(operation, src) \
}

/* Operation that uses fixed type of destination and source operands. */
#define OPERATION_FIXED_S(operandtype, operation, s_type, u_type) { \
    GET_ ## operandtype() \
    operation(s_type, u_type, *(s_type *)(src)) \
}

/* Operation that uses fixed type of destination and source operands. */
#define OPERATION_FIXED_U(operandtype, operation, s_type, u_type) { \
    GET_ ## operandtype() \
    operation(s_type, u_type, *(u_type *)(src)) \
}


#include <stdbool.h>
#include <stdint.h>

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

#endif /* __EXECUTE_PRIVATE_H_ */