#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: Shared networking utility functions for VPN UDP sockets.
* Author: Moshe Yifrah
*
*********************************************************/

/* Includes *********************************************/

#include <stdio.h>		/* perror */
#include <string.h>		/* memset */
#include <sys/socket.h>	/* socket, AF_INET, SOCK_DGRAM */
#include <arpa/inet.h>	/* inet_addr, htons */
#include <ctype.h>		/* isalnum */

#include "network_utils.h"
#include "protocol.h"

/*********************************************************/

int create_udp_socket(void)
{
	int sock = -1;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (0 > sock)
	{
		perror("socket() failed");
		return -1;
	}
	
	return sock;
}

/*********************************************************/

void init_sockaddr(struct sockaddr_in* addr, const char* ip, 
		   unsigned short port)
{
	if (NULL == addr || NULL == ip)
	{
		return;
	}
	
	memset(addr, 0, sizeof(*addr));
	
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr(ip);
	addr->sin_port = htons(port);
}

/*********************************************************/

int validate_system_input(const char* str)
{
	if (NULL == str)
	{
		return ERROR_CODE;
	}

	/* Validate string against allowed characters for networking commands */
	while ('\0' != *str)
	{
		if (0 == isalnum((unsigned char)*str) && 
		    '.' != *str && 
		    '/' != *str && 
		    '-' != *str)
		{
			return ERROR_CODE;
		}
		
		++str;
	}

	return SUCCESS_CODE;
}

