#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: A multi-threaded HTTP/HTTPS Server featuring
*              path-based routing and mTLS authentication.
* Author: Moshe Yifrah
*
* Use 'make help' command to see compiling options.
*
*********************************************************/

/* Includes *********************************************/

#include <stdio.h>		/* printf, perror, sprintf, sscanf, FILE... */
#include <stdlib.h>		/* exit, EXIT_FAILURE, EXIT_SUCCESS, malloc */
#include <string.h>		/* strlen, strcmp, strncmp */
#include <unistd.h>		/* close */
#include <sys/types.h>	/* ssize_t */
#include <sys/socket.h>	/* bind, listen, accept, send, recv... */
#include <pthread.h>	/* pthread_create, pthread_detach... */

#include "tcp_utils.h"
#include "ssl_handler.h"/* CryptoContext and secure I/O wrappers */

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

/* Structures *********************************************/

typedef struct 
{
	int serv_sock;
	CryptoContext crypto_ctx;
} ServerContext;

typedef struct 
{
	int clnt_sock;
	CryptoContext crypto_ctx;
} ClientContext;

/*********************************************************/	

static void* AcceptorThread(void* arg);
static void* ClientHandlerThread(void* arg);
static int ParseHttpRequest(const char* buffer, char* method, char* path);
static void HandleHttpRequest(ClientContext* clnt_ctx, const char* method, 
			      const char* path);
static void SendHttpResponse(ClientContext* clnt_ctx, const char* status, 
			     const char* content_type, const char* body);
static void ServeFile(ClientContext* clnt_ctx, const char* filepath);

#ifdef USE_SSL
	void RunHttpServer(unsigned short listen_port, const char* cert_path, 
			   const char* key_path);
#else
	void RunHttpServer(unsigned short listen_port);
#endif

/*********************************************************/

int main(int argc, char *argv[])
{
	#ifdef USE_SSL
	
		if (argc != 3)
		{
			fprintf(stderr, "Usage: %s <cert_path> <key_path>\n", 
				argv[0]);
			return EXIT_FAILURE;
		}
		
		RunHttpServer(HTTP_PORT, argv[1], argv[2]);
	
	#else
	
		(void)argc;
		(void)argv;
		RunHttpServer(HTTP_PORT);
		
	#endif

	return EXIT_SUCCESS;
}

/*********************************************************/

#ifdef USE_SSL

void RunHttpServer(unsigned short listen_port, const char* cert_path, 
		   const char* key_path)

#else

void RunHttpServer(unsigned short listen_port)

#endif

{
	ServerContext serv_ctx = {0};
	struct sockaddr_in serv_addr;
	pthread_t acceptor_tid;

	serv_ctx.serv_sock = create_tcp_socket();
	init_sockaddr(&serv_addr, "0.0.0.0", listen_port);

	if (bind(serv_ctx.serv_sock, (struct sockaddr *)&serv_addr, 
		 sizeof(serv_addr)) < 0) 
	{
		die_with_error("bind() failed");
	}
	
	if (listen(serv_ctx.serv_sock, LISTEN_BACKLOG) < 0) 
	{
		die_with_error("listen() failed");
	}

	#ifdef USE_SSL
	
		serv_ctx.crypto_ctx = InitCryptoContext(cert_path, key_path);
		
		if (serv_ctx.crypto_ctx) 
		{
			printf("HTTPS Server running on port %u...\n", 
				listen_port);
		} 
		
		else 
		{
			printf("HTTP Server running on port %u...\n", 
				listen_port);
		}
		
	#else
	
		printf("HTTP Server running on port %u...\n", listen_port);
		
	#endif

	if (pthread_create(&acceptor_tid, NULL, AcceptorThread, &serv_ctx) != 0) 
	{
		die_with_error("pthread_create() failed");
	}

	pthread_join(acceptor_tid, NULL);
	close(serv_ctx.serv_sock);

	#ifdef USE_SSL
	
		CleanupCryptoContext(serv_ctx.crypto_ctx);
	
	#endif
}

/*********************************************************/

