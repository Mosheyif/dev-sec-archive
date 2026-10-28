/*********************************************************
*
* Description: SSL/TLS context abstraction — wraps OpenSSL EVP
*              with macro fallbacks for plain-socket mode
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef SSL_HANDLER_H
#define SSL_HANDLER_H

/* Includes ******************************************************/

#include <sys/types.h>		/* ssize_t */
#include <sys/socket.h>		/* recv, send */

/* Typedefs ******************************************************/

typedef void* CryptoContext;

#ifdef USE_SSL

/* Implementation Functions **************************************/

CryptoContext InitCryptoContext(const char* cert_path, const char* key_path);
void CleanupCryptoContext(CryptoContext ctx);
CryptoContext AcceptCryptoConnection(CryptoContext server_ctx, int clnt_sock);
ssize_t SecureRecv(CryptoContext ctx, int sock, void *buf, size_t len);
ssize_t SecureSend(CryptoContext ctx, int sock, const void *buf, size_t len);
void FreeCryptoConnection(CryptoContext ctx);
int VerifyClientCert(CryptoContext ctx, const char *required_string);

#else

/* Fallback Macros ***********************************************/

#define InitCryptoContext(cert, key) (NULL)
#define CleanupCryptoContext(ctx) ((void)0)
#define AcceptCryptoConnection(serv, sock) (NULL)
#define SecureRecv(ctx, sock, buf, len) recv(sock, buf, len, 0)
#define SecureSend(ctx, sock, buf, len) send(sock, buf, len, 0)
#define FreeCryptoConnection(ctx) ((void)0)
#define VerifyClientCert(ctx, req_str) (1)

#endif /* USE_SSL */

#endif /* SSL_HANDLER_H */
