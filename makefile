CC = g++
CFLAGS = -Iinclude -std=c++26 -Wall

TARGET = server
OBJ = request.o logger.o

.PHONY: all clean

all: $(TARGET)

%.o: %.cpp
	${CC} ${CFLAGS} -c $^

$(TARGET): $(OBJ)
	${CC} $(CFLAGS) $^ $(TARGET).cpp  -o $@

clean:
	rm -f $(TARGET) *.o
