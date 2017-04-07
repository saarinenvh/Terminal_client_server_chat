CC=gcc
CFLAGS=-g -std=gnu11 -o

chat:
	$(MAKE) server
	$(MAKE) client

server: server.c
	$(CC) $(CFLAGS) $@ $^

client: client.c
	$(CC) $(CFLAGS) $@ $^

clean:
	rm -f client server client.o server.o
