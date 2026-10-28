CC = gcc
CFLAGS = -ansi -pedantic-errors -Wall -Wextra -pthread -I../../transport_layer/tcp
LDFLAGS = ../../transport_layer/tcp/libtcp.a

TARGET = http_server
SRC = http_server.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f $(TARGET)
