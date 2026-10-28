/*********************************************************
*
* Description: Shared networking utility functions for VPN UDP sockets.
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

/* Includes *********************************************/

#include <sys/types.h>		/* ssize_t */
#include <sys/socket.h>		/* socklen_t */
#include <netinet/in.h>		/* struct sockaddr_in */

/* Constants & Macros ***********************************/

#define BUFFER_SIZE		2048

/* Implementation Functions *****************************/

int create_udp_socket(void);
void init_sockaddr(struct sockaddr_in* addr, const char* ip, unsigned short port);
int validate_system_input(const char* str);

#endif /* NETWORK_UTILS_H */

