#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: A multi-threaded HTTP Server that accepts GET
* requests, returns "Hello World" for the root path, and
* serves physical files for other paths.
* Author: Moshe Yifrah
*
* Use 'make' to compile or compile manually with:
*
* gcc -pthread -I../../transport_layer/tcp http_server.c 
* ../../transport_layer/tcp/libtcp.a -o http_server
*
* Run by: ./http_server
*
*********************************************************/

/* Includes *********************************************/

#include "tcp_utils.h"
	
#include <stdio.h>	    /* printf, perror, sprintf, sscanf, FILE, fopen, fread, fclose, fseek, ftell, SEEK_END, SEEK_SET */
#include <stdlib.h>	    /* exit, EXIT_FAILURE, EXIT_SUCCESS, malloc, free */
#include <string.h>	    /* strlen, strcmp */
#include <unistd.h>	    /* close */
#include <sys/types.h>	/* ssize_t */
#include <sys/socket.h>	/* bind, listen, accept, send, recv, struct sockaddr */
#include <pthread.h>	/* pthread_create, pthread_detach, pthread_join */

/* Constants & Macros ***********************************/

#define HTTP_PORT			8000
#define REQ_BUF_SIZE		2048
#define HEADER_BUF_SIZE		512
#define FILE_CHUNK_SIZE		1024
#define LISTEN_BACKLOG		50

#define MAX_METHOD_CHARS	15
#define MAX_PATH_CHARS		255

#define METHOD_ARR_SIZE		(MAX_METHOD_CHARS + 1)
#define PATH_ARR_SIZE		(MAX_PATH_CHARS + 1)

/* Macro Stringification for secure sscanf formatting */

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/*********************************************************/

static void* AcceptorThread(void* arg);
static void* ClientHandlerThread(void* arg);
static int ParseHttpRequest(const char* buffer, char* method, char* path);
static void HandleHttpRequest(int clnt_sock, const char* method, const char* path);
static void SendHttpResponse(int clnt_sock, const char* status, const char* content_type, const char* body);
static void ServeFile(int clnt_sock, const char* filepath);
void run_http_server(unsigned short listen_port);

/*********************************************************/

int main(void)
{
	run_http_server(HTTP_PORT);
	return EXIT_SUCCESS;
}

/*********************************************************/

void run_http_server(unsigned short listen_port)
{
	int serv_sock;
	struct sockaddr_in serv_addr;
	pthread_t acceptor_tid;

	serv_sock = create_tcp_socket();
	init_sockaddr(&serv_addr, "0.0.0.0", listen_port);

	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		die_with_error("bind() failed");
	}

	if (listen(serv_sock, LISTEN_BACKLOG) < 0)
	{
		die_with_error("listen() failed");
	}

	printf("HTTP Server running on port %u...\n", listen_port);

	if (pthread_create(&acceptor_tid, NULL, AcceptorThread, &serv_sock) != 0)
	{
		die_with_error("pthread_create() failed for Acceptor");
	}

	pthread_join(acceptor_tid, NULL);
	close(serv_sock);
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

	for (;;)
	{
		clnt_len = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_len);

		if (clnt_sock < 0)
		{
			perror("accept() failed");
			continue;
		}

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
			}
		}
		else
		{
			close(clnt_sock);
		}
	}
	
	return NULL;
}

/*********************************************************/

static void* ClientHandlerThread(void* arg)
{
	int clnt_sock = *(int*)arg;
	char buffer[REQ_BUF_SIZE];
	ssize_t bytes_handled;
	char method[METHOD_ARR_SIZE];
	char path[PATH_ARR_SIZE];

	free(arg);

	bytes_handled = recv(clnt_sock, buffer, REQ_BUF_SIZE - 1, 0);

	if (bytes_handled > 0)
	{
		buffer[bytes_handled] = '\0';
		
		if (ParseHttpRequest(buffer, method, path))
		{
			printf("[HTTP Request]: %s %s\n", method, path);
			HandleHttpRequest(clnt_sock, method, path);
		}
	}

	close(clnt_sock);
	
	return NULL;
}

/*********************************************************/

static int ParseHttpRequest(const char* buffer, char* method, char* path)
{
	/* Constructs format string dynamically, e.g., "%15s %255s" */
	const char* format_str = "%" STR(MAX_METHOD_CHARS) "s %" STR(MAX_PATH_CHARS) "s";
	return (sscanf(buffer, format_str, method, path) == 2);
}

/*********************************************************/

static void HandleHttpRequest(int clnt_sock, const char* method, const char* path)
{
	if (strcmp(method, "GET") == 0)
	{
		if (strcmp(path, "/") == 0)
		{
			SendHttpResponse(clnt_sock, "200 OK", "text/plain", "Hello World");
		}
		else
		{
			ServeFile(clnt_sock, path + 1);
		}
	}
	else
	{
		SendHttpResponse(clnt_sock, "405 Method Not Allowed", "text/plain", "Only GET is supported.");
	}
}

/*********************************************************/

static void SendHttpResponse(int clnt_sock, const char* status, const char* content_type, const char* body)
{
	char response[REQ_BUF_SIZE];
	int body_len;
	
	body_len = (body != NULL) ? strlen(body) : 0;
	
	sprintf(response, 
		"HTTP/1.1 %s\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %d\r\n"
		"Connection: close\r\n"
		"\r\n"
		"%s", 
		status, content_type, body_len, (body != NULL) ? body : "");
		
	send(clnt_sock, response, strlen(response), 0);
}

/*********************************************************/

static void ServeFile(int clnt_sock, const char* filepath)
{
	FILE* file;
	char header[HEADER_BUF_SIZE];
	char file_buffer[FILE_CHUNK_SIZE];
	size_t bytes_read;
	long file_size;
	
	file = fopen(filepath, "rb");
	
	if (NULL == file)
	{
		SendHttpResponse(clnt_sock, "404 Not Found", "text/plain", "404 File Not Found");
		return;
	}
	
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	sprintf(header, 
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: %ld\r\n"
		"Connection: close\r\n"
		"\r\n", file_size);
		
	send(clnt_sock, header, strlen(header), 0);
	
	while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0)
	{
		send(clnt_sock, file_buffer, bytes_read, 0);
	}
	
	fclose(file);
}

