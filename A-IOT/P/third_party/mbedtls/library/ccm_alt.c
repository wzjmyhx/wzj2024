/*
 *  NIST SP800-38C compliant CCM implementation
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

/*
 * Definition of CCM:
 * http://csrc.nist.gov/publications/nistpubs/800-38C/SP800-38C_updated-July20_2007.pdf
 * RFC 3610 "Counter with CBC-MAC (CCM)"
 *
 * Related:
 * RFC 5116 "An Interface and Algorithms for Authenticated Encryption"
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_CCM_C)

#include "mbedtls/ccm.h"
#include "mbedtls/platform_util.h"

#include <string.h>

#if defined(MBEDTLS_AES_C)
#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif /* MBEDTLS_PLATFORM_C */
#endif /* MBEDTLS_AES_C */

#if defined(MBEDTLS_CCM_ALT)

#define CCM_VALIDATE_RET( cond ) \
    MBEDTLS_INTERNAL_VALIDATE_RET( cond, MBEDTLS_ERR_CCM_BAD_INPUT )
#define CCM_VALIDATE( cond ) \
    MBEDTLS_INTERNAL_VALIDATE( cond )

#define CCM_ENCRYPT 0
#define CCM_DECRYPT 1
#define CCM_ALT_ALIGNSIZE 4
#define CCM_HW_SUPPORT_KEY_LEN 128
/*
 * Initialize context
 */
void mbedtls_ccm_init( mbedtls_ccm_context *ctx )
{
    CCM_VALIDATE( ctx != NULL );
    memset( ctx, 0, sizeof( mbedtls_ccm_context ) );
}

int mbedtls_ccm_setkey( mbedtls_ccm_context *ctx,
                        mbedtls_cipher_id_t cipher,
                        const unsigned char *key,
                        unsigned int keybits )
{
    int ret;
    const mbedtls_cipher_info_t *cipher_info;

    CCM_VALIDATE_RET( ctx != NULL );
    CCM_VALIDATE_RET( key != NULL );

    cipher_info = mbedtls_cipher_info_from_values( cipher, keybits, MBEDTLS_MODE_ECB );
    if( cipher_info == NULL )
        return( MBEDTLS_ERR_CCM_BAD_INPUT );

    if( cipher_info->block_size != 16 )
        return( MBEDTLS_ERR_CCM_BAD_INPUT );

    mbedtls_cipher_free( &ctx->cipher_ctx );

    if( ( ret = mbedtls_cipher_setup( &ctx->cipher_ctx, cipher_info ) ) != 0 )
        return( ret );

    if( ( ret = mbedtls_cipher_setkey( &ctx->cipher_ctx, key, keybits,
                               MBEDTLS_ENCRYPT ) ) != 0 )
    {
        return( ret );
    }

    return( 0 );
}

/*
 * Free context
 */
void mbedtls_ccm_free( mbedtls_ccm_context *ctx )
{
    if( ctx == NULL )
        return;
    mbedtls_cipher_free( &ctx->cipher_ctx );
    mbedtls_platform_zeroize( ctx, sizeof( mbedtls_ccm_context ) );
}

/*
 * Macros for common operations.
 * Results in smaller compiled code than static inline functions.
 */

/*
 * Update the CBC-MAC state in y using a block in b
 * (Always using b as the source helps the compiler optimise a bit better.)
 */
#define UPDATE_CBC_MAC                                                      \
    for( i = 0; i < 16; i++ )                                               \
        y[i] ^= b[i];                                                       \
                                                                            \
    if( ( ret = mbedtls_cipher_update( &ctx->cipher_ctx, y, 16, y, &olen ) ) != 0 ) \
        return( ret );

/*
 * Encrypt or decrypt a partial block with CTR
 * Warning: using b for temporary storage! src and dst must not be b!
 * This avoids allocating one more 16 bytes buffer while allowing src == dst.
 */
