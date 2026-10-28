/*********************************************************
*
* Description: Protocol definitions and packet headers.
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef PROTOCOL_H
#define PROTOCOL_H

/* Includes *********************************************/

#include <stdint.h>		/* uint8_t, uint16_t, uint32_t */

/* Global Macros ****************************************/

#define SUCCESS_CODE		0
#define ERROR_CODE			-1
#define IPV4_MIN_HEADER_LEN 20
#define BUFFER_SIZE			2048
#define CRYPTO_KEY_SIZE		32  /* AES-256 */
#define CRYPTO_IV_SIZE		16  /* AES block size */
#define CRYPTO_BLOCK_SIZE	16
#define MAX_PAYLOAD_READ (BUFFER_SIZE - sizeof(vpn_header_t) - \
			  CRYPTO_IV_SIZE - CRYPTO_BLOCK_SIZE)

/* Types ************************************************/

typedef enum
{
	MSG_AUTH_REQUEST = 1,
	MSG_AUTH_SUCCESS = 2,
	MSG_AUTH_FAILED  = 3,
	MSG_DATA         = 4,
	MSG_KEEPALIVE    = 5
} msg_type_t;

#pragma pack(push, 1)
typedef struct
{
	uint8_t type;
	uint32_t client_id;
	uint16_t payload_len;
} vpn_header_t;
#pragma pack(pop)

#endif /* PROTOCOL_H */
