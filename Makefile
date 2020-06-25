CC=g++
CFLAGS=-I.
LDLIBS=-lm -lncursesw

wv: main.cpp block.h track.h
	$(CC) -o wv main.cpp -I. $(LDLIBS)
	#./wv d.raw

clean:
	rm wv
