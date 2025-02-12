#ifndef __X64STACK_PRIVATE_H_
#define __X64STACK_PRIVATE_H_

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/auxv.h>
#include <errno.h>

#include "x64emu.h"
#include "x64regs_private.h"

// stack operations

static inline void push_32(x64emu_t *emu, uint32_t v) {
    *(uint32_t *)(r_rsp -= 4) = v;
}

static inline void push_64(x64emu_t *emu, uint64_t v) {
    *(uint64_t *)(r_rsp -= 8) = v;
}

/**
 * Push auxiliary vector of type `t` and value `v`.
 */
static inline void push_auxv(x64emu_t *emu, uint64_t v, uint64_t t) {
    push_64(emu, v);
    push_64(emu, t);
}

/**
 * If the system provides auxiliary vector of type `t`, then push it to the stack.
 */
static inline void push_real_auxv(x64emu_t *emu, uint64_t t) {
    uint64_t v = getauxval(t);
    if (errno != ENOENT) push_auxv(emu, v, t);
}

/**
 * Push `0` sequence so that `r_rsp` is aligned.
 */
static inline void push_align(x64emu_t *emu) {
    uint64_t aligned = r_rsp & ~(emu->ctx->stack.align - 1);
    memset((void *)aligned, 0, r_rsp - aligned);
    r_rsp = aligned;
}

/**
 * Push string characters to the stack.
 */
static inline void push_string(x64emu_t *emu, const char *str) {
    int size = strlen(str) + 1; // NULL-terminated size
    r_rsp -= size;
    memcpy((void*)r_rsp, str, size);
}

/**
 * Pop qword from top of the stack (at rsp).
 */
static inline uint64_t pop_64(x64emu_t *emu) {
    return *(uint64_t *)(r_rsp += 8, r_rsp - 8);
}


#endif /* __X64STACK_PRIVATE_H_ */