#define CTR_CRYPT( dst, src, len  )                                            \
    do                                                                  \
    {                                                                   \
        if( ( ret = mbedtls_cipher_update( &ctx->cipher_ctx, ctr,       \
                                           16, b, &olen ) ) != 0 )      \
        {                                                               \
            return( ret );                                              \
        }                                                               \
                                                                        \
        for( i = 0; i < (len); i++ )                                    \
            (dst)[i] = (src)[i] ^ b[i];                                 \
    } while( 0 )

/*
 * Authenticated encryption or decryption
 */
static int ccm_auth_crypt( mbedtls_ccm_context *ctx, int mode, size_t length,
                           const unsigned char *iv, size_t iv_len,
                           const unsigned char *add, size_t add_len,
                           const unsigned char *input, unsigned char *output,
                           unsigned char *tag, size_t tag_len )
{
    int ret;
    unsigned char i;
    unsigned char q;
    size_t len_left, olen;
    unsigned char b[16];
    unsigned char y[16];
    unsigned char ctr[16];
    const unsigned char *src;
    unsigned char *dst;

    /*
     * Check length requirements: SP800-38C A.1
     * Additional requirement: a < 2^16 - 2^8 to simplify the code.
     * 'length' checked later (when writing it to the first block)
     *
     * Also, loosen the requirements to enable support for CCM* (IEEE 802.15.4).
     */
    if( tag_len == 2 || tag_len > 16 || tag_len % 2 != 0 )
        return( MBEDTLS_ERR_CCM_BAD_INPUT );

    /* Also implies q is within bounds */
    if( iv_len < 7 || iv_len > 13 )
        return( MBEDTLS_ERR_CCM_BAD_INPUT );

    if( add_len > 0xFF00 )
        return( MBEDTLS_ERR_CCM_BAD_INPUT );

    q = 16 - 1 - (unsigned char) iv_len;

    /*
     * First block B_0:
     * 0        .. 0        flags
     * 1        .. iv_len   nonce (aka iv)
     * iv_len+1 .. 15       length
     *
     * With flags as (bits):
     * 7        0
     * 6        add present?
     * 5 .. 3   (t - 2) / 2
     * 2 .. 0   q - 1
     */
    b[0] = 0;
    b[0] |= ( add_len > 0 ) << 6;
    b[0] |= ( ( tag_len - 2 ) / 2 ) << 3;
    b[0] |= q - 1;

    memcpy( b + 1, iv, iv_len );

    for( i = 0, len_left = length; i < q; i++, len_left >>= 8 )
        b[15-i] = (unsigned char)( len_left & 0xFF );

    if( len_left > 0 )
        return( MBEDTLS_ERR_CCM_BAD_INPUT );


    /* Start CBC-MAC with first block */
    memset( y, 0, 16 );
    UPDATE_CBC_MAC;

    /*
     * If there is additional data, update CBC-MAC with
     * add_len, add, 0 (padding to a block boundary)
     */
    if( add_len > 0 )
    {
        size_t use_len;
        len_left = add_len;
        src = add;

        memset( b, 0, 16 );
        b[0] = (unsigned char)( ( add_len >> 8 ) & 0xFF );
        b[1] = (unsigned char)( ( add_len      ) & 0xFF );

        use_len = len_left < 16 - 2 ? len_left : 16 - 2;
        memcpy( b + 2, src, use_len );
        len_left -= use_len;
        src += use_len;

        UPDATE_CBC_MAC;

        while( len_left > 0 )
        {
            use_len = len_left > 16 ? 16 : len_left;

            memset( b, 0, 16 );
            memcpy( b, src, use_len );
            UPDATE_CBC_MAC;

            len_left -= use_len;
            src += use_len;
        }
    }

    /*
     * Prepare counter block for encryption:
     * 0        .. 0        flags
     * 1        .. iv_len   nonce (aka iv)
     * iv_len+1 .. 15       counter (initially 1)
     *
     * With flags as (bits):
     * 7 .. 3   0
     * 2 .. 0   q - 1
     */
    ctr[0] = q - 1;
    memcpy( ctr + 1, iv, iv_len );
    memset( ctr + 1 + iv_len, 0, q );
    ctr[15] = 1;

    /*
     * Authenticate and {en,de}crypt the message.
     *
     * The only difference between encryption and decryption is
     * the respective order of authentication and {en,de}cryption.
     */
    len_left = length;
    src = input;
    dst = output;

    while( len_left > 0 )
    {
        size_t use_len = len_left > 16 ? 16 : len_left;

        if( mode == CCM_ENCRYPT )
        {
            memset( b, 0, 16 );
            memcpy( b, src, use_len );
            UPDATE_CBC_MAC;
        }

        CTR_CRYPT( dst, src, use_len );

        if( mode == CCM_DECRYPT )
        {
            memset( b, 0, 16 );
            memcpy( b, dst, use_len );
            UPDATE_CBC_MAC;
        }

        dst += use_len;
        src += use_len;
        len_left -= use_len;

        /*
         * Increment counter.
         * No need to check for overflow thanks to the length check above.
         */
        for( i = 0; i < q; i++ )
            if( ++ctr[15-i] != 0 )
                break;
    }

    /*
     * Authentication: reset counter and crypt/mask internal tag
     */
    for( i = 0; i < q; i++ )
        ctr[15-i] = 0;

    CTR_CRYPT( y, y, 16 );
    memcpy( tag, y, tag_len );

    return( 0 );
}

