CC = gcc
CFLAGS = -Wall -g

sh: sh.o
	$(CC) $(CFLAGS) -o ./out/sh ./out/sh.o

sh.o: sh.c
	$(CC) $(CFLAGS) -c sh.c -o ./out/sh.o
