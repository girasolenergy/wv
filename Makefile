CC		:= g++
CFLAGS	:= -I.
LDLIBS	:= -lm -lpthread -ltermbox

SOURCES	:= $(wildcard *.cpp)
OBJS	:= $(patsubst %.cpp, %.o, $(SOURCES))
TARGET	:= wv

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

run: $(TARGET)
	./$(TARGET) ddd.raw

clean:
	rm $(TARGET)
	rm *.o
