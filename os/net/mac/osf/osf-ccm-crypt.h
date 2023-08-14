/*
 * Copyright (c) 2022, Technology Innovation Institute
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OSF_CCM_CRYPT_H_
#define OSF_CCM_CRYPT_H_

/* Fixed parameters */
#define CCM_KEY_SIZE        16
#define CCM_IV_SIZE         8
#define CCM_COUNTER_SIZE    5
#define CCM_MIC_SIZE        4

#define AES_ECB_KEY_SIZE    16
#define AES_ECB_BLOCK_SIZE  16

enum {CCM_CRYPT_SUCCESS = 0, CCM_CRYPT_INVALID_MIC, CCM_CRYPT_RUNTIME_ERROR};

/* Configurable parameters */
#define DATA_LENGTH (251)
#define SCRATCH_AREA_SIZE (DATA_LENGTH + 16)

typedef struct {
  uint8_t key[CCM_KEY_SIZE];
  uint64_t counter;
  uint8_t direction;
  uint8_t iv[CCM_IV_SIZE];
}ccm_data_t;

typedef struct {
  uint8_t aes_key[AES_ECB_KEY_SIZE];
  uint8_t aes_input[AES_ECB_BLOCK_SIZE];
  uint8_t aes_output[AES_ECB_BLOCK_SIZE];
}aes_ecb_data_t;

/* Functions */
void osf_rng_fill_buffer(uint8_t *buf, uint32_t bufsize);
void osf_srand_init();
void osf_srand_fill_buffer(uint8_t *buf, uint32_t bufsize);

void osf_ccm_init(nrf_radio_mode_t  mode);
void osf_ccm_disable();
void osf_ccm_iv_set(uint8_t *iv, uint8_t iv_size);
void osf_ccm_counter_set(uint64_t counter);
uint8_t osf_ccm_packet_encrypt_async(uint8_t *in_packet, uint8_t *out_packet);
uint8_t osf_ccm_packet_decrypt_async(uint8_t *in_packet, uint8_t *out_packet);
uint8_t osf_ccm_packet_encrypt(uint8_t *in_packet, uint8_t *out_packet);
uint8_t osf_ccm_packet_decrypt(uint8_t *in_packet, uint8_t *out_packet);
uint8_t osf_ccm_status_get();
uint8_t osf_aes_ecb_encrypt(uint8_t *in_buf, uint8_t *out_buf, uint8_t buf_size);

#endif /* OSF_CCM_CRYPT_H_ */