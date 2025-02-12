#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "x64context.h"
#include "debug.h"

SET_DEBUG_CHANNEL("X64CONTEXT")

static bool segments_free(x64context_t *ctx) {
    if (!ctx || !ctx->segments_len) return true;

    for (int i = 0; i < ctx->segments_len; i++) {
        log_dump("Unmapping 0x%lx, size 0x%lx", (uintptr_t)ctx->segments[i].base, ctx->segments[i].size);
        if (munmap(ctx->segments[i].base, ctx->segments[i].size) != 0) {
            log_err("Failed to unmap segment: %s", strerror(errno));
            return false;
        }
    }
    free(ctx->segments);
    ctx->segments_len = 0;
    return true;
}

bool x64context_init(x64context_t *ctx, int argc, char *argv[], char *envp[]) {
    int envc = 0;
    while (envp[++envc]) {}

    /* first argument is emulator binary, remove it. */
    ctx->argc = argc - 1;
    ctx->argv = argv + 1;
    ctx->envc = envc;
    ctx->envv = envp;
    ctx->page_size = sysconf(_SC_PAGESIZE);

    log_debug("Set up context with %d args, %d environment variables, 0x%lx host page size",
                ctx->argc, ctx->envc, ctx->page_size);

    if (!x64stack_init(ctx)) return false;

    return true;
}

bool x64context_free(x64context_t *ctx) {
    /* try to free everything and report error at the end. */

    bool ret = true;

    if (!x64stack_free(ctx)) ret = false;

    if (!segments_free(ctx)) ret = false;

    return ret;
}