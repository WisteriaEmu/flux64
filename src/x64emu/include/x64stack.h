#ifndef __X64STACK_H_
#define __X64STACK_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "x64context.h"
#include "x64emu.h"

/** Map the initial stack. */
bool x64stack_init(x64context_t *ctx);

/** Unmap the stack. */
bool x64stack_free(x64context_t *ctx);

/** Push initial data to the stack. */
void x64stack_setup(x64emu_t *emu);

/** Push word to the stack. */
void push_16(x64emu_t *emu, uint16_t v);

/** Push dword to the stack. */
void push_32(x64emu_t *emu, uint32_t v);

/** Push qword to the stack. */
void push_64(x64emu_t *emu, uint64_t v);

/** Push auxiliary vector of type `t` and value `v`. */
void push_auxv(x64emu_t *emu, uint64_t v, uint64_t t);

/**
 * If the system provides auxiliary vector of type `t`, then push it to the stack.
 */
void push_real_auxv(x64emu_t *emu, uint64_t t);

/** Push `0` sequence so that `r_rsp` is aligned. */
void push_align(x64emu_t *emu);

/** Push string characters to the stack. */
void push_string(x64emu_t *emu, const char *str);

/** Pop word from top of the stack (at rsp). */
uint16_t pop_16(x64emu_t *emu);

/** Pop dword from top of the stack (at rsp). */
uint32_t pop_32(x64emu_t *emu);

/** Pop qword from top of the stack (at rsp). */
uint64_t pop_64(x64emu_t *emu);

#endif /* __X64STACK_H_ */