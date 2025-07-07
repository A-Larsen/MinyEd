CC = gcc.exe
LIBS = ../libs/lua54.lib

all: buffer wrapLines

buffer: ../editor.c buffer.c
	$(CC) $^ -g -Wall -o buffer $(LIBS)

wrapLines: ../editor.c wrapLines.c
	$(CC) $^ -g -Wall -o wrapLines $(LIBS)

clean:
	DEL .\buffer.exe

.PHONY: clean
