#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: TUN interface allocation and initialization.
* Author: Moshe Yifrah
*
*********************************************************/

/* Includes *********************************************/

#include <stdio.h>		  /* perror */
#include <string.h>		  /* memset, strncpy, strcpy */
#include <unistd.h>		  /* close */
#include <fcntl.h>		  /* open, O_RDWR */
#include <sys/ioctl.h>	  /* ioctl */
#include <sys/socket.h>   /* struct sockaddr */
#include <net/if.h>		  /* struct ifreq, IF_NAMESIZE */
#include <linux/if_tun.h> /* IFF_TUN, IFF_NO_PI, TUNSETIFF */

#include "tun_interface.h"

/*********************************************************/

int tun_alloc(char* dev)
{
	struct ifreq ifr = {0};
	int fd = -1;
	int err = 0;
	
	if (NULL == dev)
	{
		return -1;
	}
	
	fd = open("/dev/net/tun", O_RDWR);
	
	if (0 > fd)
	{
		perror("open(/dev/net/tun) failed");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

	if ('\0' != *dev)
	{
		strncpy(ifr.ifr_name, dev, IF_NAMESIZE - 1);
		ifr.ifr_name[IF_NAMESIZE - 1] = '\0';
	}

	err = ioctl(fd, TUNSETIFF, (void*)&ifr);
	
	if (0 > err)
	{
		perror("ioctl(TUNSETIFF) failed");
		close(fd);
		return -1;
	}

	strncpy(dev, ifr.ifr_name, IF_NAMESIZE - 1);
	dev[IF_NAMESIZE - 1] = '\0';
	
	return fd;
}
