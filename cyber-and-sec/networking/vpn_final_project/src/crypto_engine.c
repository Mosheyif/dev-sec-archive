#define _POSIX_C_SOURCE 200112L

/*********************************************************
*
* Description: OpenSSL EVP wrappers for symmetric AES encryption.
* Author: Moshe Yifrah
*
*********************************************************/

/* Includes *********************************************/

#include <stdlib.h>		  /* malloc, free */
#include <string.h>		  /* memcpy */
#include <openssl/evp.h>  /* EVP_CIPHER_CTX, EVP_aes_256_cbc */
#include <openssl/rand.h> /* RAND_bytes */

#include "crypto_engine.h"
#include "protocol.h"

/* Constants & Macros ***********************************/

#define OPENSSL_SUCCESS	1

/* Types ************************************************/

struct crypto_ctx_t
{
	EVP_CIPHER_CTX* evp_enc_ctx;
	EVP_CIPHER_CTX* evp_dec_ctx;
	uint8_t key[CRYPTO_KEY_SIZE];
};

/* Implementation Functions *****************************/

crypto_ctx_t* crypto_init(const uint8_t* key)
{
	crypto_ctx_t* ctx = NULL;
	
	if (NULL == key)
	{
		return NULL;
	}

	ctx = (crypto_ctx_t*)malloc(sizeof(crypto_ctx_t));
	
	if (NULL == ctx)
	{
		return NULL;
	}

	memcpy(ctx->key, key, CRYPTO_KEY_SIZE);

	/* Allocate independent contexts for TX and RX paths */
	ctx->evp_enc_ctx = EVP_CIPHER_CTX_new();
	ctx->evp_dec_ctx = EVP_CIPHER_CTX_new();
	
	if (NULL == ctx->evp_enc_ctx || NULL == ctx->evp_dec_ctx)
	{
		crypto_cleanup(ctx);
		return NULL;
	}

	return ctx;
}

/*********************************************************/

void crypto_cleanup(crypto_ctx_t* ctx)
{
	if (NULL != ctx)
	{
		if (NULL != ctx->evp_enc_ctx)
		{
			EVP_CIPHER_CTX_free(ctx->evp_enc_ctx);
		}
		if (NULL != ctx->evp_dec_ctx)
		{
			EVP_CIPHER_CTX_free(ctx->evp_dec_ctx);
		}
		free(ctx);
	}
}

/*********************************************************/

int crypto_encrypt(crypto_ctx_t* ctx, const uint8_t* plaintext, int plaintext_len,
					 uint8_t* ciphertext, int* ciphertext_len)
{
	int len = 0;
	int total_len = 0;
	uint8_t iv[CRYPTO_IV_SIZE] = {0};

	if (NULL == ctx || NULL == plaintext || NULL == ciphertext || 
		NULL == ciphertext_len)
	{
		return ERROR_CODE;
	}

	if (OPENSSL_SUCCESS != RAND_bytes(iv, CRYPTO_IV_SIZE))
	{
		return ERROR_CODE;
	}

	memcpy(ciphertext, iv, CRYPTO_IV_SIZE);

	if (OPENSSL_SUCCESS != EVP_EncryptInit_ex(ctx->evp_enc_ctx, EVP_aes_256_cbc(), 
			NULL, ctx->key, iv))
	{
		return ERROR_CODE;
	}

	if (OPENSSL_SUCCESS != EVP_EncryptUpdate(ctx->evp_enc_ctx, 
			ciphertext + CRYPTO_IV_SIZE, &len, plaintext, plaintext_len))
	{
		return ERROR_CODE;
	}
	
	total_len = len;

	if (OPENSSL_SUCCESS != EVP_EncryptFinal_ex(ctx->evp_enc_ctx, 
					        ciphertext + CRYPTO_IV_SIZE + len, &len))
	{
		return ERROR_CODE;
	}
	
	total_len += len;
	*ciphertext_len = CRYPTO_IV_SIZE + total_len;

	return SUCCESS_CODE;
}

/*********************************************************/

int crypto_decrypt(crypto_ctx_t* ctx, const uint8_t* ciphertext, 
					int ciphertext_len, uint8_t* plaintext, int* plaintext_len)
{
	int len = 0;
	int total_len = 0;
	uint8_t iv[CRYPTO_IV_SIZE] = {0};

	if (NULL == ctx || NULL == ciphertext || NULL == plaintext || 
		NULL == plaintext_len || CRYPTO_IV_SIZE > ciphertext_len)
	{
		return ERROR_CODE;
	}

	memcpy(iv, ciphertext, CRYPTO_IV_SIZE);

	if (OPENSSL_SUCCESS != EVP_DecryptInit_ex(ctx->evp_dec_ctx, EVP_aes_256_cbc(), 
			NULL, ctx->key, iv))
	{
		return ERROR_CODE;
	}

	if (OPENSSL_SUCCESS != EVP_DecryptUpdate(ctx->evp_dec_ctx, plaintext, &len, 
			ciphertext + CRYPTO_IV_SIZE, ciphertext_len - CRYPTO_IV_SIZE))
	{
		return ERROR_CODE;
	}
	
	total_len = len;

	if (OPENSSL_SUCCESS != EVP_DecryptFinal_ex(ctx->evp_dec_ctx, plaintext + len, &len))
	{
		return ERROR_CODE;
	}
	
	total_len += len;
	*plaintext_len = total_len;

	return SUCCESS_CODE;
}
