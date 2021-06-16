CC		:= g++
CFLAGS	:= -I. -g
CXXFLAGS:= -fpermissive -std=c++17
LDLIBS	:= -lm -lpthread -lstdc++fs

SOURCES	:= $(wildcard *.cpp) $(wildcard termbox/*.c) 
OBJS	:= $(patsubst %.cpp, %.o, $(SOURCES))
HEADERS := $(patsubst %.cpp, %.h, $(SOURCES))
TARGET	:= wv

all: $(TARGET)

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) $(CFLAGS) $(CXXFLAGS) -o $@ $(OBJS) $(LDLIBS)

run: $(TARGET)
	#./$(TARGET) data.raw

install:
	cp $(TARGET) /usr/local/sbin/

clean:
	rm *.o
	rm $(TARGET)
