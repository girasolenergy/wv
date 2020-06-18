CC=gcc
CFLAGS=-I.
LDLIBS=-lm -lncursesw

wv: main.c
	$(CC) -o wv main.c -I. $(LDLIBS)
	./wv

clean:
	rm wv
