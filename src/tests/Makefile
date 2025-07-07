CC = gcc.exe
LIBS = ../libs/lua54.lib

all: buffer wrapLines

buffer: ../editor.c buffer.c
	$(CC) $^ -g -Wall -o .\bin\buffer $(LIBS)

wrapLines: ../editor.c wrapLines.c
	$(CC) $^ -g -Wall -o .\bin\wrapLines $(LIBS)

clean:
	DEL .\buffer.exe

.PHONY: clean
