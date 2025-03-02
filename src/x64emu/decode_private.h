#ifndef __X64DECODE_PRIVATE_H_
#define __X64DECODE_PRIVATE_H_

#define FETCH_IMM_8() ins->imm.ub[0] = fetch_8(emu, ins);
#define FETCH_IMM_16() ins->imm.uw[0] = fetch_16(emu, ins);
#define FETCH_IMM_32() ins->imm.ud[0] = fetch_32(emu, ins);
#define FETCH_IMM_64() ins->imm.uq[0] = fetch_64(emu, ins);

/** Fetch 16 or 32 bits based on 66H prefix. */
#define FETCH_IMM_16_32() \
    if (ins->operand_sz) FETCH_IMM_16() \
    else FETCH_IMM_32()

/** Fetch 16, 32 or 64 bits based on REX.W and 66H prefix. */
#define FETCH_IMM_16_32_64() \
    if (ins->rex.w) FETCH_IMM_64() \
    else if (ins->operand_sz) FETCH_IMM_16() \
    else FETCH_IMM_32()


#endif /* __X64DECODE_PRIVATE_H_ */