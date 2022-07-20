CC = gcc
CFLAGS = -Wall -g

grep: grep.o
	$(CC) $(CFLAGS) -o ./out/grep ./out/grep.o

grep.o: grep.c
	$(CC) $(CFLAGS) -c grep.c -o ./out/grep.o
