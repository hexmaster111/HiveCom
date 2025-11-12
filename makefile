


linux:
	cc fonts/fontgen.c -ggdb -lraylib -lm -ofontgen
	./fontgen
	cc src/main.c -ggdb -lraylib -lm -ohivecom_linux 

clean:
	rm -f hivecom_linux
	rm -f fontgen
	rm -f src/font_8_16.h
	rm -f src/font_8_16.png
	rm -f src/font_16_32.h
	rm -f src/font_16_32.png