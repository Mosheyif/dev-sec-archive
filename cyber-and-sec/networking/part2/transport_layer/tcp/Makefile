CC = gcc
CFLAGS = -ansi -pedantic-errors -Wall -Wextra -pthread
AR = ar
ARFLAGS = rcs

LIB = libtcp.a

.PHONY: all clean help

all: $(LIB) tcp_ping tcp_pong

$(LIB): tcp_utils.o
	$(AR) $(ARFLAGS) $@ $^

tcp_ping: tcp_ping.o $(LIB)
	$(CC) $(CFLAGS) -o tcp_ping tcp_ping.o $(LIB)

tcp_pong: tcp_pong.o $(LIB)
	$(CC) $(CFLAGS) -o tcp_pong tcp_pong.o $(LIB)

tcp_ping.o: tcp_ping.c tcp_utils.h
	$(CC) $(CFLAGS) -c tcp_ping.c

tcp_pong.o: tcp_pong.c tcp_utils.h
	$(CC) $(CFLAGS) -c tcp_pong.c

tcp_utils.o: tcp_utils.c tcp_utils.h
	$(CC) $(CFLAGS) -c tcp_utils.c

clean:
	rm -f *.o tcp_ping tcp_pong

help:
	@echo "=========================================================="
	@echo " TCP Utils & Ping/Pong Tools Build System"
	@echo "=========================================================="
	@echo "Available commands:"
	@echo "  make        - Builds libtcp.a, tcp_ping, and tcp_pong"
	@echo "  make clean  - Removes all objects and executables"
	@echo "  make help   - Displays this instruction menu"
	@echo "=========================================================="
