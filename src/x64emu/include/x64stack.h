#ifndef __X64STACK_H_
#define __X64STACK_H_

#include <stdbool.h>

#include "x64context.h"
#include "x64emu.h"

/** Map the initial stack. */
bool x64stack_init(x64context_t *ctx);

/** Unmap the stack. */
bool x64stack_free(x64context_t *ctx);

/** Push initial data to the stack. */
void x64stack_setup(x64emu_t *emu);

#endif /* __X64STACK_H_ */