/*
 * Authenticated encryption
 */

static hi_s32 set_aes_config(hi_u8 *key, hi_u32 klen,
                            hi_u8 *iv, hi_u32 ivlen,
                            hi_bool random_en,
                            hi_cipher_aes_key_from key_from,
                            hi_cipher_aes_work_mode work_mode,
                            hi_cipher_aes_key_length key_len,
                            hi_cipher_aes_ccm *ccm,
                            hi_cipher_aes_ctrl *aes_ctrl)
{
    hi_s32 ret;

    ret = memcpy_s( aes_ctrl->key, sizeof(aes_ctrl->key), key, klen );
    if (ret != 0)
        return ret;

    ret = memcpy_s( aes_ctrl->iv, sizeof(aes_ctrl->iv), iv, ivlen );
    if (ret != 0)
        return ret;

    aes_ctrl->random_en = random_en;
    aes_ctrl->key_from  = key_from;
    aes_ctrl->work_mode = work_mode;
    aes_ctrl->key_len = key_len;
    aes_ctrl->ccm = ccm;

    return 0;
}

static int mbedtls_ccm_buf_align(const unsigned char *input,
                                    const unsigned char *output,
                                    unsigned char **p_temp_input,
                                    unsigned char **p_temp_output,
                                    size_t length)
{
    if( (uintptr_t)input % CCM_ALT_ALIGNSIZE )
    {
        *p_temp_input = (unsigned char *)mbedtls_calloc( length, 1 );
        if ( *p_temp_input == NULL )
            return -1;

        memcpy( *p_temp_input, input, length );
    }
    else
        *p_temp_input = (unsigned char *)input;

    if( (uintptr_t)output % CCM_ALT_ALIGNSIZE )
    {
        *p_temp_output = (unsigned char *)mbedtls_calloc( length, 1 );
        if ( *p_temp_output == NULL )
            return -1;
    }
    else
        *p_temp_output = (unsigned char *)output;

    return 0;
}

