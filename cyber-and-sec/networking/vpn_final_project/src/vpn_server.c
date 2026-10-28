#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: Core VPN Server with unified event loop.
* Author: Moshe Yifrah
*
*********************************************************/

/* Includes *********************************************/

#include <stdio.h>		/* printf, perror, fprintf */
#include <stdlib.h>		/* exit, EXIT_FAILURE, EXIT_SUCCESS, atoi */
#include <string.h>		/* strncpy, memcpy */
#include <unistd.h>		/* close, read, write */
#include <net/if.h>		/* IF_NAMESIZE */
#include <arpa/inet.h>	/* ntohl, htonl, htons */
#include <sys/uio.h>	/* struct iovec */

#include "vpn_core.h"
#include "network_utils.h"
#include "tun_interface.h"
#include "protocol.h"
#include "vpn_registry.h"
#include "os_routing.h"

/* Constants & Macros ***********************************/

#define IS_SERVER			1
#define ARG_IDX_PORT 		1
#define ARG_IDX_KEY 		2
#define ARG_IDX_TUN			3
#define ARG_IDX_VIP			4
#define ARG_IDX_SUBNET		5
#define EXPECTED_ARGS		6
#define IPV4_DEST_OFFSET	16

/* Globals **********************************************/

static uint32_t g_next_client_vip;

/* Implementation Functions *****************************/

static void handle_udp_inbound(int udp_sock, int tun_fd, crypto_ctx_t* crypto, 
			void* context);
static void handle_tun_outbound(int tun_fd, int udp_sock, crypto_ctx_t* crypto, 
			void* context);
static void process_auth_request(int udp_sock, struct sockaddr_in* client_addr);

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
	char vip_server[IF_NAMESIZE] = {0};
	char subnet[IF_NAMESIZE] = {0};

	if (EXPECTED_ARGS != argc)
	{
		printf("Usage: %s <Port> <Key> <TUN> <VIP> <Subnet>\n", argv[0]);
		return EXIT_FAILURE;
	}

	port = (unsigned short)atoi(argv[ARG_IDX_PORT]);
	
	strncpy(tun_name, argv[ARG_IDX_TUN], IF_NAMESIZE - 1);
	tun_name[IF_NAMESIZE - 1] = '\0';
	
	strncpy(vip_server, argv[ARG_IDX_VIP], IF_NAMESIZE - 1);
	vip_server[IF_NAMESIZE - 1] = '\0';
	
	strncpy(subnet, argv[ARG_IDX_SUBNET], IF_NAMESIZE - 1);
	subnet[IF_NAMESIZE - 1] = '\0';

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

	init_sockaddr(&serv_addr, "0.0.0.0", port);
	
	if (SUCCESS_CODE > bind(udp_sock, (struct sockaddr*)&serv_addr, 
				sizeof(serv_addr)))
	{
		perror("bind() failed");
		close(udp_sock);
		crypto_cleanup(crypto);
		return EXIT_FAILURE;
	}

	tun_fd = tun_alloc(tun_name);
	
	if (SUCCESS_CODE != route_setup_server(tun_name, vip_server, subnet))
	{
		printf("Failed to configure server routing.\n");
		close(tun_fd);
		close(udp_sock);
		crypto_cleanup(crypto);
		return EXIT_FAILURE;
	}

	printf("Server listening on UDP %u, TUN interface: %s\n", port, tun_name);
	fflush(stdout);
	
	setup_signal_handlers();
	registry_init();
	
	g_next_client_vip = ntohl(inet_addr(vip_server)) + 1;
	
	run_vpn_loop(udp_sock, tun_fd, crypto, handle_udp_inbound, 
		     handle_tun_outbound, NULL);

	printf("\n[*] Interrupt received. Initiating server teardown...\n");
	
	route_cleanup(tun_name, subnet, IS_SERVER);
	
	if (NULL != crypto)
	{
		crypto_cleanup(crypto);
	}
	
	close(tun_fd);
	close(udp_sock);

	printf("[+] Netfilter rules reverted, TUN destroyed, and memory freed.\n");

	return EXIT_SUCCESS;
}

/*********************************************************/

