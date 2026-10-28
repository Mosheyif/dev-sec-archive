#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: Passive TCP Pong node (Server) that listens for 
* incoming connections and supports multiple concurrent clients
* using a multi-threaded architecture (pthreads).
* Author: Moshe Yifrah
*
* Compile with: make
*
* Run by: ./tcp_pong <Listen Port>
*
*********************************************************/

/* Includes *********************************************/

#include "tcp_utils.h"

#include <stdio.h>	    /* printf, fprintf, stderr, perror */
#include <stdlib.h>	    /* exit, EXIT_FAILURE, EXIT_SUCCESS, atoi, malloc, free */
#include <string.h>	    /* strlen */
#include <unistd.h>	    /* close, STDIN_FILENO */
#include <sys/types.h>	/* ssize_t */
#include <sys/socket.h>	/* bind, listen, accept, send, recv, struct sockaddr */
#include <pthread.h>	/* pthread_create, pthread_mutex_t, pthread_detach */

#define MAX_CLIENTS 10

/*********************************************************/

static int g_client_sockets[MAX_CLIENTS];
static pthread_mutex_t g_clients_mutex;

/*********************************************************/

static void* AcceptorThread(void* arg);
static void* ClientHandlerThread(void* arg);
static void BroadcastMessage(const char* msg);
void run_tcp_server(unsigned short listen_port);

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
	
	run_tcp_server(port);

	return EXIT_SUCCESS;
}

/*********************************************************/

void run_tcp_server(unsigned short listen_port)
{
	int serv_sock;
	struct sockaddr_in serv_addr;
	pthread_t acceptor_tid;
	char buffer[BUFFER_SIZE];
	int i;

	for (i = 0; i < MAX_CLIENTS; ++i)
	{
		g_client_sockets[i] = -1;
	}

	if (pthread_mutex_init(&g_clients_mutex, NULL) != 0)
	{
		die_with_error("pthread_mutex_init() failed");
	}

	serv_sock = create_tcp_socket();
	init_sockaddr(&serv_addr, "0.0.0.0", listen_port);

	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		die_with_error("bind() failed");
	}

	if (listen(serv_sock, MAX_CLIENTS) < 0)
	{
		die_with_error("listen() failed");
	}

	printf("Waiting for TCP connections on port %u...\n", listen_port);

	if (pthread_create(&acceptor_tid, NULL, AcceptorThread, &serv_sock) != 0)
	{
		die_with_error("pthread_create() failed for Acceptor");
	}

	printf("Server running. Type messages at any time to broadcast to all clients:\n");

	/* Main Thread: Reads from STDIN and broadcasts to all active clients */
	for (;;)
	{
		get_user_input(buffer, BUFFER_SIZE);
		BroadcastMessage(buffer);
	}

	close(serv_sock);
	pthread_mutex_destroy(&g_clients_mutex);
}

/*********************************************************/

static void* AcceptorThread(void* arg)
{
	int serv_sock = *(int*)arg;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_len;
	int clnt_sock;
	int* new_sock_ptr;
	pthread_t client_tid;
	int i;
	int added;

	for (;;)
	{
		clnt_len = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_len);

		if (clnt_sock < 0)
		{
			perror("accept() failed");
			continue;
		}

		pthread_mutex_lock(&g_clients_mutex);
		added = 0;
		for (i = 0; i < MAX_CLIENTS; ++i)
		{
			if (g_client_sockets[i] == -1)
			{
				g_client_sockets[i] = clnt_sock;
				added = 1;
				break;
			}
		}
		pthread_mutex_unlock(&g_clients_mutex);

		if (added)
		{
			printf("\n[System]: New client connected (Slot %d).\n", i);
			
			new_sock_ptr = (int*)malloc(sizeof(int));
			if (new_sock_ptr != NULL)
			{
				*new_sock_ptr = clnt_sock;
				if (pthread_create(&client_tid, NULL, ClientHandlerThread, new_sock_ptr) == 0)
				{
					pthread_detach(client_tid);
				}
				else
				{
					perror("pthread_create() failed for client");
					free(new_sock_ptr);
					close(clnt_sock);
					
					pthread_mutex_lock(&g_clients_mutex);
					g_client_sockets[i] = -1;
					pthread_mutex_unlock(&g_clients_mutex);
				}
			}
		}
		else
		{
			printf("\n[System]: Server full. Rejecting new client.\n");
			close(clnt_sock);
		}
	}
	
	return NULL;
}

/*********************************************************/

static void* ClientHandlerThread(void* arg)
{
	int clnt_sock = *(int*)arg;
	char buffer[BUFFER_SIZE];
	ssize_t bytes_handled;
	int i;

	free(arg);

	for (;;)
	{
		bytes_handled = recv(clnt_sock, buffer, BUFFER_SIZE - 1, 0);

		if (bytes_handled < 0)
		{
			perror("recv() failed in client thread");
			break;
		}
		else if (bytes_handled == 0)
		{
			printf("\n[System]: Client on sock %d disconnected.\n", clnt_sock);
			break;
		}

		buffer[bytes_handled] = '\0';
		printf("\n[Client sock %d]: %s\n", clnt_sock, buffer);
	}

	pthread_mutex_lock(&g_clients_mutex);
	for (i = 0; i < MAX_CLIENTS; ++i)
	{
		if (g_client_sockets[i] == clnt_sock)
		{
			g_client_sockets[i] = -1;
			break;
		}
	}
	pthread_mutex_unlock(&g_clients_mutex);

	close(clnt_sock);
	
	return NULL;
}

/*********************************************************/

static void BroadcastMessage(const char* msg)
{
	int i;
	ssize_t sent_bytes;

	pthread_mutex_lock(&g_clients_mutex);
	for (i = 0; i < MAX_CLIENTS; ++i)
	{
		if (g_client_sockets[i] != -1)
		{
			sent_bytes = send(g_client_sockets[i], msg, strlen(msg), 0);
			if (sent_bytes < 0)
			{
				perror("send() failed during broadcast");
			}
		}
	}
	pthread_mutex_unlock(&g_clients_mutex);
}