static int ccm_auth_crypt_hw( mbedtls_ccm_context *ctx, int mode, size_t length,
                           const unsigned char *iv, size_t iv_len,
                           const unsigned char *add, size_t add_len,
                           const unsigned char *input, unsigned char *output,
                           unsigned char *tag, size_t tag_len )
{
    int ret;
    hi_u32 enc_tag_out_len = 0;
    hi_cipher_aes_ccm ccm;
    hi_cipher_aes_ctrl aes_ctrl;
    unsigned char *temp_input = NULL;
    unsigned char *temp_output = NULL;

    mbedtls_platform_zeroize( &aes_ctrl, sizeof(aes_ctrl) );
    mbedtls_platform_zeroize( &ccm, sizeof(ccm) );

    ccm.aad_addr = (uintptr_t)add;
    ccm.aad_len = (hi_u32)add_len;
    ccm.n = (hi_u8 *)iv;
    ccm.n_len = (hi_u32)iv_len;
    ccm.tag_len = (hi_u32)tag_len;
    mbedtls_aes_context *aes_ctx = (mbedtls_aes_context *)ctx->cipher_ctx.cipher_ctx;
    unsigned char *aes_key = NULL;

#if defined(MBEDTLS_AES_ALT)
    aes_key = (unsigned char *)aes_ctx->key;
#else
    aes_key = (unsigned char *)aes_ctx->rk;
#endif
    /* config ctrl */
    ret = set_aes_config( aes_key, 16,
                        (hi_u8 *)iv, sizeof(iv),
                        HI_FALSE,
                        HI_CIPHER_AES_KEY_FROM_CPU,
                        HI_CIPHER_AES_WORK_MODE_CCM,
                        HI_CIPHER_AES_KEY_LENGTH_128BIT,
                        &ccm, &aes_ctrl );
    if( ret != 0 )
        return MBEDTLS_ERR_CCM_HW_ACCEL_FAILED;

    ret = hi_cipher_aes_config( &aes_ctrl );
    if( ret != 0 )
        return MBEDTLS_ERR_CCM_HW_ACCEL_FAILED;

    ret = mbedtls_ccm_buf_align( input, output, &temp_input, &temp_output, length );
    if( ret != 0 )
    {
        ret = MBEDTLS_ERR_CCM_HW_ACCEL_FAILED;
        goto exit;
    }

    ret = hi_cipher_aes_crypto( (uintptr_t)temp_input, (uintptr_t)temp_output, length, (mode == CCM_ENCRYPT) );
    if( ret == 0 )
    {
        if( temp_output != output )
            memcpy( output, temp_output, length );
    }
    else
    {
        ret = MBEDTLS_ERR_CCM_HW_ACCEL_FAILED;
        goto exit;
    }

    ret = hi_cipher_aes_get_tag( tag, tag_len, &enc_tag_out_len );
    if( ret != 0)
        ret = MBEDTLS_ERR_CCM_HW_ACCEL_FAILED;

exit:
    hi_cipher_aes_destroy_config();

    if( (temp_input != NULL) && (temp_input != input) )
        mbedtls_free(temp_input);

    if( (temp_output != NULL) && (temp_output != output) )
        mbedtls_free(temp_output);

    return ret;

}
int mbedtls_ccm_star_encrypt_and_tag( mbedtls_ccm_context *ctx, size_t length,
                         const unsigned char *iv, size_t iv_len,
                         const unsigned char *add, size_t add_len,
                         const unsigned char *input, unsigned char *output,
                         unsigned char *tag, size_t tag_len )
{
    const mbedtls_cipher_info_t *cipher_info = ctx->cipher_ctx.cipher_info;

    CCM_VALIDATE_RET( ctx != NULL );
    CCM_VALIDATE_RET( iv != NULL );
    CCM_VALIDATE_RET( add_len == 0 || add != NULL );
    CCM_VALIDATE_RET( length == 0 || input != NULL );
    CCM_VALIDATE_RET( length == 0 || output != NULL );
    CCM_VALIDATE_RET( tag_len == 0 || tag != NULL );

    if( cipher_info->key_bitlen == CCM_HW_SUPPORT_KEY_LEN && cipher_info->base->cipher == MBEDTLS_CIPHER_ID_AES )
        return( ccm_auth_crypt_hw( ctx, CCM_ENCRYPT, length, iv, iv_len,
                                add, add_len, input, output, tag, tag_len ) );

    return( ccm_auth_crypt( ctx, CCM_ENCRYPT, length, iv, iv_len,
                            add, add_len, input, output, tag, tag_len ) );
}

