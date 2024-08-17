import sys
import re

src = ""

for filename in [
    'room.xbm', 'ullr.xbm', 'urll.xbm', 'ul.xbm', 'lr.xbm', 'ur.xbm', 'll.xbm'
]:
    with open(filename, "r") as f:
        src = src + f.read() 

mapping = [
    #  0: Room
    *range(16),
    # 16: Passage ULLR
    16, 17, 18, 20, 21, 22, 23, 24, 25, 26, 27, 29, 30, 31,
    # 30: Passage URLL
    33, 34,
    # 32: space
    19,
    # 33: Passage URLL continued
    *range(35, 48),
    # Wall UL, LR
    55, 58, 71, 74,
    # Wall UR, LL
    86, 91, 102, 107
]

tiles_per_row = 4

# reverses the bits, from bit twiddling hacks
reverse = lambda b: (((b * 0x0802 & 0x22110) | (b * 0x8020 & 0x88440)) * 0x10101 >> 16) & 0xFF

def convert(tile_number):
    offset = (tile_number // tiles_per_row * tiles_per_row * 8) + \
        (tile_number % tiles_per_row)
    for y in range(8):
        v = data[offset]
        v = reverse(v)
        yield v
        offset = offset + tiles_per_row

data = [int(x[2:], base=16) for x in re.findall('0x[0-9A-Fa-f][0-9A-Fa-f]', src)]

print("""\
#include <stdint.h>

#pragma data-name("CHARMEM")

uint8_t graphics_data[] = {
""")

print(",\n".join( \
    (", ".join(hex(v) for v in convert(n)) for n in mapping)
))

print("""\
};
""")
