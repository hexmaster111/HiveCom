
linux: font_16_32.o font_8_16.o
	cc -c src/main.c -ohivecom_linux.o
	cc hivecom_linux.o font_8_16.o font_16_32.o -lraylib -lm -ohivecom_linux

font_8_16.o:
	cc -c fonts/font_8_16.c -ofont_8_16.o

font_16_32.o:
	cc -c fonts/font_16_32.c -ofont_16_32.o


clean:
	rm -f hivecom_linux
	rm -f hivecom_linux.o
	rm -f font_8_16.o
	rm -f font_16_32.o