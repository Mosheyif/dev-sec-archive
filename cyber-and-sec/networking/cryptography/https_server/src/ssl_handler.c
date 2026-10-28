#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: SSL/TLS Context handler for HTTPS Server featuring 
*              mTLS (Mutual TLS) and path-based client authentication.
* Author: Moshe Yifrah
*
* Use 'make help' command to see compiling options or compile manually with:
*
* Compile with: gcc -pthread -I./include -I../../part2/transport_layer/tcp src/
*               https_server.c src/ssl_handler.c ../../part2/transport_layer/
*               tcp/libtcp.a -DUSE_SSL -lssl -lcrypto -o bin/https_server
*
* Run by: ./bin/https_server <cert_path> <key_path>
*
*********************************************************/

#ifdef USE_SSL

/* Includes *********************************************/

#include <openssl/ssl.h>	/* SSL_CTX, SSL_CTX_new, SSL_new, SSL_read,
				 * SSL_write, SSL_accept, SSL_free, 
				 * SSL_CTX_free, TLS_server_method, 
				 * SSL_library_init, OpenSSL_add_all_algorithms,
				 * SSL_load_error_strings, SSL_set_fd,
				 * SSL_CTX_use_certificate_file,
				 * SSL_CTX_use_PrivateKey_file, 
				 * SSL_FILETYPE_PEM, SSL_get_peer_certificate */
#include <openssl/err.h>	/* ERR_print_errors_fp */
#include <openssl/x509.h>	/* X509, X509_NAME_oneline, X509_free */
#include <unistd.h>		/* access, F_OK */
#include <stdio.h>		/* printf, stderr */
#include <stdlib.h>		/* exit, EXIT_FAILURE */
#include <string.h>		/* strstr */
#include "ssl_handler.h"	/* CryptoContext, InitCryptoContext,
				 * CleanupCryptoContext, AcceptCryptoConnection,
				 * SecureRecv, SecureSend, 
				 * FreeCryptoConnection, VerifyClientCert */

/*********************************************************/

CryptoContext InitCryptoContext(const char* cert_path, const char* key_path)
{
	SSL_CTX *ctx;
	
	if (access(cert_path, F_OK) == -1 || access(key_path, F_OK) == -1)
	{
		printf("[!] Certificates not found. Falling back to HTTP.\n");
		return NULL;
	}

	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	ctx = SSL_CTX_new(TLS_server_method());
	
	if (!ctx || 
	    SSL_CTX_use_certificate_file(ctx, cert_path, 
					 SSL_FILETYPE_PEM) <= 0 || 
	    SSL_CTX_use_PrivateKey_file(ctx, key_path, 
					SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		
		if (ctx)
		{
			SSL_CTX_free(ctx);
		}
		
		return NULL;
	}
	
	/* Load the CA certificate to establish the Root of Trust */
	if (!SSL_CTX_load_verify_locations(ctx, "certs/ca/ca.crt", NULL)) 
	{
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	/* Request client cert without failing the handshake if missing */
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	
	return (CryptoContext)ctx;
}

/*********************************************************/

void CleanupCryptoContext(CryptoContext ctx)
{
	if (ctx)
	{
		SSL_CTX_free((SSL_CTX*)ctx);
	}
}

/*********************************************************/

CryptoContext AcceptCryptoConnection(CryptoContext server_ctx, int clnt_sock)
{
	SSL *ssl;
	
	if (!server_ctx)
	{
		return NULL;
	}

	ssl = SSL_new((SSL_CTX*)server_ctx);
	
	if (!ssl)
	{
		return NULL;
	}

	SSL_set_fd(ssl, clnt_sock);
	
	if (SSL_accept(ssl) <= 0)
	{
		ERR_print_errors_fp(stderr);
		SSL_free(ssl);
		return NULL;
	}
	
	return (CryptoContext)ssl;
}

/*********************************************************/

ssize_t SecureRecv(CryptoContext ctx, int sock, void *buf, size_t len)
{
	if (ctx)
	{
		return SSL_read((SSL*)ctx, buf, len);
	}
	
	return recv(sock, buf, len, 0);
}

/*********************************************************/

ssize_t SecureSend(CryptoContext ctx, int sock, const void *buf, size_t len)
{
	if (ctx)
	{
		return SSL_write((SSL*)ctx, buf, len);
	}
	
	return send(sock, buf, len, 0);
}

/*********************************************************/

void FreeCryptoConnection(CryptoContext ctx)
{
	if (ctx)
	{
		SSL_free((SSL*)ctx);
	}
}

/*********************************************************/

int VerifyClientCert(CryptoContext ctx, const char *required_string)
{
	SSL *ssl;
	X509 *cert;
	char subject_name[256];
	int is_valid = 0;

	if (!ctx)
	{
		return 0;
	}

	ssl = (SSL*)ctx;
	cert = SSL_get_peer_certificate(ssl);
	
	if (!cert)
	{
		return 0;
	}

	if (required_string == NULL)
	{
		is_valid = 1;
	}
	else
	{
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name,
				  sizeof(subject_name));
							
		if (strstr(subject_name, required_string) != NULL)
		{
			is_valid = 1;
		}
	}

	X509_free(cert);
	
	return is_valid;
}

#endif /* USE_SSL */