int mbedtls_ccm_encrypt_and_tag( mbedtls_ccm_context *ctx, size_t length,
                         const unsigned char *iv, size_t iv_len,
                         const unsigned char *add, size_t add_len,
                         const unsigned char *input, unsigned char *output,
                         unsigned char *tag, size_t tag_len )
{
    CCM_VALIDATE_RET( ctx != NULL );
    CCM_VALIDATE_RET( iv != NULL );
    CCM_VALIDATE_RET( add_len == 0 || add != NULL );
    CCM_VALIDATE_RET( length == 0 || input != NULL );
    CCM_VALIDATE_RET( length == 0 || output != NULL );
    CCM_VALIDATE_RET( tag_len == 0 || tag != NULL );
    if( tag_len == 0 )
        return( MBEDTLS_ERR_CCM_BAD_INPUT );

    return( mbedtls_ccm_star_encrypt_and_tag( ctx, length, iv, iv_len, add,
                add_len, input, output, tag, tag_len ) );
}

/*
 * Authenticated decryption
 */
int mbedtls_ccm_star_auth_decrypt( mbedtls_ccm_context *ctx, size_t length,
                      const unsigned char *iv, size_t iv_len,
                      const unsigned char *add, size_t add_len,
                      const unsigned char *input, unsigned char *output,
                      const unsigned char *tag, size_t tag_len )
{
    int ret;
    unsigned char check_tag[16];
    unsigned char i;
    int diff;
    const mbedtls_cipher_info_t *cipher_info = ctx->cipher_ctx.cipher_info;

    CCM_VALIDATE_RET( ctx != NULL );
    CCM_VALIDATE_RET( iv != NULL );
    CCM_VALIDATE_RET( add_len == 0 || add != NULL );
    CCM_VALIDATE_RET( length == 0 || input != NULL );
    CCM_VALIDATE_RET( length == 0 || output != NULL );
    CCM_VALIDATE_RET( tag_len == 0 || tag != NULL );

    if( cipher_info->key_bitlen == CCM_HW_SUPPORT_KEY_LEN && cipher_info->base->cipher == MBEDTLS_CIPHER_ID_AES )
    {
        if( ( ret = ccm_auth_crypt_hw( ctx, CCM_DECRYPT, length,
                                    iv, iv_len, add, add_len,
                                    input, output, check_tag, tag_len ) ) != 0 )
        {
            return( ret );
        }
    }
    else
    {
        if( ( ret = ccm_auth_crypt( ctx, CCM_DECRYPT, length,
                                    iv, iv_len, add, add_len,
                                    input, output, check_tag, tag_len ) ) != 0 )
        {
            return( ret );
        }
    }

    /* Check tag in "constant-time" */
    for( diff = 0, i = 0; i < tag_len; i++ )
        diff |= tag[i] ^ check_tag[i];

    if( diff != 0 )
    {
        mbedtls_platform_zeroize( output, length );
        return( MBEDTLS_ERR_CCM_AUTH_FAILED );
    }

    return( 0 );
}

int mbedtls_ccm_auth_decrypt( mbedtls_ccm_context *ctx, size_t length,
                      const unsigned char *iv, size_t iv_len,
                      const unsigned char *add, size_t add_len,
                      const unsigned char *input, unsigned char *output,
                      const unsigned char *tag, size_t tag_len )
{
    CCM_VALIDATE_RET( ctx != NULL );
    CCM_VALIDATE_RET( iv != NULL );
    CCM_VALIDATE_RET( add_len == 0 || add != NULL );
    CCM_VALIDATE_RET( length == 0 || input != NULL );
    CCM_VALIDATE_RET( length == 0 || output != NULL );
    CCM_VALIDATE_RET( tag_len == 0 || tag != NULL );

    if( tag_len == 0 )
        return( MBEDTLS_ERR_CCM_BAD_INPUT );

    return( mbedtls_ccm_star_auth_decrypt( ctx, length, iv, iv_len, add,
                add_len, input, output, tag, tag_len ) );
}
#endif /* !MBEDTLS_CCM_ALT */

#endif /* MBEDTLS_CCM_C */