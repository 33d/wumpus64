import sys
import re

# reverses the bits, from bit twiddling hacks
reverse = lambda b: (((b * 0x0802 & 0x22110) | (b * 0x8020 & 0x88440)) * 0x10101 >> 16) & 0xFF

def convert_tile(data, tile_number, tiles_per_row):
    offset = (tile_number // tiles_per_row * tiles_per_row * 8) + \
        (tile_number % tiles_per_row)
    for y in range(8):
        v = data[offset]
        v = reverse(v)
        yield v
        offset = offset + tiles_per_row

def process(filenames, mapping, tiles_per_row):
    src = ""

    for filename in filenames:
        with open(filename, "r") as f:
            src = src + f.read()

    data = [int(x[2:], base=16) for x in re.findall('0x[0-9A-Fa-f][0-9A-Fa-f]', src)]

    print(",\n".join( \
        (", ".join(
            hex(v) for v in convert_tile(data, n, tiles_per_row)
        ) for n in mapping)
    ))

print("""\
#include <stdint.h>

#pragma data-name("CHARMEM")

uint8_t graphics_data[] = {
""")

process([
    'room.xbm', 'ullr.xbm', 'urll.xbm', 'ul.xbm',
    'ur.xbm', 'll.xbm', 'lr.xbm'
], [
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
    # 46: Wall UL, UR
    54, 57, 69, 74,
    # 50: Wall LL, LR
    85, 90, 102, 105
], 4)
print(",")

# 53: wumpus
process(['wumpus.xbm'], range(4), 2)
print(",")

# 55-63: blank
for n in range(57, 63):
    print("0, 0, 0, 0, 0, 0, 0, 0,")

# 64: text
process(['text.xbm'], range(128), 192)
print(",")

# 192: title graphics
process(['title.xbm'], range(40), 10)
print(",")

# 232-237: joystick message
process(['joystick.xbm'], range(6), 6)
print(",")

# 238-248: URL
process(['url.xbm'], range(11), 11)
print(",")

# 249-254: empty
for n in range(249, 255):
    print("0, 0, 0, 0, 0, 0, 0, 0,")

# 255: solid
print("255, 255, 255, 255, 255, 255, 255, 255")

print("""\
};
""")
