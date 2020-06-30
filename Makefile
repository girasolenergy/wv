CC=g++
CFLAGS=-I.
LDLIBS=-lm -lncursesw -lpthread -ltermbox

all: wv run

wv: main.cpp block.h track.h
	$(CC) -o wv main.cpp -I. $(LDLIBS)

run: wv
	./wv ddd.raw

clean:
	rm wv
