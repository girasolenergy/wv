CC		:= g++
CFLAGS	:= -I.
LDLIBS	:= -lm -lpthread -ltermbox

SOURCES	:= $(wildcard *.cpp)
OBJS	:= $(patsubst %.cpp, %.o, $(SOURCES))
HEADERS := $(patsubst %.cpp, %.h, $(SOURCES))
TARGET	:= wv

all: $(TARGET)

$(TARGET): $(OBJS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

run: $(TARGET)
	./$(TARGET) ddd.raw

clean:
	rm $(TARGET)
	rm *.o
