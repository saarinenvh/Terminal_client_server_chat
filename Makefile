CC=gcc
CFLAGS=-I.

chat:
	$(MAKE) server
	$(MAKE) client

server: server.c
	$(CC) -g -o $@ $^

client: client.c
	$(CC) -g -o $@ $^

clean:
	rm -f client server client.o server.o
