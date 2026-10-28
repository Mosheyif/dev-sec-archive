/*********************************************************
*
* Description: OS-level network routing configuration.
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef OS_ROUTING_H
#define OS_ROUTING_H

/* Implementation Functions *****************************/

int route_setup_server(const char* dev, const char* ip, const char* subnet);
int route_setup_client(const char* dev, const char* ip, const char* dest_subnet);
void route_cleanup(const char* dev, const char* network, int is_server);

#endif /* OS_ROUTING_H */
