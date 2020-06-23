CC=g++
CFLAGS=-I.
LDLIBS=-lm -lncursesw

wv: main.c
	$(CC) -o wv main.c -I. $(LDLIBS)
	#./wv d.raw

clean:
	rm wv
