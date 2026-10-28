/*********************************************************
*
* Description: OpenSSL EVP wrappers for symmetric AES encryption.
* Author: Moshe Yifrah
*
*********************************************************/

#ifndef CRYPTO_ENGINE_H
#define CRYPTO_ENGINE_H

/* Includes *********************************************/

#include <stddef.h>		/* size_t */
#include <stdint.h>		/* uint8_t */

/* Constants & Macros ***********************************/

#define CRYPTO_KEY_SIZE		32
#define CRYPTO_IV_SIZE		16
#define CRYPTO_BLOCK_SIZE	16

/* Types ************************************************/

/* Opaque pointer to hide OpenSSL internals from the rest of the app */
typedef struct crypto_ctx_t crypto_ctx_t;

/* Implementation Functions *****************************/

crypto_ctx_t* crypto_init(const uint8_t* key);
void crypto_cleanup(crypto_ctx_t* ctx);
int crypto_encrypt(crypto_ctx_t* ctx, const uint8_t* plaintext, 
		   int plaintext_len, uint8_t* ciphertext, 
		   int* ciphertext_len);
int crypto_decrypt(crypto_ctx_t* ctx, const uint8_t* ciphertext, 
		   int ciphertext_len, uint8_t* plaintext, 
		   int* plaintext_len);

#endif /* CRYPTO_ENGINE_H */
