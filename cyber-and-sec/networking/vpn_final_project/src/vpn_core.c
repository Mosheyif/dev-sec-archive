#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: Core VPN event loop and shared utilities.
* Author: Moshe Yifrah
*
*********************************************************/

/* Includes *********************************************/

#include <stdio.h>		/* perror */
#include <unistd.h>		/* close, read */
#include <fcntl.h>		/* open, O_RDONLY */
#include <sys/select.h> /* select, fd_set, FD_ZERO, FD_SET, FD_ISSET */
#include <errno.h>		/* errno */
#include <signal.h>		/* sigaction, sigemptyset, SIGINT, SIGTERM */
#include <sys/epoll.h>	/* epoll_create1, epoll_ctl, epoll_wait, epoll_event */

#include "vpn_core.h"
#include "protocol.h"

/* Constants & Macros ***********************************/

#define MAX_EVENTS		10
#define KEY_FILE_FLAGS	O_RDONLY

/* Globals **********************************************/

volatile sig_atomic_t g_keep_running = 1;

/* Implementation Functions *****************************/

static void handle_signal(int sig);

/*********************************************************/

static void handle_signal(int sig)
{
	(void)sig;
	g_keep_running = 0;
}

/*********************************************************/

void setup_signal_handlers(void)
{
	struct sigaction sa = {0};
	
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

/*********************************************************/

int load_shared_key(const char* filepath, uint8_t* key_buffer)
{
	int fd = -1;
	ssize_t bytes_read = 0;

	if (NULL == filepath || NULL == key_buffer)
	{
		return ERROR_CODE;
	}

	fd = open(filepath, KEY_FILE_FLAGS);
	
	if (SUCCESS_CODE > fd)
	{
		return ERROR_CODE;
	}

	bytes_read = read(fd, key_buffer, CRYPTO_KEY_SIZE);
	close(fd);

	if (CRYPTO_KEY_SIZE != bytes_read)
	{
		return ERROR_CODE;
	}

	return SUCCESS_CODE;
}

/*********************************************************/

void run_vpn_loop(int udp_sock, int tun_fd, crypto_ctx_t* crypto, 
		  udp_handler_t on_udp, tun_handler_t on_tun, void* context)
{
	int epfd = -1;
	struct epoll_event ev = {0};
	struct epoll_event events[MAX_EVENTS] = {{0}};
	int nfds = 0;
	int i = 0;

	if (NULL == crypto)
	{
		return;
	}

	epfd = epoll_create1(0);
	if (0 > epfd)
	{
		perror("epoll_create1 failed");
		return;
	}

	ev.events = EPOLLIN;
	ev.data.fd = udp_sock;
	if (SUCCESS_CODE > epoll_ctl(epfd, EPOLL_CTL_ADD, udp_sock, &ev))
	{
		close(epfd);
		return;
	}

	ev.events = EPOLLIN;
	ev.data.fd = tun_fd;
	if (SUCCESS_CODE > epoll_ctl(epfd, EPOLL_CTL_ADD, tun_fd, &ev))
	{
		close(epfd);
		return;
	}

	while (1 == g_keep_running)
	{
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		
		if (0 > nfds)
		{
			if (EINTR == errno)
			{
				continue;
			}
			perror("epoll_wait failed");
			break;
		}

		for (i = 0; nfds > i; ++i)
		{
			if (udp_sock == events[i].data.fd && NULL != on_udp)
			{
				on_udp(udp_sock, tun_fd, crypto, context);
			}
			else if (tun_fd == events[i].data.fd && NULL != on_tun)
			{
				on_tun(tun_fd, udp_sock, crypto, context);
			}
		}
	}
	
	close(epfd);
}
