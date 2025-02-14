#ifndef __X64CONTEXT_H_
#define __X64CONTEXT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * Mapped segment of emulated binary.
 */
typedef struct {
    void*    base; /* start of the segment */
    size_t   size;
} segment_t;

typedef struct {
    void*    base; /* start of the mapped stack */
    size_t   size;
    size_t   align;
} x64stack_t;

/**
 * The context that the emulated binary is running in.
 */
typedef struct {
    uintptr_t     entry; /* entry point address, set when loading elf. */

    x64stack_t    stack;

    segment_t    *segments;
    uint32_t      segments_len;

    /* emulated argc, argv, envp to be pushed to the stack */
    int           argc;
    char**        argv;
    int           envc;
    char**        envv;

    long          page_size; /* host page size */
} x64context_t;

/**
 * Initialize context and stack.
 */
bool x64context_init(x64context_t *ctx, int argc, char *argv[], char *envp[]);

/**
 * Unload segments and destroy the stack.
 */
bool x64context_free(x64context_t *ctx);

#endif /* __X64_CONTEXT_H_ */
