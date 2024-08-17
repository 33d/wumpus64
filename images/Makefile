CONVERT_ARGS = -monochrome -depth 1

GENERATED = room.xbm ul.xbm ur.xbm ll.xbm lr.xbm ullr.xbm urll.xbm

../src/sprite_data.c: convert_sprites.py $(GENERATED)
	python3 convert_sprites.py > ../src/sprite_data.c

../src/graphics_data.c: convert_image.py $(GENERATED)
	python3 convert_image.py > ../src/graphics_data.c

urll.xbm: ullr.svg
	convert $< -rotate 90 $(CONVERT_ARGS) $@

ur.xbm: ul.svg
	convert $< -rotate 90 $(CONVERT_ARGS) $@

lr.xbm: ul.svg
	convert $< -rotate 180 $(CONVERT_ARGS) $@

ll.xbm: ul.svg
	convert $< -rotate 270 $(CONVERT_ARGS) $@

%.xbm: %.svg
	convert $< $(CONVERT_ARGS) $@

clean:
	-rm $(GENERATED)

.PHONY: clean
