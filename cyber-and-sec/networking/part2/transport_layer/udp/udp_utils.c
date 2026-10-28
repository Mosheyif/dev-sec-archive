#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: Shared networking utility functions for UDP sockets.
* Author: Moshe Yifrah
*
* Use 'make' to compile the program.
*
*********************************************************/

/* Includes *********************************************/

#include "udp_utils.h"

#include <stdio.h>      /* perror, fgets, stdin */
#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include <string.h>     /* memset, strlen */
#include <unistd.h>     /* read, STDIN_FILENO */
#include <sys/types.h>  /* ssize_t */
#include <sys/socket.h> /* socket, AF_INET, SOCK_DGRAM */
#include <arpa/inet.h>  /* inet_addr */

/*********************************************************/

void die_with_error(const char* error_message)
{
	perror(error_message);
	exit(EXIT_FAILURE);
}

/*********************************************************/

int create_udp_socket(void)
{
	int sock;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (sock < 0)
	{
		die_with_error("socket() failed");
	}
	
	return sock;
}

/*********************************************************/

void init_sockaddr(struct sockaddr_in* addr, const char* ip, unsigned short port)
{
	memset(addr, 0, sizeof(*addr));
	
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr(ip);
	addr->sin_port = htons(port);
}

/*********************************************************/

void get_user_input(char* buffer, int size)
{
	ssize_t bytes_read;
	
	bytes_read = read(STDIN_FILENO, buffer, size - 1);
	
	if (bytes_read < 0)
	{
		die_with_error("read() failed");
	}
	
	buffer[bytes_read] = '\0';
	
	if (bytes_read > 0 && buffer[bytes_read - 1] == '\n')
	{
		buffer[bytes_read - 1] = '\0';
	}
}
