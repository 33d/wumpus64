#if !defined(SYS_C64_H)
#define SYS_C64_H

#include <stdint.h>

/// The screen memory; defined in the linker script
extern uint8_t screenmem[80*25];

/// The sprite data; defined in the linker script
extern const uint8_t spritedata[24*21*8];

/// Bits 4-1 of the character memory select register
extern const uint8_t* _charmem_reg;
// You can only take the address of a linker variable
#define charmem_reg ((const uint8_t) &_charmem_reg)

extern const uint8_t* _spritedata_reg;
#define spritedata_reg ((const uint8_t) &_spritedata_reg)

#endif
