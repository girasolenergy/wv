CC=g++
CFLAGS=-I.
LDLIBS=-lm -lncursesw

wv: main.c block.h track.h
	$(CC) -o wv main.c -I. $(LDLIBS)
	#./wv d.raw

clean:
	rm wv
