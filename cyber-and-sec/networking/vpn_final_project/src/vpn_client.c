#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: Core VPN Client with unified event loop.
* Author: Moshe Yifrah
*
*********************************************************/

/* Includes *********************************************/

#include <stdio.h>		/* printf, perror, fprintf */
#include <stdlib.h>		/* exit, EXIT_FAILURE, EXIT_SUCCESS, atoi */
#include <string.h>		/* strncpy, memcpy */
#include <unistd.h>		/* close, read, write */
#include <net/if.h>		/* IF_NAMESIZE */
#include <arpa/inet.h>  /* htonl , htons */
#include <sys/uio.h>	/* struct iovec */

#include "vpn_core.h"
#include "network_utils.h"
#include "tun_interface.h"
#include "protocol.h"
#include "os_routing.h"

/* Constants & Macros ***********************************/

#define IS_CLIENT		0
#define ARG_IDX_SVRIP 	1
#define ARG_IDX_PORT 	2
#define ARG_IDX_KEY 	3
#define ARG_IDX_TUN		4
#define ARG_IDX_VIP		5
#define ARG_IDX_SUBNET	6
#define EXPECTED_ARGS	7

/* Implementation Functions *****************************/

static void send_auth_request(int udp_sock, struct sockaddr_in* serv_addr);
static void handle_udp_inbound(int udp_sock, int tun_fd, crypto_ctx_t* crypto, 
			void* context);
static void handle_tun_outbound(int tun_fd, int udp_sock, crypto_ctx_t* crypto, 
			void* context);

/*********************************************************/

int main(int argc, char *argv[])
{
	unsigned short port = 0;
	int udp_sock = -1;
	int tun_fd = -1;
	uint8_t key[CRYPTO_KEY_SIZE] = {0};
	crypto_ctx_t* crypto = NULL;
	struct sockaddr_in serv_addr = {0};
	char tun_name[IF_NAMESIZE] = {0};
	char vip_client[IF_NAMESIZE] = {0};
	char dest_subnet[IF_NAMESIZE] = {0};

	if (EXPECTED_ARGS != argc)
	{
		printf("Usage: %s <Server IP> <Port> <Key> <TUN> <Client VIP> "
			   "<Dest Subnet>\n", argv[0]);
		return EXIT_FAILURE;
	}

	port = (unsigned short)atoi(argv[ARG_IDX_PORT]);
	
	strncpy(tun_name, argv[ARG_IDX_TUN], IF_NAMESIZE - 1);
	tun_name[IF_NAMESIZE - 1] = '\0';
	
	strncpy(vip_client, argv[ARG_IDX_VIP], IF_NAMESIZE - 1);
	vip_client[IF_NAMESIZE - 1] = '\0';
	
	strncpy(dest_subnet, argv[ARG_IDX_SUBNET], IF_NAMESIZE - 1);
	dest_subnet[IF_NAMESIZE - 1] = '\0';

	if (SUCCESS_CODE != load_shared_key(argv[ARG_IDX_KEY], key))
	{
		printf("Failed to load cryptographic key.\n");
		return EXIT_FAILURE;
	}

	crypto = crypto_init(key);
	
	if (NULL == crypto)
	{
		printf("Failed to initialize crypto engine.\n");
		return EXIT_FAILURE;
	}

	udp_sock = create_udp_socket();
	
	if (SUCCESS_CODE > udp_sock)
	{
		crypto_cleanup(crypto);
		return EXIT_FAILURE;
	}

	init_sockaddr(&serv_addr, argv[ARG_IDX_SVRIP], port);

	tun_fd = tun_alloc(tun_name);
	
	if (SUCCESS_CODE != route_setup_client(tun_name, vip_client, dest_subnet))
	{
		printf("Failed to configure client routing.\n");
		close(tun_fd);
		close(udp_sock);
		crypto_cleanup(crypto);
		return EXIT_FAILURE;
	}

	printf("Client started. Server: %s:%u, TUN: %s\n", argv[ARG_IDX_SVRIP], 
			port, tun_name);

	setup_signal_handlers();
	send_auth_request(udp_sock, &serv_addr);
	
	run_vpn_loop(udp_sock, tun_fd, crypto, handle_udp_inbound, 
		     handle_tun_outbound, (void*)&serv_addr);

	printf("\n[*] Interrupt received. Initiating client teardown...\n");
	
	route_cleanup(tun_name, NULL, IS_CLIENT);
	
	if (NULL != crypto)
	{
		crypto_cleanup(crypto);
	}
	
	close(tun_fd);
	close(udp_sock);

	printf("[+] Local routing reverted, TUN destroyed, and memory freed.\n");

	return EXIT_SUCCESS;
}

