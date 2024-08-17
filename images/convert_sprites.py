import sys
import re

src = ""

for filename in [
    'player_fg.xbm', 'player_bg.xbm'
]:
    with open(filename, "r") as f:
        src = src + f.read() 

# reverses the bits, from bit twiddling hacks
reverse = lambda b: (((b * 0x0802 & 0x22110) | (b * 0x8020 & 0x88440)) * 0x10101 >> 16) & 0xFF

data = [int(x[2:], base=16) for x in re.findall('0x[0-9A-Fa-f][0-9A-Fa-f]', src)]

def convert(filename):
    with open(filename, "r") as f:
        src = f.read() 
    data = [int(x[2:], base=16) for x in re.findall('0x[0-9A-Fa-f][0-9A-Fa-f]', src)]
    return ", ".join(hex(reverse(v)) for v in data)

print("""\
#include <stdint.h>

#pragma data-name("SPRITEDATA")

uint8_t sprite_data[] = {
""")

print(convert("player_fg.xbm") + ", 0,")
print(convert("player_bg.xbm") + ", 0,")

print("""\
};
""")
