CC = gcc
CFLAGS = -ansi -pedantic-errors -Wall -Wextra -pthread -I./include -I../../part2/transport_layer/tcp
TCP_LIB = ../../part2/transport_layer/tcp/libtcp.a

.PHONY: all http https clean help

# Default target: builds both servers
all: http https

# Target 1: Standard HTTP Server (No SSL overhead, strictly C89 compliant)
http: src/https_server.c
	mkdir -p bin
	$(CC) $(CFLAGS) src/https_server.c $(TCP_LIB) -o bin/http_server

# Target 2: Secure HTTPS Server (Requires OpenSSL linking)
https: src/https_server.c src/ssl_handler.c
	mkdir -p bin
	$(CC) $(CFLAGS) -DUSE_SSL src/https_server.c src/ssl_handler.c $(TCP_LIB) -lssl -lcrypto -o bin/https_server

# Cleanup rules
clean:
	rm -rf bin

# Interactive Help Menu
help:
	@echo "=========================================================="
	@echo " Hybrid HTTP/HTTPS Server Build System"
	@echo "=========================================================="
	@echo "Available commands:"
	@echo "  make        - Builds both http_server and https_server"
	@echo "  make http   - Builds ONLY the plaintext HTTP server"
	@echo "  make https  - Builds ONLY the secure HTTPS server"
	@echo "  make clean  - Removes the 'bin' directory and executables"
	@echo "  make help   - Displays this instruction menu"
	@echo "=========================================================="
