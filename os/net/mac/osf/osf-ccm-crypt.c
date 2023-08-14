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

#include "contiki.h"
#include <nrf_ccm.h>
#include <nrf_ppi.h>
#include <nrf_rng.h>
#include "osf.h"
#include "osf-ccm-crypt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t ccm_scratch_area[SCRATCH_AREA_SIZE];
static ccm_data_t m_ccm_config;

static const uint8_t ccm_psk_key[CCM_KEY_SIZE] = \
{ 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };

static aes_ecb_data_t m_aes_ecb_config;
static const uint8_t aes_ecb_key[AES_ECB_KEY_SIZE] = \
{ 0x2c, 0x7f, 0x16, 0x17, 0x29, 0xaf, 0xd3, 0xa7, 0xac, 0xf8, 0x16, 0x89, 0x0A, 0xdf, 0x5f, 0x4c };

static uint32_t CCM_MODE_DATARATE_E = 0UL;
static uint32_t CCM_MODE_DATARATE_D = 0UL;
static uint32_t CCM_MODE_DATARATE_SYNC_E = 0UL;
static uint32_t CCM_MODE_DATARATE_SYNC_D = 0UL;

/*---------------------------------------------------------------------------*/
static void
osf_ccm_datarate_set(nrf_radio_mode_t  mode)
{
  switch (mode) {
    case PHY_BLE_2M:
      CCM_MODE_DATARATE_E =  CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_D = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_SYNC_E = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_SYNC_D = CCM_MODE_DATARATE_2Mbit;
      break;
    case PHY_BLE_1M:
      CCM_MODE_DATARATE_E = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_D = CCM_MODE_DATARATE_1Mbit;
      CCM_MODE_DATARATE_SYNC_E = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_SYNC_D = CCM_MODE_DATARATE_2Mbit;
      break;
    case PHY_BLE_500K:
      CCM_MODE_DATARATE_E = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_D = CCM_MODE_DATARATE_500Kbps;
      CCM_MODE_DATARATE_SYNC_E = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_SYNC_D = CCM_MODE_DATARATE_2Mbit;
      break;
    case PHY_BLE_125K:
      CCM_MODE_DATARATE_E = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_D = CCM_MODE_DATARATE_125Kbps;
      CCM_MODE_DATARATE_SYNC_E = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_SYNC_D = CCM_MODE_DATARATE_2Mbit;
      break;
    default:
      CCM_MODE_DATARATE_E = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_D = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_SYNC_E = CCM_MODE_DATARATE_2Mbit;
      CCM_MODE_DATARATE_SYNC_D = CCM_MODE_DATARATE_2Mbit;
      break;
  }
}

