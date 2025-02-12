#ifndef __VIRTUAL_H_
#define __VIRTUAL_H_

#include <stdbool.h>
#include <stddef.h>

/**
 * @return whether virtual address space is bigger than 39 bits.
 */
bool detect_48bit_va(void);

bool dump_self_maps(void);

#endif /* __VIRTUAL_H_ */