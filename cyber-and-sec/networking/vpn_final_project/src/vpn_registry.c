#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: Client registry for UDP state management.
* Author: Moshe Yifrah
*
*********************************************************/

/* Includes *********************************************/

#include <string.h>		/* memset, memcmp */
#include <stddef.h>		/* NULL */

#include "vpn_registry.h"

/* Constants & Macros ***********************************/

#define SUCCESS_CODE	0
#define ERROR_CODE		-1

/* Globals **********************************************/

static client_record_t g_clients[MAX_CLIENTS];

/* Implementation Functions *****************************/

void registry_init(void)
{
	memset(g_clients, 0, sizeof(g_clients));
}

/*********************************************************/

int registry_add_client(const struct sockaddr_in* addr, uint32_t vip)
{
	int i = 0;
	client_record_t* existing = NULL;

	if (NULL == addr)
	{
		return ERROR_CODE;
	}

	/* Prevent State Exhaustion (DoS) by verifying uniqueness */
	existing = registry_get_by_addr(addr);
	
	if (NULL != existing)
	{
		/* Client already registered, avoid consuming a new slot */
		return SUCCESS_CODE;
	}

	for (i = 0; MAX_CLIENTS > i; ++i)
	{
		if (0 == g_clients[i].is_active)
		{
			g_clients[i].ext_addr = *addr;
			g_clients[i].virtual_ip = vip;
			g_clients[i].is_active = 1;
			
			return SUCCESS_CODE;
		}
	}

	return ERROR_CODE;
}

/*********************************************************/

client_record_t* registry_get_by_addr(const struct sockaddr_in* addr)
{
	int i = 0;

	if (NULL == addr)
	{
		return NULL;
	}

	for (i = 0; MAX_CLIENTS > i; ++i)
	{
		if (1 == g_clients[i].is_active)
		{
			/* Compare raw memory of the IP and check exact port */
			if (0 == memcmp(&(g_clients[i].ext_addr.sin_addr), 
					&(addr->sin_addr), 
					sizeof(addr->sin_addr)) &&
			    g_clients[i].ext_addr.sin_port == addr->sin_port)
			{
				return &g_clients[i];
			}
		}
	}

	return NULL;
}

/*********************************************************/

client_record_t* registry_get_by_vip(uint32_t vip)
{
	int i = 0;

	for (i = 0; MAX_CLIENTS > i; ++i)
	{
		if (1 == g_clients[i].is_active && vip == g_clients[i].virtual_ip)
		{
			return &g_clients[i];
		}
	}

	return NULL;
}

/*********************************************************/