/*********************************************************/

static void send_auth_request(int udp_sock, struct sockaddr_in* serv_addr)
{
	vpn_header_t auth_hdr = {0};
	uint8_t out_buf[BUFFER_SIZE] = {0};

	if (NULL == serv_addr)
	{
		return;
	}

	auth_hdr.type = MSG_AUTH_REQUEST;
	auth_hdr.client_id = 0;
	auth_hdr.payload_len = 0;

	memcpy(out_buf, &auth_hdr, sizeof(vpn_header_t));

	sendto(udp_sock, out_buf, sizeof(vpn_header_t), 0, 
	       (struct sockaddr*)serv_addr, sizeof(*serv_addr));
	       
	printf("Sent Authentication Request to server.\n");
}

/*********************************************************/

static void handle_udp_inbound(int udp_sock, int tun_fd, crypto_ctx_t* crypto, 
			void* context)
{
	uint8_t buffer[BUFFER_SIZE] = {0};
	uint8_t plaintext[BUFFER_SIZE] = {0};
	struct sockaddr_in serv_addr = {0};
	socklen_t serv_len = sizeof(serv_addr);
	ssize_t bytes_recv = 0;
	vpn_header_t header = {0};
	int plaintext_len = 0;

	if (NULL == crypto)
	{
		return;
	}
	
	(void)context;

	bytes_recv = recvfrom(udp_sock, buffer, BUFFER_SIZE, 0, 
			      (struct sockaddr*)&serv_addr, &serv_len);

	if (0 >= bytes_recv || sizeof(vpn_header_t) > (size_t)bytes_recv)
	{
		return;
	}

	memcpy(&header, buffer, sizeof(vpn_header_t));

	if (MSG_AUTH_SUCCESS == header.type)
	{
		printf("Authentication successful! Tunnel is active.\n");
		return;
	}

	if (MSG_DATA == header.type)
	{
		if (SUCCESS_CODE != crypto_decrypt(crypto, buffer + sizeof(vpn_header_t), 
						   bytes_recv - sizeof(vpn_header_t), 
						   plaintext, &plaintext_len))
		{
			fprintf(stderr, "[Security Warning] Crypto engine failure /" 
				"invalid MAC.Packet dropped.\n");
			return;
		}

		write(tun_fd, plaintext, plaintext_len);
	}
}

/*********************************************************/

static void handle_tun_outbound(int tun_fd, int udp_sock, crypto_ctx_t* crypto, 
			void* context)
{
	uint8_t packet_buf[BUFFER_SIZE] = {0};
	uint8_t ciphertext[BUFFER_SIZE] = {0};
	ssize_t bytes_read = 0;
	vpn_header_t header = {0};
	int ciphertext_len = 0;
	struct sockaddr_in* serv_addr = (struct sockaddr_in*)context;
	struct iovec iov[2] = {{0}};
	struct msghdr msg = {0};

	if (NULL == serv_addr || NULL == crypto)
	{
		return;
	}

	bytes_read = read(tun_fd, packet_buf, MAX_PAYLOAD_READ);
	
	if (0 > bytes_read)
	{
		perror("read_tun");
		return;
	}

	if (IPV4_MIN_HEADER_LEN > bytes_read)
	{
		return; 
	}

	if (SUCCESS_CODE != crypto_encrypt(crypto, packet_buf, (int)bytes_read, 
					   ciphertext, &ciphertext_len))
	{
		return;
	}

	header.type = MSG_DATA;
	header.client_id = htonl(0); 
	header.payload_len = htons((uint16_t)ciphertext_len);

	iov[0].iov_base = &header;
	iov[0].iov_len = sizeof(vpn_header_t);
	
	iov[1].iov_base = ciphertext;
	iov[1].iov_len = (size_t)ciphertext_len;

	msg.msg_name = serv_addr;
	msg.msg_namelen = sizeof(struct sockaddr_in);
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	sendmsg(udp_sock, &msg, 0);
}