static void handle_udp_inbound(int udp_sock, int tun_fd, crypto_ctx_t* crypto, 
			void* context)
{
	uint8_t buffer[BUFFER_SIZE] = {0};
	uint8_t plaintext[BUFFER_SIZE] = {0};
	struct sockaddr_in client_addr = {0};
	socklen_t client_len = sizeof(client_addr);
	ssize_t bytes_recv = 0;
	client_record_t* client = NULL;
	vpn_header_t header = {0};
	int plaintext_len = 0;

	(void)context;
	
	if (NULL == crypto)
	{
		return;
	}

	bytes_recv = recvfrom(udp_sock, buffer, BUFFER_SIZE, 0, 
			      (struct sockaddr*)&client_addr, &client_len);

	if (0 >= bytes_recv || sizeof(vpn_header_t) > (size_t)bytes_recv)
	{
		return;
	}

	memcpy(&header, buffer, sizeof(vpn_header_t));
	client = registry_get_by_addr(&client_addr);

	if (NULL == client)
	{
		if (MSG_AUTH_REQUEST == header.type)
		{
			process_auth_request(udp_sock, &client_addr);
		}
		return;
	}

	if (MSG_DATA == header.type)
	{
		if (SUCCESS_CODE != crypto_decrypt(crypto, buffer + sizeof(vpn_header_t), 
				bytes_recv - sizeof(vpn_header_t), plaintext, &plaintext_len))
		{
			fprintf(stderr, "[Security Warning] Crypto engine failure /" 
					"invalid MAC.Packet dropped.\n");
			return;
		}

		write(tun_fd, plaintext, plaintext_len);
	}
}

/*********************************************************/

static void process_auth_request(int udp_sock, struct sockaddr_in* client_addr)
{
	vpn_header_t auth_reply = {0};
	uint8_t reply_buf[BUFFER_SIZE] = {0};
	uint32_t allocated_vip = 0;

	if (NULL == client_addr)
	{
		return;
	}

	allocated_vip = g_next_client_vip++;

	if (SUCCESS_CODE != registry_add_client(client_addr, allocated_vip))
	{
		return;
	}

	auth_reply.type = MSG_AUTH_SUCCESS;
	auth_reply.client_id = htonl(allocated_vip);
	auth_reply.payload_len = 0;

	memcpy(reply_buf, &auth_reply, sizeof(vpn_header_t));
	sendto(udp_sock, reply_buf, sizeof(vpn_header_t), 0,
	       (struct sockaddr*)client_addr, sizeof(*client_addr));
}

/*********************************************************/

static void handle_tun_outbound(int tun_fd, int udp_sock, crypto_ctx_t* crypto, 
			void* context)
{
	uint8_t packet_buf[BUFFER_SIZE] = {0};
	uint8_t ciphertext[BUFFER_SIZE] = {0};
	ssize_t bytes_read = 0;
	uint32_t dest_ip = 0;
	client_record_t* client = NULL;
	vpn_header_t header = {0};
	int ciphertext_len = 0;
	struct iovec iov[2] = {{0}};
	struct msghdr msg = {0};

	(void)context;
	
	if (NULL == crypto)
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

	memcpy(&dest_ip, packet_buf + IPV4_DEST_OFFSET, sizeof(uint32_t));
	dest_ip = ntohl(dest_ip);

	client = registry_get_by_vip(dest_ip);
	
	if (NULL == client)
	{
		return;
	}

	if (SUCCESS_CODE != crypto_encrypt(crypto, packet_buf, (int)bytes_read, 
					   ciphertext, &ciphertext_len))
	{
		return;
	}

	header.type = MSG_DATA;
	header.client_id = htonl(dest_ip);
	header.payload_len = htons((uint16_t)ciphertext_len);

	iov[0].iov_base = &header;
	iov[0].iov_len = sizeof(vpn_header_t);
	
	iov[1].iov_base = ciphertext;
	iov[1].iov_len = (size_t)ciphertext_len;

	msg.msg_name = &(client->ext_addr);
	msg.msg_namelen = sizeof(struct sockaddr_in);
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	sendmsg(udp_sock, &msg, 0);
}
