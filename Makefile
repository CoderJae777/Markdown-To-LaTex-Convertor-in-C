CC = gcc
CFLAGS = -Wall
SRCS = main.c lexer.c parser_block.c parser_inline.c parser_latex.c
TARGET = output/main.exe

all: $(TARGET)

$(TARGET): $(SRCS) lexer.h parser.h parser_block.h parser_inline.h parser_latex.h
	mkdir -p output
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
