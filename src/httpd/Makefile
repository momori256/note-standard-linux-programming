CC = gcc
CFLAGS = -Wall -g

httpd: httpd.o socket.o service.o http.o
	$(CC) $(CFLAGS) -o ./out/httpd ./out/httpd.o ./out/socket.o ./out/service.o ./out/http.o

httpd.o: httpd.c
	$(CC) $(CFLAGS) -c httpd.c -o ./out/httpd.o

socket.o: socket.c
	$(CC) $(CFLAGS) -c socket.c -o ./out/socket.o

service.o: service.c
	$(CC) $(CFLAGS) -c service.c -o ./out/service.o

http.o: http.c
	$(CC) $(CFLAGS) -c http.c -o ./out/http.o
