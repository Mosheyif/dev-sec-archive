/*********************************************************
*
* Description: Shared core definitions for VPN event loop.
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef VPN_CORE_H
#define VPN_CORE_H

/* Includes *********************************************/

#include <stdint.h>		/* uint8_t */
#include <signal.h>		/* sig_atomic_t */

#include "crypto_engine.h"

/* Typedefs & Data Structures ***************************/

typedef void (*udp_handler_t)(int udp_sock, int tun_fd, crypto_ctx_t* crypto, 
			void* context);
typedef void (*tun_handler_t)(int tun_fd, int udp_sock, crypto_ctx_t* crypto, 
			void* context);

/* Globals **********************************************/

extern volatile sig_atomic_t g_keep_running;

/* Shared API *******************************************/

void setup_signal_handlers(void);
int load_shared_key(const char* filepath, uint8_t* key_buffer);
void run_vpn_loop(int udp_sock, int tun_fd, crypto_ctx_t* crypto, 
                  udp_handler_t on_udp, tun_handler_t on_tun, void* context);

#endif /* VPN_CORE_H */
