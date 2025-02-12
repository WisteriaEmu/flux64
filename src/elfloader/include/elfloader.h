#ifndef __ELFLOADER_H_
#define __ELFLOADER_H_

#include <stdbool.h>

#include "x64context.h"

/**
 * Load the binary headers, map segments, set entry point.
 */
bool elfloader_load(x64context_t *ctx, char *path);

#endif /* __ELFLOADER_H_ */
