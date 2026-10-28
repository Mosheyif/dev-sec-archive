#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: Active UDP Ping node that initiates communication 
* and supports asynchronous full-duplex chat.
* Author: Moshe Yifrah
*
* Use 'make' to compile the program.
*
* Run by: ./udp_ping <Target IP> <Target Port>
*
*********************************************************/

/* Includes *********************************************/

#include "udp_utils.h"

#include <stdio.h>      /* printf, fprintf, stderr */
#include <stdlib.h>     /* exit, EXIT_FAILURE, EXIT_SUCCESS, atoi */
#include <string.h>     /* strlen */
#include <unistd.h>     /* close, STDIN_FILENO */
#include <sys/types.h>  /* ssize_t */
#include <sys/socket.h> /* sendto, recvfrom, struct sockaddr */
#include <sys/select.h> /* select, fd_set, FD_ZERO, FD_SET, FD_ISSET */

/*********************************************************/

static void HandleIncomingData(int sock, const char* target_ip);
static void HandleKeyboardInput(int sock, struct sockaddr_in* target_addr);
void run_ping_node(const char* target_ip, unsigned short target_port);

/*********************************************************/

int main(int argc, char *argv[])
{
	unsigned short port;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <Target IP> <Target Port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	port = (unsigned short)atoi(argv[2]);
	
	run_ping_node(argv[1], port);

	return EXIT_SUCCESS;
}

/*********************************************************/

void run_ping_node(const char* target_ip, unsigned short target_port)
{
	int sock;
	struct sockaddr_in target_addr;
	fd_set read_fds;
	int max_fd;

	sock = create_udp_socket();
	init_sockaddr(&target_addr, target_ip, target_port);

	printf("Ready. Type messages to send at any time:\n");

	for (;;)
	{
		FD_ZERO(&read_fds);
		FD_SET(STDIN_FILENO, &read_fds);
		FD_SET(sock, &read_fds);

		max_fd = (sock > STDIN_FILENO) ? sock : STDIN_FILENO;

		if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
        	{
			die_with_error("select() failed");
		}

		if (FD_ISSET(sock, &read_fds))
		{
			HandleIncomingData(sock, target_ip);
		}

		if (FD_ISSET(STDIN_FILENO, &read_fds))
		{
			HandleKeyboardInput(sock, &target_addr);
		}
	}

	close(sock);
}

/*********************************************************/

static void HandleIncomingData(int sock, const char* target_ip)
{
	struct sockaddr_in from_addr;
	socklen_t from_len;
	char buffer[BUFFER_SIZE];
	ssize_t bytes_handled;
	
	from_len = sizeof(from_addr);
	bytes_handled = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&from_addr, &from_len);
	
	if (bytes_handled < 0)
	{
		die_with_error("recvfrom() failed");
	}
	
	buffer[bytes_handled] = '\0';
	printf("\n[IP: %s]: %s\n", target_ip, buffer);
}

/*********************************************************/

static void HandleKeyboardInput(int sock, struct sockaddr_in* target_addr)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytes_handled;
	
	get_user_input(buffer, BUFFER_SIZE);
	
	bytes_handled = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)target_addr, sizeof(*target_addr));
	
	if (bytes_handled < 0)
	{
		die_with_error("sendto() failed");
	}
}
