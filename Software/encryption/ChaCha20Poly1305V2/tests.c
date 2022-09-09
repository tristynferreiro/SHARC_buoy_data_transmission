/*********************************************************************
* Copyright (c) 2016 Jonas Schnelli                                  *
* Distributed under the MIT software license, see the accompanying   *
* file COPYING or http://www.opensource.org/licenses/mit-license.php.*
**********************************************************************/

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "chacha.h"
#include "chachapoly_aead.h"
#include "poly1305.h"

struct chacha20_testvector {
    uint8_t key[32];
    uint8_t nonce[8];
    uint8_t resulting_keystream[512];
    int keystream_check_size;
};

struct poly1305_testvector {
    uint8_t input[64];
    int inputlen;
    uint8_t key[64];
    uint8_t resulting_tag[16];
};

/*
   Testvectors have been taken from the draft RFC
   https://tools.ietf.org/html/draft-agl-tls-chacha20poly1305-04#section-7
*/

static const struct chacha20_testvector chacha20_testvectors[] = {
    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x76, 0xb8, 0xe0, 0xad, 0xa0, 0xf1, 0x3d, 0x90, 0x40, 0x5d, 0x6a,
            0xe5, 0x53, 0x86, 0xbd, 0x28, 0xbd, 0xd2, 0x19, 0xb8, 0xa0, 0x8d,
            0xed, 0x1a, 0xa8, 0x36, 0xef, 0xcc, 0x8b, 0x77, 0x0d, 0xc7, 0xda,
            0x41, 0x59, 0x7c, 0x51, 0x57, 0x48, 0x8d, 0x77, 0x24, 0xe0, 0x3f,
            0xb8, 0xd8, 0x4a, 0x37, 0x6a, 0x43, 0xb8, 0xf4, 0x15, 0x18, 0xa1,
            0x1c, 0xc3, 0x87, 0xb6, 0x69, 0xb2, 0xee, 0x65, 0x86},
        64},
    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x45, 0x40, 0xf0, 0x5a, 0x9f, 0x1f, 0xb2, 0x96, 0xd7, 0x73, 0x6e,
            0x7b, 0x20, 0x8e, 0x3c, 0x96, 0xeb, 0x4f, 0xe1, 0x83, 0x46, 0x88,
            0xd2, 0x60, 0x4f, 0x45, 0x09, 0x52, 0xed, 0x43, 0x2d, 0x41, 0xbb,
            0xe2, 0xa0, 0xb6, 0xea, 0x75, 0x66, 0xd2, 0xa5, 0xd1, 0xe7, 0xe2,
            0x0d, 0x42, 0xaf, 0x2c, 0x53, 0xd7, 0x92, 0xb1, 0xc4, 0x3f, 0xea,
            0x81, 0x7e, 0x9a, 0xd2, 0x75, 0xae, 0x54, 0x69, 0x63},
        64},
    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
        {0xde, 0x9c, 0xba, 0x7b, 0xf3, 0xd6, 0x9e, 0xf5, 0xe7, 0x86, 0xdc, 0x63,
            0x97, 0x3f, 0x65, 0x3a, 0x0b, 0x49, 0xe0, 0x15, 0xad, 0xbf, 0xf7, 0x13,
            0x4f, 0xcb, 0x7d, 0xf1, 0x37, 0x82, 0x10, 0x31, 0xe8, 0x5a, 0x05, 0x02,
            0x78, 0xa7, 0x08, 0x45, 0x27, 0x21, 0x4f, 0x73, 0xef, 0xc7, 0xfa, 0x5b,
            0x52, 0x77, 0x06, 0x2e, 0xb7, 0xa0, 0x43, 0x3e, 0x44, 0x5f, 0x41, 0xe3},
        60},
    {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0xef, 0x3f, 0xdf, 0xd6, 0xc6, 0x15, 0x78, 0xfb, 0xf5, 0xcf, 0x35,
            0xbd, 0x3d, 0xd3, 0x3b, 0x80, 0x09, 0x63, 0x16, 0x34, 0xd2, 0x1e,
            0x42, 0xac, 0x33, 0x96, 0x0b, 0xd1, 0x38, 0xe5, 0x0d, 0x32, 0x11,
            0x1e, 0x4c, 0xaf, 0x23, 0x7e, 0xe5, 0x3c, 0xa8, 0xad, 0x64, 0x26,
            0x19, 0x4a, 0x88, 0x54, 0x5d, 0xdc, 0x49, 0x7a, 0x0b, 0x46, 0x6e,
            0x7d, 0x6b, 0xbd, 0xb0, 0x04, 0x1b, 0x2f, 0x58, 0x6b},
        64},
    {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
         0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
         0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f},
        {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07},
        {0xf7, 0x98, 0xa1, 0x89, 0xf1, 0x95, 0xe6, 0x69, 0x82, 0x10, 0x5f, 0xfb,
            0x64, 0x0b, 0xb7, 0x75, 0x7f, 0x57, 0x9d, 0xa3, 0x16, 0x02, 0xfc, 0x93,
            0xec, 0x01, 0xac, 0x56, 0xf8, 0x5a, 0xc3, 0xc1, 0x34, 0xa4, 0x54, 0x7b,
            0x73, 0x3b, 0x46, 0x41, 0x30, 0x42, 0xc9, 0x44, 0x00, 0x49, 0x17, 0x69,
            0x05, 0xd3, 0xbe, 0x59, 0xea, 0x1c, 0x53, 0xf1, 0x59, 0x16, 0x15, 0x5c,
            0x2b, 0xe8, 0x24, 0x1a, 0x38, 0x00, 0x8b, 0x9a, 0x26, 0xbc, 0x35, 0x94,
            0x1e, 0x24, 0x44, 0x17, 0x7c, 0x8a, 0xde, 0x66, 0x89, 0xde, 0x95, 0x26,
            0x49, 0x86, 0xd9, 0x58, 0x89, 0xfb, 0x60, 0xe8, 0x46, 0x29, 0xc9, 0xbd,
            0x9a, 0x5a, 0xcb, 0x1c, 0xc1, 0x18, 0xbe, 0x56, 0x3e, 0xb9, 0xb3, 0xa4,
            0xa4, 0x72, 0xf8, 0x2e, 0x09, 0xa7, 0xe7, 0x78, 0x49, 0x2b, 0x56, 0x2e,
            0xf7, 0x13, 0x0e, 0x88, 0xdf, 0xe0, 0x31, 0xc7, 0x9d, 0xb9, 0xd4, 0xf7,
            0xc7, 0xa8, 0x99, 0x15, 0x1b, 0x9a, 0x47, 0x50, 0x32, 0xb6, 0x3f, 0xc3,
            0x85, 0x24, 0x5f, 0xe0, 0x54, 0xe3, 0xdd, 0x5a, 0x97, 0xa5, 0xf5, 0x76,
            0xfe, 0x06, 0x40, 0x25, 0xd3, 0xce, 0x04, 0x2c, 0x56, 0x6a, 0xb2, 0xc5,
            0x07, 0xb1, 0x38, 0xdb, 0x85, 0x3e, 0x3d, 0x69, 0x59, 0x66, 0x09, 0x96,
            0x54, 0x6c, 0xc9, 0xc4, 0xa6, 0xea, 0xfd, 0xc7, 0x77, 0xc0, 0x40, 0xd7,
            0x0e, 0xaf, 0x46, 0xf7, 0x6d, 0xad, 0x39, 0x79, 0xe5, 0xc5, 0x36, 0x0c,
            0x33, 0x17, 0x16, 0x6a, 0x1c, 0x89, 0x4c, 0x94, 0xa3, 0x71, 0x87, 0x6a,
            0x94, 0xdf, 0x76, 0x28, 0xfe, 0x4e, 0xaa, 0xf2, 0xcc, 0xb2, 0x7d, 0x5a,
            0xaa, 0xe0, 0xad, 0x7a, 0xd0, 0xf9, 0xd4, 0xb6, 0xad, 0x3b, 0x54, 0x09,
            0x87, 0x46, 0xd4, 0x52, 0x4d, 0x38, 0x40, 0x7a, 0x6d, 0xeb, 0x3a, 0xb7,
            0x8f, 0xab, 0x78, 0xc9},
        256}};

