CC = gcc
CFLAGS = -ansi -pedantic-errors -Wall -Wextra

all: udp_ping udp_pong udp_broadcast

udp_ping: udp_ping.o udp_utils.o
	$(CC) $(CFLAGS) -o udp_ping udp_ping.o udp_utils.o

udp_pong: udp_pong.o udp_utils.o
	$(CC) $(CFLAGS) -o udp_pong udp_pong.o udp_utils.o

udp_broadcast: udp_broadcast.o udp_utils.o
	$(CC) $(CFLAGS) -o udp_broadcast udp_broadcast.o udp_utils.o

udp_ping.o: udp_ping.c udp_utils.h
	$(CC) $(CFLAGS) -c udp_ping.c

udp_pong.o: udp_pong.c udp_utils.h
	$(CC) $(CFLAGS) -c udp_pong.c

udp_broadcast.o: udp_broadcast.c udp_utils.h
	$(CC) $(CFLAGS) -c udp_broadcast.c

udp_utils.o: udp_utils.c udp_utils.h
	$(CC) $(CFLAGS) -c udp_utils.c

clean:
	rm -f *.o udp_ping udp_pong udp_broadcast
