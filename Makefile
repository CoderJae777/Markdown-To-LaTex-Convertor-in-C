CC = gcc
CFLAGS = -Wall
SRCS = main.c lexer.c
TARGET = output/main.exe

all: $(TARGET)

$(TARGET): $(SRCS) lexer.h
	mkdir -p output
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