static const struct poly1305_testvector poly1305_testvectors[] = {
    {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        32,
        {0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x33, 0x32, 0x2d,
            0x62, 0x79, 0x74, 0x65, 0x20, 0x6b, 0x65, 0x79, 0x20, 0x66, 0x6f,
            0x72, 0x20, 0x50, 0x6f, 0x6c, 0x79, 0x31, 0x33, 0x30, 0x35},
        {0x49, 0xec, 0x78, 0x09, 0x0e, 0x48, 0x1e, 0xc6, 0xc2, 0x6b, 0x33, 0xb9,
            0x1c, 0xcc, 0x03, 0x07},
    },
    {{0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21},
        12,
        {0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x33, 0x32, 0x2d,
            0x62, 0x79, 0x74, 0x65, 0x20, 0x6b, 0x65, 0x79, 0x20, 0x66, 0x6f,
            0x72, 0x20, 0x50, 0x6f, 0x6c, 0x79, 0x31, 0x33, 0x30, 0x35},
        {0xa6, 0xf7, 0x45, 0x00, 0x8f, 0x81, 0xc9, 0x16, 0xa2, 0x0d, 0xcc, 0x74,
            0xee, 0xf2, 0xb2, 0xf0}}};

int main(void)
{
    struct chacha_ctx ctx;
    uint8_t iv[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int i = 0;
    uint8_t keystream[512];
    uint8_t poly1305_tag[16];

    /* test chacha20 */
    for (i = 0;
         i < (sizeof(chacha20_testvectors) / sizeof(chacha20_testvectors[0]));
         i++) {
        chacha_ivsetup(&ctx, chacha20_testvectors[i].nonce, NULL);
        memset(keystream, 0, 512);
        chacha_keysetup(&ctx, chacha20_testvectors[i].key, 256);
        chacha_encrypt_bytes(&ctx, keystream, keystream, 512);
        assert(memcmp(keystream, chacha20_testvectors[i].resulting_keystream,
                   chacha20_testvectors[i].keystream_check_size) == 0);
    }


    /* test poly1305 */
    for (i = 0;
         i < (sizeof(poly1305_testvectors) / sizeof(poly1305_testvectors[0]));
         i++) {
        memset(poly1305_tag, 0, 16);
        poly1305_auth(poly1305_tag, poly1305_testvectors[i].input,
            poly1305_testvectors[i].inputlen,
            poly1305_testvectors[i].key);
        assert(memcmp(poly1305_tag, poly1305_testvectors[i].resulting_tag, 16) ==
               0);
        int i = 100;
    }

    /* test chacha20poly1305 AEAD */
    struct chachapolyaead_ctx aead_ctx;
    uint32_t seqnr = 0;
    uint32_t seqnr_aad = 0;
    int pos_aad = 0;
    uint8_t aead_k_1[64] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
        0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
    uint8_t aead_k_2[64] = {
        0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
        0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};

    uint8_t plaintext_buf[256] = {
        0xff, 0x00, 0x00, 0xf1, 0x95, 0xe6, 0x69, 0x82, 0x10, 0x5f, 0xfb,
        0x64, 0x0b, 0xb7, 0x75, 0x7f, 0x57, 0x9d, 0xa3, 0x16, 0x02, 0xfc, 0x93,
        0xec, 0x01, 0xac, 0x56, 0xf8, 0x5a, 0xc3, 0xc1, 0x34, 0xa4, 0x54, 0x7b,
        0x73, 0x3b, 0x46, 0x41, 0x30, 0x42, 0xc9, 0x44, 0x00, 0x49, 0x17, 0x69,
        0x05, 0xd3, 0xbe, 0x59, 0xea, 0x1c, 0x53, 0xf1, 0x59, 0x16, 0x15, 0x5c,
        0x2b, 0xe8, 0x24, 0x1a, 0x38, 0x00, 0x8b, 0x9a, 0x26, 0xbc, 0x35, 0x94,
        0x1e, 0x24, 0x44, 0x17, 0x7c, 0x8a, 0xde, 0x66, 0x89, 0xde, 0x95, 0x26,
        0x49, 0x86, 0xd9, 0x58, 0x89, 0xfb, 0x60, 0xe8, 0x46, 0x29, 0xc9, 0xbd,
        0x9a, 0x5a, 0xcb, 0x1c, 0xc1, 0x18, 0xbe, 0x56, 0x3e, 0xb9, 0xb3, 0xa4,
        0xa4, 0x72, 0xf8, 0x2e, 0x09, 0xa7, 0xe7, 0x78, 0x49, 0x2b, 0x56, 0x2e,
        0xf7, 0x13, 0x0e, 0x88, 0xdf, 0xe0, 0x31, 0xc7, 0x9d, 0xb9, 0xd4, 0xf7,
        0xc7, 0xa8, 0x99, 0x15, 0x1b, 0x9a, 0x47, 0x50, 0x32, 0xb6, 0x3f, 0xc3,
        0x85, 0x24, 0x5f, 0xe0, 0x54, 0xe3, 0xdd, 0x5a, 0x97, 0xa5, 0xf5, 0x76,
        0xfe, 0x06, 0x40, 0x25, 0xd3, 0xce, 0x04, 0x2c, 0x56, 0x6a, 0xb2, 0xc5,
        0x07, 0xb1, 0x38, 0xdb, 0x85, 0x3e, 0x3d, 0x69, 0x59, 0x66, 0x09, 0x96,
        0x54, 0x6c, 0xc9, 0xc4, 0xa6, 0xea, 0xfd, 0xc7, 0x77, 0xc0, 0x40, 0xd7,
        0x0e, 0xaf, 0x46, 0xf7, 0x6d, 0xad, 0x39, 0x79, 0xe5, 0xc5, 0x36, 0x0c,
        0x33, 0x17, 0x16, 0x6a, 0x1c, 0x89, 0x4c, 0x94, 0xa3, 0x71, 0x87, 0x6a,
        0x94, 0xdf, 0x76, 0x28, 0xfe, 0x4e, 0xaa, 0xf2, 0xcc, 0xb2, 0x7d, 0x5a,
        0xaa, 0xe0, 0xad, 0x7a, 0xd0, 0xf9, 0xd4, 0xb6, 0xad, 0x3b, 0x54, 0x09,
        0x87, 0x46, 0xd4, 0x52, 0x4d, 0x38, 0x40, 0x7a, 0x6d, 0xeb, 0x3a, 0xb7,
        0x8f, 0xab, 0x78, 0xc9};

    uint8_t ciphertext_buf[255 + 16] = {0};
    uint8_t plaintext_buf_new[255] = {0};
    chacha20poly1305_init(&aead_ctx, aead_k_1, 32, aead_k_2, 32);
    assert((uint32_t)plaintext_buf[0] == 255);
    chacha20poly1305_crypt(&aead_ctx, seqnr, seqnr_aad, pos_aad, ciphertext_buf, 300, plaintext_buf, 255, 1);
    uint32_t out_len = 0;
    chacha20poly1305_get_length(&aead_ctx, &out_len, seqnr, ciphertext_buf);
    assert(out_len == 255);
    chacha20poly1305_crypt(&aead_ctx, seqnr, seqnr_aad, pos_aad, plaintext_buf_new, 255, ciphertext_buf,
        sizeof(ciphertext_buf), 0);
    assert(memcmp(plaintext_buf, plaintext_buf_new, 252) == 0);

}
