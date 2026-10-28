#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: A UDP broadcast program that periodically 
* sends messages to all computers in the local network.
* Author: Moshe Yifrah
*
* Use 'make' to compile the program.
*
* Run by: ./udp_broadcast <Target Port>
*
*********************************************************/

/* Includes *********************************************/

#include "udp_utils.h"

#include <stdio.h>      /* printf, fprintf, stderr */
#include <stdlib.h>	    /* exit, EXIT_FAILURE, EXIT_SUCCESS, atoi */
#include <string.h>     /* strlen */
#include <unistd.h>	    /* close, sleep */
#include <sys/types.h>  /* ssize_t */
#include <sys/socket.h> /* setsockopt, sendto, struct sockaddr, SOL_SOCKET, SO_BROADCAST */

/*********************************************************/

static void SendBroadcastMessage(int sock, struct sockaddr_in* broadcast_addr);
void run_broadcast_node(unsigned short target_port);

/*********************************************************/

int main(int argc, char *argv[])
{
	unsigned short port;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <Target Port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	port = (unsigned short)atoi(argv[1]);
	
	run_broadcast_node(port);

	return EXIT_SUCCESS;
}

/*********************************************************/

void run_broadcast_node(unsigned short target_port)
{
	int sock;
	struct sockaddr_in broadcast_addr;
	int broadcast_enable;

	broadcast_enable = 1;
	sock = create_udp_socket();
	
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0)
	{
		die_with_error("setsockopt() failed");
	}

	init_sockaddr(&broadcast_addr, "255.255.255.255", target_port);

	printf("Starting UDP broadcast on port %u...\n", target_port);

	for (;;)
	{
		SendBroadcastMessage(sock, &broadcast_addr);
		sleep(2);
	}

	close(sock);
}

/*********************************************************/

static void SendBroadcastMessage(int sock, struct sockaddr_in* broadcast_addr)
{
	ssize_t sent_bytes;
	const char *msg = "ANNOUNCEMENT: Broadcast test from node";
	
	sent_bytes = sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)broadcast_addr, sizeof(*broadcast_addr));
	
	if (sent_bytes < 0)
	{
		die_with_error("sendto() failed");
	}
	
	printf("Broadcast message sent.\n");
}
