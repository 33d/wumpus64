#if !defined(SYS_C64_H)
#define SYS_C64_H

#include <stdint.h>

/// The screen memory; defined in the linker script
extern uint8_t screenmem[80*25];

/// The sprite data; defined in the linker script
extern uint8_t spritedata[24*21*8];

#endif
