CC     = gcc
CFLAGS = -Wall -g
SRC    = src/ls-v1.0.0.c
TARGET = bin/ls

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
