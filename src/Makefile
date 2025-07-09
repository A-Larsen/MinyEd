# CC = gcc.exe
# LIBS = ./libs/lua54.lib
CC = gcc
LIBS = -llua5.4

all: filedo minyed

filedo: filedo.c filedo_options.c
	$(CC) $^ -g -Wall -o ..\bin\filedo

minyed: editor.c minyed.c
	$(CC) $^ -g -Wall -o ../bin/minyed $(LIBS)

clean:
	DEL ..\bin\filedo.exe
	DEL ..\bin\minyed.exe

.PHONY: clean
