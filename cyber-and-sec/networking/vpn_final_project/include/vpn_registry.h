/*********************************************************
*
* Description: Client registry for UDP state management.
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef VPN_REGISTRY_H
#define VPN_REGISTRY_H

/* Includes *********************************************/

#include <netinet/in.h>	/* struct sockaddr_in */
#include <stdint.h>		/* uint32_t */

/* Constants & Macros ***********************************/

#define MAX_CLIENTS		10

/* Types ************************************************/

typedef struct
{
	struct sockaddr_in ext_addr;
	uint32_t virtual_ip;
	int is_active;
} client_record_t;

/* Implementation Functions *****************************/

void registry_init(void);
int registry_add_client(const struct sockaddr_in* addr, uint32_t vip);
client_record_t* registry_get_by_addr(const struct sockaddr_in* addr);
client_record_t* registry_get_by_vip(uint32_t vip);

#endif /* VPN_REGISTRY_H */
