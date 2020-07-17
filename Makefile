CC		:= g++
CFLAGS	:= -I. -g -std=c++11
CXXFLAGS = -fpermissive
LDLIBS	:= -lm -lpthread

SOURCES	:= $(wildcard *.cpp) $(wildcard termbox/*.c) 
OBJS	:= $(patsubst %.cpp, %.o, $(SOURCES))
HEADERS := $(patsubst %.cpp, %.h, $(SOURCES))
TARGET	:= wv

all: $(TARGET)

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) $(CFLAGS) $(CXXFLAGS) -o $@ $(OBJS) $(LDLIBS)

run: $(TARGET)
	#./$(TARGET) data.raw

clean:
	rm *.o
	rm $(TARGET)