/*---------------------------------------------------------------------------*/
void
osf_ccm_init(nrf_radio_mode_t  mode)
{
  memset(&m_ccm_config, 0x00, sizeof(m_ccm_config));
  //memset(&ccm_scratch_area[0], 0x00, sizeof(ccm_scratch_area));
  memcpy(m_ccm_config.key, ccm_psk_key, CCM_KEY_SIZE);
  m_ccm_config.direction = 0;
  m_ccm_config.counter = 0;
  osf_ccm_datarate_set(mode);

  NRF_CCM->ENABLE = CCM_ENABLE_ENABLE_Disabled << CCM_ENABLE_ENABLE_Pos;
  NRF_CCM->ENABLE = CCM_ENABLE_ENABLE_Enabled << CCM_ENABLE_ENABLE_Pos;
  NRF_CCM->SHORTS = 0UL;
  NRF_CCM->INTENSET = 0UL;
  NRF_CCM->INTENCLR = 7UL;
  NRF_CCM->EVENTS_ERROR = 0UL;
  NRF_CCM->MAXPACKETSIZE = 0xFB; /* [0x001B..0x00FB] */
  /*NRF_CCM->MAXPACKETSIZE = (NRF_RADIO->PCNF1 & RADIO_PCNF1_MAXLEN_Msk) >> RADIO_PCNF1_MAXLEN_Pos; */
  NRF_CCM->SCRATCHPTR = (uint32_t)ccm_scratch_area;
  NRF_CCM->CNFPTR = (uint32_t)&m_ccm_config;
}
/*---------------------------------------------------------------------------*/
void
osf_ccm_disable(void)
{
  NRF_PPI->CHEN &= ~(1UL << NRF_PPI_CHANNEL24);
  NRF_PPI->CHEN &= ~(1UL << NRF_PPI_CHANNEL25);
  if(NRF_CCM->ENABLE & CCM_ENABLE_ENABLE_Msk) {
    NRF_CCM->TASKS_STOP = 1UL;
    NRF_CCM->SHORTS = 0UL;
    NRF_CCM->INTENSET = 0UL;
    NRF_CCM->INTENCLR = 7UL;
    NRF_CCM->EVENTS_ENDKSGEN = 0UL;
    NRF_CCM->EVENTS_ENDCRYPT = 0UL;
    NRF_CCM->EVENTS_ERROR = 0UL;
    NRF_CCM->MAXPACKETSIZE = 0UL;
  }
  NRF_CCM->ENABLE = CCM_ENABLE_ENABLE_Disabled << CCM_ENABLE_ENABLE_Pos;
  //memset(&ccm_scratch_area[0], 0x00, sizeof(ccm_scratch_area));
}
/*---------------------------------------------------------------------------*/
void
osf_ccm_iv_set(uint8_t *iv, uint8_t iv_size)
{
  memcpy(m_ccm_config.iv, iv, iv_size);
}
/*---------------------------------------------------------------------------*/
void
osf_ccm_counter_set(uint64_t counter)
{
  m_ccm_config.counter = counter;
}
/*---------------------------------------------------------------------------*/
uint8_t
osf_ccm_status_get(void)
{
  /* Deprecated, but can be used */
  if(NRF_CCM->EVENTS_ERROR) {
    return CCM_CRYPT_RUNTIME_ERROR;
  }

  /* MIC status */
  if(NRF_CCM->MICSTATUS == (CCM_MICSTATUS_MICSTATUS_CheckFailed << CCM_MICSTATUS_MICSTATUS_Pos)) {
    return CCM_CRYPT_INVALID_MIC;
  }

  return CCM_CRYPT_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
osf_ccm_packet_encrypt_async(uint8_t *in_packet, uint8_t *out_packet)
{
  /* The OUTPTR pointer in the AES CCM must point to the same memory location
   * as the PACKETPTR pointer in the radio.
   */
  NRF_CCM->INPTR = (uint32_t)in_packet;
  NRF_CCM->OUTPTR = (uint32_t)out_packet;

  NRF_CCM->MODE = (CCM_MODE_MODE_Encryption << CCM_MODE_MODE_Pos) |
    (CCM_MODE_DATARATE_E << CCM_MODE_DATARATE_Pos) |
    (CCM_MODE_LENGTH_Extended << CCM_MODE_LENGTH_Pos);

  /*NRF_CCM->RATEOVERRIDE = CCM_MODE_DATARATE_E; */

  /* Initiate generation of keystream */
  NRF_CCM->EVENTS_ENDKSGEN = 0UL;
  NRF_CCM->EVENTS_ENDCRYPT = 0UL;
  NRF_CCM->EVENTS_ERROR = 0UL;

  /*
   * If a shortcut is used between the ENDKSGEN event and CRYPT task,
   * pointer INPTR and the pointers OUTPTR must also be configured before the KSGEN task is triggered.
   */

  /* For short packets (MODE.LENGTH = Default), the KSGEN task must be triggered before
   * or at the same time as the START task in RADIO is triggered. In addition,
   * the shortcut between the ENDKSGEN event and the CRYPT task must be enabled.
   * It uses a PPI connection between the READY event in RADIO and the KSGEN task in the AES CCM peripheral.
   *
   * For long packets (MODE.LENGTH = Extended), the keystream generation needs to start earlier,
   * such as when the TXEN task in RADIO is triggered.
   */

  /* Trigger KSGEN task. It can take ~50us.
   * And encrypt data after that. Input data in buffer already.
   */
  NRF_CCM->SHORTS = CCM_SHORTS_ENDKSGEN_CRYPT_Enabled;
  NRF_CCM->TASKS_KSGEN = 1UL;

  return CCM_CRYPT_SUCCESS;
}
/*----------------------------------------------------------------------------*/
uint8_t
osf_ccm_packet_decrypt_async(uint8_t *in_packet, uint8_t *out_packet)
{
  /* When the AES CCM peripheral decrypts a packet on-the-fly while RADIO is receiving it,
   * the AES CCM peripheral must read the encrypted packet from the same memory location that RADIO is writing to.
   * The INPTR pointer in the AES CCM must point to the same memory location as the PACKETPTR pointer in RADIO.
   */
  NRF_CCM->INPTR = (uint32_t)in_packet;
  NRF_CCM->OUTPTR = (uint32_t)out_packet;

  NRF_CCM->MODE = (CCM_MODE_MODE_Decryption << CCM_MODE_MODE_Pos) |
    (CCM_MODE_DATARATE_D << CCM_MODE_DATARATE_Pos) |
    (CCM_MODE_LENGTH_Extended << CCM_MODE_LENGTH_Pos);

  /*NRF_CCM->RATEOVERRIDE = CCM_MODE_DATARATE_D; */

  /* Initiate generation of keystream */
  NRF_CCM->EVENTS_ENDKSGEN = 0UL;
  NRF_CCM->EVENTS_ENDCRYPT = 0UL;
  NRF_CCM->EVENTS_ERROR = 0UL;

  /* In order to match RADIO’s timing, the KSGEN task must be triggered early enough to allow
   * the keystream generation to complete before the decryption of the packet shall start.
   * For short packets (MODE.LENGTH = Default) the KSGEN task must be triggered no later than
   * when the START task in RADIO is triggered. In addition, the CRYPT task must be triggered
   *  no earlier than when the ADDRESS event is generated by RADIO.
   *
   * If the CRYPT task is triggered exactly at the same time as the ADDRESS event is generated by RADIO,
   * the AES CCM peripheral will guarantee that the decryption is completed no later than
   * when the END event in RADIO is generated.
   * This use-case is illustrated in On-the-fly decryption of short packets (MODE.LENGTH = Default)
   * using a PPI connection using a PPI connection between the ADDRESS event in RADIO and the CRYPT task in the AES CCM peripheral.
   *
   * The KSGEN task is triggered from the READY event in RADIO through a PPI connection.
   * or long packets (MODE.LENGTH = Extended) the keystream generation will need to start even earlier,
   * such as when the RXEN task in RADIO is triggered.
   */

  /* Preprogrammed PPI channels
   * 24	RADIO->EVENTS_READY	CCM->TASKS_KSGEN
   * 25	RADIO->EVENTS_ADDRESS	CCM->TASKS_CRYPT
   */

  /* Enable pre-programmed PPI channel RADIO->EVENTS_ADDRESS ->	CCM->TASKS_CRYPT */
  NRF_PPI->CHEN |= (1UL << NRF_PPI_CHANNEL25);

  /* Trigger KSGEN task. It can take ~50us */
  NRF_CCM->TASKS_KSGEN = 1UL;
  /*NRF_CCM->TASKS_RATEOVERRIDE = 1UL; */

  return CCM_CRYPT_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
osf_ccm_packet_encrypt(uint8_t *in_packet, uint8_t *out_packet)
{
  NRF_CCM->INPTR = (uint32_t)in_packet;
  NRF_CCM->OUTPTR = (uint32_t)out_packet;

  NRF_CCM->MODE = (CCM_MODE_MODE_Encryption << CCM_MODE_MODE_Pos) |
    (CCM_MODE_DATARATE_SYNC_E << CCM_MODE_DATARATE_Pos) |
    (CCM_MODE_LENGTH_Extended << CCM_MODE_LENGTH_Pos);

  /* Generate keystream */
  NRF_CCM->EVENTS_ENDKSGEN = 0UL;
  NRF_CCM->EVENTS_ERROR = 0UL;
  NRF_CCM->TASKS_KSGEN = 1UL;
  while(NRF_CCM->EVENTS_ENDKSGEN == 0UL);
  if(NRF_CCM->EVENTS_ERROR) {
    return CCM_CRYPT_RUNTIME_ERROR;
  }

  /* Encrypt the packet */
  NRF_CCM->EVENTS_ENDCRYPT = 0UL;
  NRF_CCM->EVENTS_ERROR = 0UL;
  NRF_CCM->TASKS_CRYPT = 1UL;
  while(NRF_CCM->EVENTS_ENDCRYPT == 0UL);
  if(NRF_CCM->EVENTS_ERROR) {
    return CCM_CRYPT_RUNTIME_ERROR;
  }

  return CCM_CRYPT_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
osf_ccm_packet_decrypt(uint8_t *in_packet, uint8_t *out_packet)
{
  NRF_CCM->INPTR = (uint32_t)in_packet;
  NRF_CCM->OUTPTR = (uint32_t)out_packet;

  NRF_CCM->MODE = (CCM_MODE_MODE_Decryption << CCM_MODE_MODE_Pos) |
    (CCM_MODE_DATARATE_SYNC_D << CCM_MODE_DATARATE_Pos) |
    (CCM_MODE_LENGTH_Extended << CCM_MODE_LENGTH_Pos);

  /* Generate keystream */
  NRF_CCM->EVENTS_ENDKSGEN = 0UL;
  NRF_CCM->EVENTS_ERROR = 0UL;
  NRF_CCM->EVENTS_ENDCRYPT = 0UL;
  NRF_CCM->TASKS_KSGEN = 1UL;
  while(NRF_CCM->EVENTS_ENDKSGEN == 0UL);
  if(NRF_CCM->EVENTS_ERROR) {
    return CCM_CRYPT_RUNTIME_ERROR;
  }

  /* Encrypt the packet */
  NRF_CCM->EVENTS_ENDCRYPT = 0UL;
  NRF_CCM->EVENTS_ERROR = 0UL;
  NRF_CCM->TASKS_CRYPT = 1UL;
  while(NRF_CCM->EVENTS_ENDCRYPT == 0UL);
  if(NRF_CCM->EVENTS_ERROR) {
    return CCM_CRYPT_RUNTIME_ERROR;
  }

  return CCM_CRYPT_SUCCESS;
}
/*---------------------------------------------------------------------------*/
uint8_t
osf_aes_ecb_encrypt(uint8_t *in_buf, uint8_t *out_buf, uint8_t buf_size)
{
  NRF_ECB->TASKS_STOPECB = 1UL;
  NRF_ECB->EVENTS_ENDECB = 0UL;
  NRF_ECB->INTENSET = 0UL;
  NRF_ECB->INTENCLR = 3UL;
  NRF_ECB->EVENTS_ERRORECB = 0UL;
  NRF_ECB->ECBDATAPTR = (uint32_t)&m_aes_ecb_config;

  memset(&m_aes_ecb_config, 0x00, sizeof(m_aes_ecb_config));
  memcpy(m_aes_ecb_config.aes_key, aes_ecb_key, AES_ECB_KEY_SIZE);
  buf_size = (buf_size > AES_ECB_BLOCK_SIZE) ? AES_ECB_BLOCK_SIZE : buf_size;
  memcpy(m_aes_ecb_config.aes_input, in_buf, buf_size /*AES_ECB_BLOCK_SIZE*/);

  NRF_ECB->TASKS_STARTECB = 1UL;
  while(NRF_ECB->EVENTS_ENDECB == 0UL) {
  }
  NRF_ECB->EVENTS_ENDECB = 0UL;

  if(NRF_ECB->EVENTS_ERRORECB) {
    NRF_ECB->TASKS_STOPECB = 1UL;
    return CCM_CRYPT_RUNTIME_ERROR;
  }

  NRF_ECB->TASKS_STOPECB = 1UL;
  memcpy(out_buf, m_aes_ecb_config.aes_output, buf_size /*AES_ECB_BLOCK_SIZE*/);

  return CCM_CRYPT_SUCCESS;
}

/*---------------------------------------------------------------------------*/
void
osf_srand_init(void)
{
  unsigned int seed;
  osf_rng_fill_buffer((void *)&seed, sizeof(seed));
  srand(seed);
}
/*---------------------------------------------------------------------------*/
void
osf_srand_fill_buffer(uint8_t *buf, uint32_t bufsize)
{
  for(uint32_t i = 0; i < bufsize; i++) {
    buf[i] = (uint8_t)rand();
  }
}
/*---------------------------------------------------------------------------*/
void
osf_rng_fill_buffer(uint8_t *buf, uint32_t bufsize)
{
  NRF_RNG->CONFIG = RNG_CONFIG_DERCEN_Enabled << RNG_CONFIG_DERCEN_Pos;
  while(bufsize--) {
    NRF_RNG->EVENTS_VALRDY = 0;
    NRF_RNG->TASKS_START = 1;
    while(NRF_RNG->EVENTS_VALRDY == 0);
    *buf++ = NRF_RNG->VALUE;
  }
}
/*---------------------------------------------------------------------------*/
