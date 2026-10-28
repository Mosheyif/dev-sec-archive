#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: Passive UDP Pong node that listens for incoming
* connections and supports asynchronous full-duplex chat.
* Author: Moshe Yifrah
*
* Use 'make' to compile the program.
*
* Run by: ./udp_pong <Listen Port>
*
*********************************************************/

/* Includes *********************************************/

#include "udp_utils.h"

#include <stdio.h>      /* printf, fprintf, stderr */
#include <stdlib.h>     /* exit, EXIT_FAILURE, EXIT_SUCCESS, atoi */
#include <string.h>     /* strlen */
#include <unistd.h>     /* close, STDIN_FILENO */
#include <sys/types.h>  /* ssize_t */
#include <sys/socket.h> /* bind, sendto, recvfrom, struct sockaddr */
#include <sys/select.h> /* select, fd_set, FD_ZERO, FD_SET, FD_ISSET */

/*********************************************************/

static void HandleIncomingClientData(int sock, struct sockaddr_in* client_addr,
 socklen_t* client_len, int* client_known);
static void HandleKeyboardReply(int sock, struct sockaddr_in* client_addr, socklen_t client_len);
void run_pong_node(unsigned short listen_port);

/*********************************************************/

int main(int argc, char *argv[])
{
	unsigned short port;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <Listen Port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	port = (unsigned short)atoi(argv[1]);
	
	run_pong_node(port);

	return EXIT_SUCCESS;
}

/*********************************************************/

void run_pong_node(unsigned short listen_port)
{
	int sock;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_len;
	fd_set read_fds;
	int max_fd;
	int client_known;

	sock = create_udp_socket();
	init_sockaddr(&server_addr, "0.0.0.0", listen_port);

	if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		die_with_error("bind() failed");
	}

	client_len = sizeof(client_addr);
	client_known = 0;

	printf("Waiting for first message on port %u...\n", listen_port);

	for (;;)
	{
		FD_ZERO(&read_fds);
		FD_SET(sock, &read_fds);
		
		if (client_known)
		{
			FD_SET(STDIN_FILENO, &read_fds);
		}

		max_fd = (sock > STDIN_FILENO) ? sock : STDIN_FILENO;

		if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
		{
			die_with_error("select() failed");
		}

		if (FD_ISSET(sock, &read_fds))
		{
			HandleIncomingClientData(sock, &client_addr, &client_len, &client_known);
		}

		if (client_known && FD_ISSET(STDIN_FILENO, &read_fds))
		{
			HandleKeyboardReply(sock, &client_addr, client_len);
		}
	}

	close(sock);
}

/*********************************************************/

static void HandleIncomingClientData(int sock, struct sockaddr_in* client_addr,
 socklen_t* client_len, int* client_known)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytes_handled;

	*client_len = sizeof(*client_addr);

	bytes_handled = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, 
	(struct sockaddr*)client_addr, client_len);
	
	if (bytes_handled < 0)
	{
		die_with_error("recvfrom() failed");
	}
		
	buffer[bytes_handled] = '\0';

	if (!(*client_known))
	{
		*client_known = 1;
		printf("Client connected. You can now type messages at any time:\n");
	}
	
	printf("\n[Client]: %s\n", buffer);
}

/*********************************************************/

static void HandleKeyboardReply(int sock, struct sockaddr_in* client_addr, socklen_t client_len)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytes_handled;
	
	get_user_input(buffer, BUFFER_SIZE);
	
	bytes_handled = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)client_addr, client_len);
	
	if (bytes_handled < 0)
	{
		die_with_error("sendto() failed");
	}
}
