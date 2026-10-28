#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: OS-level network routing configuration.
* Author: Moshe Yifrah
*
*********************************************************/

/* Includes *********************************************/

#include <stdlib.h>		/* system */
#include <stdio.h>		/* sprintf, snprintf, perror */
#include <string.h>		/* memset */
#include <unistd.h>		/* read, write, close */
#include <fcntl.h>		/* open, O_RDONLY, O_WRONLY */

#include "os_routing.h"
#include "network_utils.h"
#include "protocol.h"

/* Constants & Macros ***********************************/

#define CMD_SIZE	256

/* Implementation Functions *****************************/

int route_setup_server(const char* dev, const char* ip, const char* subnet)
{
	char cmd[CMD_SIZE] = {0};
	int fd_ipf = -1;
	char ipf_val = 0;
	int snprintf_res = 0;
	
	if (NULL == dev || NULL == ip || NULL == subnet)
	{
		return ERROR_CODE;
	}

	/* Prevent Command Injection by sanitizing all external inputs */
	if (SUCCESS_CODE != validate_system_input(dev) ||
		SUCCESS_CODE != validate_system_input(ip) ||
		SUCCESS_CODE != validate_system_input(subnet))
	{
		perror("[Error] Command sanitization check failed!");
		return ERROR_CODE;
	}

	snprintf_res = snprintf(cmd, CMD_SIZE, "iptables -t nat -A POSTROUTING -s " 
					"%s ! -o %s -j MASQUERADE", subnet, dev);
	if (0 > snprintf_res || CMD_SIZE <= snprintf_res)
	{
		perror("[Error] Command formatting failed or truncated!");
		return ERROR_CODE;
	}

	if (SUCCESS_CODE != system(cmd))
	{
		perror("[Error] Failed to execute iptables rule.");
	}

	memset(cmd, 0, CMD_SIZE);
	
	/* Add the virtual IP to the server's TUN interface */
	snprintf_res = snprintf(cmd, CMD_SIZE, "ip addr add %s/24 dev %s", ip, dev);
	if (0 > snprintf_res || CMD_SIZE <= snprintf_res)
	{
		perror("[Error] Command formatting failed or truncated!");
		return ERROR_CODE;
	}

	if (SUCCESS_CODE != system(cmd))
	{
		perror("[Error] Failed to assign IP to server TUN!");
		return ERROR_CODE;
	}

	memset(cmd, 0, CMD_SIZE);
	
	snprintf_res = snprintf(cmd, CMD_SIZE, "ip link set dev %s up", dev);
	if (0 > snprintf_res || CMD_SIZE <= snprintf_res)
	{
		perror("[Error] Command formatting failed or truncated!");
		return ERROR_CODE;
	}

	if (SUCCESS_CODE != system(cmd))
	{
		perror("[Error] IP link set failed!");
		return ERROR_CODE;
	}
	
	memset(cmd, 0, CMD_SIZE);
	
	/* 1. Check current forwarding state */
	fd_ipf = open("/proc/sys/net/ipv4/ip_forward", O_RDONLY);
	if (SUCCESS_CODE <= fd_ipf)
	{
		if (1 == read(fd_ipf, &ipf_val, 1) && '1' == ipf_val)
		{
			/* Forwarding is already enabled. */
			close(fd_ipf);
		}
		else
		{
			close(fd_ipf);
			
			/* 2. Forwarding is disabled. Attempt to enable it. */
			fd_ipf = open("/proc/sys/net/ipv4/ip_forward", O_WRONLY);
			
			if (SUCCESS_CODE > fd_ipf)
			{
				return ERROR_CODE;
			}
			
			if (1 != write(fd_ipf, "1", 1))
			{
				close(fd_ipf);
				return ERROR_CODE;
			}
			
			close(fd_ipf);
		}
	}
	else
	{
		perror("[Error] IP forwarding failed!");
		return ERROR_CODE;
	}

	memset(cmd, 0, CMD_SIZE);

	return SUCCESS_CODE;
}

/*********************************************************/

int route_setup_client(const char* dev, const char* ip, const char* dest_subnet)
{
	char cmd[CMD_SIZE] = {0};
	int snprintf_res = 0;

	if (NULL == dev || NULL == ip || NULL == dest_subnet)
	{
		return ERROR_CODE;
	}

	/* Prevent Command Injection by sanitizing all external inputs */
	if (SUCCESS_CODE != validate_system_input(dev) ||
		SUCCESS_CODE != validate_system_input(ip) ||
		SUCCESS_CODE != validate_system_input(dest_subnet))
	{
		perror("[Error] Command sanitization check failed!");
		return ERROR_CODE;
	}

	snprintf_res = snprintf(cmd, CMD_SIZE, "ip addr add %s/24 dev %s", ip, dev);
	if (0 > snprintf_res || CMD_SIZE <= snprintf_res)
	{
		perror("[Error] Command formatting failed or truncated!");
		return ERROR_CODE;
	}
	
	if (SUCCESS_CODE != system(cmd))
	{
		perror("[Error] Adding IP address failed!");
		return ERROR_CODE;
	}
	
	memset(cmd, 0, CMD_SIZE);
	
	snprintf_res = snprintf(cmd, CMD_SIZE, "ip link set dev %s up", dev);
	if (0 > snprintf_res || CMD_SIZE <= snprintf_res)
	{
		perror("[Error] Command formatting failed or truncated!");
		return ERROR_CODE;
	}

	if (SUCCESS_CODE != system(cmd))
	{
		perror("[Error] IP link set failed!");
		return ERROR_CODE;
	}

	memset(cmd, 0, CMD_SIZE);

	/* Routing target subnet traffic into the TUN interface */
	snprintf_res = snprintf(cmd, CMD_SIZE, "ip route add %s dev %s > /dev/null 2>&1", 
				dest_subnet, dev);
	if (0 > snprintf_res || CMD_SIZE <= snprintf_res)
	{
		perror("[Error] Command formatting failed or truncated!");
		return ERROR_CODE;
	}
	
	if (SUCCESS_CODE != system(cmd))
	{
		printf("[Warning] Route %s via %s already exists or failed to add.\n", 
		       dest_subnet, dev);
	}
	
	memset(cmd, 0, CMD_SIZE);

	return SUCCESS_CODE;
}

/*********************************************************/

void route_cleanup(const char* dev, const char* network, int is_server)
{
	char cmd[CMD_SIZE] = {0};
	int snprintf_res = 0;

	if (NULL == dev)
	{
		return;
	}

	if (1 == is_server && NULL != network)
	{
		snprintf_res = snprintf(cmd, CMD_SIZE, "iptables -t nat -D "
					"POSTROUTING -s %s ! -o %s -j MASQUERADE "
					"2>/dev/null", network, dev);
		if (0 <= snprintf_res && CMD_SIZE > snprintf_res)
		{
			system(cmd);
		}
		
		memset(cmd, 0, CMD_SIZE);
	}

	snprintf_res = snprintf(cmd, CMD_SIZE, "ip link delete %s 2>/dev/null", dev);
	if (0 <= snprintf_res && CMD_SIZE > snprintf_res)
	{
		system(cmd);
	}
}
