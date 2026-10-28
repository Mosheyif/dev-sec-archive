CC = gcc
CFLAGS_STRICT = -ansi -pedantic-errors -Wall -Wextra -I./include
CFLAGS_BASIC = -I./include
LDFLAGS = -lcrypto

SRC_SHARED = src/network_utils.c src/tun_interface.c src/crypto_engine.c src/os_routing.c src/vpn_core.c
OBJ_SHARED = $(SRC_SHARED:.c=.o)
OBJ_SERVER = src/vpn_server.o src/vpn_registry.o
OBJ_CLIENT = src/vpn_client.o

# Environment Variables for Execution
PORT ?= 8080
KEY ?= keys/shared.key
DEV ?= tun0
SERVER_IP ?= 127.0.0.1
VIP_SERVER ?= 10.8.0.1
VIP_CLIENT ?= 10.8.0.2
SUBNET ?= 10.8.0.0/24

.PHONY: all basic clean help run_server run_client print_basic usage

all: bin/vpn_server bin/vpn_client

basic: CFLAGS_STRICT = $(CFLAGS_BASIC)
basic: all

bin/vpn_server: $(OBJ_SERVER) $(OBJ_SHARED)
	mkdir -p bin
	$(CC) $(CFLAGS_STRICT) $^ -o $@ $(LDFLAGS)

bin/vpn_client: $(OBJ_CLIENT) $(OBJ_SHARED)
	mkdir -p bin
	$(CC) $(CFLAGS_STRICT) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS_STRICT) -c $< -o $@

print_basic:
	@echo "Server:"
	@echo "$(CC) $(CFLAGS_BASIC) src/vpn_server.c src/vpn_registry.c $(SRC_SHARED) -o bin/vpn_server $(LDFLAGS)"
	@echo "Client:"
	@echo "$(CC) $(CFLAGS_BASIC) src/vpn_client.c $(SRC_SHARED) -o bin/vpn_client $(LDFLAGS)"

run_server: bin/vpn_server
	sudo ./bin/vpn_server $(PORT) $(KEY) $(DEV) $(VIP_SERVER) $(SUBNET)

run_client: bin/vpn_client
	sudo ./bin/vpn_client $(SERVER_IP) $(PORT) $(KEY) $(DEV) $(VIP_CLIENT) $(SUBNET)

clean:
	rm -rf bin/ src/*.o

help:
	@echo "=========================================================="
	@echo " VPN Project Build System"
	@echo "=========================================================="
	@echo " make              - Compile strictly (ANSI C89, -Wall -Wextra)"
	@echo " make basic        - Compile without strict formatting flags"
	@echo " make print_basic  - Print raw GCC compilation strings"
	@echo " make usage        - Print execution parameters and examples"
	@echo " make clean        - Remove binaries and object files"
	@echo "=========================================================="

usage:
	@echo "=========================================================="
	@echo " VPN Execution Parameters Guide"
	@echo "=========================================================="
	@echo "Server Usage:"
	@echo "  Syntax:  sudo ./bin/vpn_server <Port> <Key> <TUN> <Server VIP> <Subnet>"
	@echo "  Example: sudo ./bin/vpn_server 8080 keys/shared.key tun0 10.8.0.1 10.8.0.0/24"
	@echo ""
	@echo "Client Usage:"
	@echo "  Syntax:  sudo ./bin/vpn_client <Server IP> <Port> <Key> <TUN> <Client VIP> <Dest Subnet>"
	@echo "  Example: sudo ./bin/vpn_client 127.0.0.1 8080 keys/shared.key tun1 10.8.0.2 10.8.0.0/24"
	@echo "=========================================================="
