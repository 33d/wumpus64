#if !defined(SYS_H)
#define SYS_H

// Platform specific stuff that doesn't belong anywhere else

#include <stdint.h>

void sys_init();
uint_fast16_t sys_random_seed();

#endif
