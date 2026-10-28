/*********************************************************
*
* Description: Shared utility declarations for UDP socket helpers
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef UDP_UTILS_H
#define UDP_UTILS_H

#define _POSIX_C_SOURCE 200112L

/* Includes ******************************************************/

#include <sys/socket.h>	/* socklen_t */
#include <netinet/in.h>	/* struct sockaddr_in */

/* Macros *******************************************************/

#define BUFFER_SIZE	256

/* Implemention Functions ***********************************/

void die_with_error(const char*);
int create_udp_socket(void);
void init_sockaddr(struct sockaddr_in*, const char*, unsigned short);
void get_user_input(char*, int);

#endif /* UDP_UTILS_H */
