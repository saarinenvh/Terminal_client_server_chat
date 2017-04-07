CC=gcc
CFLAGS=-I.

chat:
	$(MAKE) server
	$(MAKE) client

server: server.c
	$(CC) -o $@ $^

client: client.c
	$(CC) -o $@ $^
