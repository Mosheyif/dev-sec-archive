#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: Active TCP Ping node (Client) that initiates 
* a connection and supports asynchronous full-duplex chat.
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./tcp_ping <Target IP> <Target Port>
*
*********************************************************/

/* Includes *********************************************/

#include "tcp_utils.h"

#include <stdio.h>	    /* printf, fprintf, stderr */
#include <stdlib.h>	    /* exit, EXIT_FAILURE, EXIT_SUCCESS, atoi */
#include <string.h>	    /* strlen */
#include <unistd.h>	    /* close, STDIN_FILENO */
#include <sys/types.h>	/* ssize_t */
#include <sys/socket.h>	/* connect, send, recv, struct sockaddr */
#include <sys/select.h>	/* select, fd_set, FD_ZERO, FD_SET, FD_ISSET */

/*********************************************************/

static int HandleIncomingData(int sock);
static void HandleKeyboardInput(int sock);
void run_tcp_client(const char* target_ip, unsigned short target_port);

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
	
	run_tcp_client(argv[1], port);

	return EXIT_SUCCESS;
}

/*********************************************************/

void run_tcp_client(const char* target_ip, unsigned short target_port)
{
	int sock;
	struct sockaddr_in serv_addr;
	fd_set read_fds;
	int max_fd;

	sock = create_tcp_socket();
	init_sockaddr(&serv_addr, target_ip, target_port);

	printf("Connecting to %s:%u...\n", target_ip, target_port);

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		die_with_error("connect() failed");
	}

	printf("Connection established. Type messages to send at any time:\n");

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
			if (0 == HandleIncomingData(sock))
			{
				break;
			}
		}

		if (FD_ISSET(STDIN_FILENO, &read_fds))
		{
			HandleKeyboardInput(sock);
		}
	}

	close(sock);
}

/*********************************************************/

static int HandleIncomingData(int sock)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytes_handled;
	
	bytes_handled = recv(sock, buffer, BUFFER_SIZE - 1, 0);
	
	if (bytes_handled < 0)
	{
		die_with_error("recv() failed");
	}
	else if (bytes_handled == 0)
	{
		printf("\nServer closed the connection.\n");
		return 0;
	}
	
	buffer[bytes_handled] = '\0';
	printf("\n[Server]: %s\n", buffer);
	
	return 1;
}

/*********************************************************/

static void HandleKeyboardInput(int sock)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytes_handled;
	
	get_user_input(buffer, BUFFER_SIZE);
	
	bytes_handled = send(sock, buffer, strlen(buffer), 0);
	
	if (bytes_handled < 0)
	{
		die_with_error("send() failed");
	}
}