static void* AcceptorThread(void* arg)
{
	ServerContext *serv_ctx = (ServerContext*)arg;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_len;
	int clnt_sock;
	ClientContext* clnt_ctx;
	pthread_t client_tid;

	for (;;)
	{
		clnt_len = sizeof(clnt_addr);
		clnt_sock = accept(serv_ctx->serv_sock, 
				   (struct sockaddr *)&clnt_addr, &clnt_len);

		if (clnt_sock < 0)
		{
			perror("accept() failed");
			continue;
		}

		clnt_ctx = (ClientContext*)malloc(sizeof(ClientContext));
		
		if (clnt_ctx != NULL)
		{
			clnt_ctx->clnt_sock = clnt_sock;
			clnt_ctx->crypto_ctx = AcceptCryptoConnection(
						serv_ctx->crypto_ctx, 
						clnt_sock);
			
			if (serv_ctx->crypto_ctx && !clnt_ctx->crypto_ctx)
			{
				close(clnt_sock);
				free(clnt_ctx);
				continue;
			}
			
			if (pthread_create(&client_tid, NULL, 
					   ClientHandlerThread, clnt_ctx) == 0)
			{
				pthread_detach(client_tid);
			}
			else
			{
				perror("pthread_create() failed for client");
				FreeCryptoConnection(clnt_ctx->crypto_ctx);
				close(clnt_sock);
				free(clnt_ctx);
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
	ClientContext *clnt_ctx = (ClientContext*)arg;
	char buffer[REQ_BUF_SIZE];
	ssize_t bytes_handled;
	char method[METHOD_ARR_SIZE];
	char path[PATH_ARR_SIZE];

	bytes_handled = SecureRecv(clnt_ctx->crypto_ctx, clnt_ctx->clnt_sock, 
				   buffer, REQ_BUF_SIZE - 1);

	if (bytes_handled > 0)
	{
		buffer[bytes_handled] = '\0';
		
		if (ParseHttpRequest(buffer, method, path))
		{
			printf("[HTTP Request]: %s %s\n", method, path);
			HandleHttpRequest(clnt_ctx, method, path);
		}
	}

	FreeCryptoConnection(clnt_ctx->crypto_ctx);
	close(clnt_ctx->clnt_sock);
	free(clnt_ctx);
	
	return NULL;
}

/*********************************************************/

static int ParseHttpRequest(const char* buffer, char* method, char* path)
{
	const char* format_str = "%" STR(MAX_METHOD_CHARS) "s %" 
				 STR(MAX_PATH_CHARS) "s";
	return (sscanf(buffer, format_str, method, path) == 2);
}

/*********************************************************/

static void HandleHttpRequest(ClientContext* clnt_ctx, const char* method, 
			      const char* path)
{
	if (strcmp(method, "GET") == 0)
	{
		if (strncmp(path, "/secure-a", 9) == 0)
		{
			if (!VerifyClientCert(clnt_ctx->crypto_ctx, NULL))
			{
				SendHttpResponse(clnt_ctx, "403 Forbidden", 
						 "text/plain", 
						 "403: Cert Required\n");
				return;
			}
			SendHttpResponse(clnt_ctx, "200 OK", "text/plain", 
					 "Access Granted to Secure Page A\n");
		}
		else if (strncmp(path, "/secure-b", 9) == 0)
		{
			if (!VerifyClientCert(clnt_ctx->crypto_ctx, 
					      "admin-infinity"))
			{
				SendHttpResponse(clnt_ctx, "403 Forbidden", 
						 "text/plain", 
						 "403: Admin Cert Required\n");
				return;
			}
			SendHttpResponse(clnt_ctx, "200 OK", "text/plain", 
					 "Access Granted to Secure Page B\n");
		}
		else if (strcmp(path, "/") == 0)
		{
			SendHttpResponse(clnt_ctx, "200 OK", "text/plain", 
					 "Hello World\n");
		}
		else
		{
			ServeFile(clnt_ctx, path + 1);
		}
	}
	else
	{
		SendHttpResponse(clnt_ctx, "405 Method Not Allowed", 
				 "text/plain", "Only GET is supported.\n");
	}
}

/*********************************************************/

static void SendHttpResponse(ClientContext* clnt_ctx, const char* status, 
			     const char* content_type, const char* body)
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
		
	SecureSend(clnt_ctx->crypto_ctx, clnt_ctx->clnt_sock, response, 
		   strlen(response));
}

/*********************************************************/

static void ServeFile(ClientContext* clnt_ctx, const char* filepath)
{
	FILE* file;
	char header[HEADER_BUF_SIZE];
	char file_buffer[FILE_CHUNK_SIZE];
	size_t bytes_read;
	long file_size;
	
	file = fopen(filepath, "rb");
	
	if (NULL == file)
	{
		SendHttpResponse(clnt_ctx, "404 Not Found", "text/plain", 
				 "404 File Not Found");
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
		
	SecureSend(clnt_ctx->crypto_ctx, clnt_ctx->clnt_sock, header, 
		   strlen(header));
	
	while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), 
				   file)) > 0)
	{
		SecureSend(clnt_ctx->crypto_ctx, clnt_ctx->clnt_sock, 
			   file_buffer, bytes_read);
	}
	
	fclose(file);
}
