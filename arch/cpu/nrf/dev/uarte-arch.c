/*
 * Copyright (C) 2020 Yago Fontoura do Rosario <yago.rosario@hotmail.com.br>
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup nrf
 * @{
 *
 * \addtogroup nrf-dev Device drivers
 * @{
 *
 * \addtogroup nrf-uarte UARTE driver
 * @{
 *
 * \file
 *         UARTE implementation for the nRF.
 * \author
 *         Yago Fontoura do Rosario <yago.rosario@hotmail.com.br>
 *
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/rtimer.h"
/*---------------------------------------------------------------------------*/
#if NRF_HAS_UARTE
/*---------------------------------------------------------------------------*/
#include "nrfx_config.h"
#include "nrfx_uarte.h"
#include "hal/nrf_gpio.h"
/*---------------------------------------------------------------------------*/
static int (*input_handler)(unsigned char c) = NULL;

#ifndef NRF_UARTE_INSTANCE_ID
#define NRF_UARTE_INSTANCE_ID 0
#endif

static nrfx_uarte_t instance = NRFX_UARTE_INSTANCE(NRF_UARTE_INSTANCE_ID);
static uint8_t uarte_buffer;
static volatile bool uart_tx_done = true;
static bool is_initialized = false;
/*---------------------------------------------------------------------------*/
/**
 * \brief Send one byte via UARTE
 * \param data The byte to send
 */
void
uarte_write(unsigned char data)
{
  if(!is_initialized) {
    return;
  }

  rtimer_clock_t start = RTIMER_NOW();
  const rtimer_clock_t timeout = RTIMER_SECOND / 4; // ~250 ms

  while(!uart_tx_done) {
    if(!RTIMER_CLOCK_LT(RTIMER_NOW(), start + timeout)) {
      return;
    }
  }

  uart_tx_done = false;
  nrfx_err_t err = nrfx_uarte_tx(&instance, &data, sizeof(data), 0);
  if(err != NRFX_SUCCESS) {
    uart_tx_done = true;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * @brief UARTE event handler
 *
 * @param p_event UARTE event
 * @param p_context UARTE context
 */
static void
uarte_handler(nrfx_uarte_event_t const *p_event, void *p_context)
{
  if(p_event->type == NRFX_UARTE_EVT_RX_DONE) {
    if(input_handler) {
      uint8_t *p_data = p_event->data.rx.p_buffer;
      size_t bytes = p_event->data.rx.length;
      for(size_t i = 0; i < bytes; i++) {
        input_handler(p_data[i]);
      }
    }
    nrfx_uarte_rx(&instance, &uarte_buffer, sizeof(uarte_buffer));
  } else if(p_event->type == NRFX_UARTE_EVT_TX_DONE) {
    uart_tx_done = true;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Set the input handler for received bytes
 * \param input Pointer to the input handler function
 */
void
uarte_set_input(int (*input)(unsigned char c))
{
  input_handler = input;
  if(input) {
    nrfx_uarte_rx(&instance, &uarte_buffer, sizeof(uarte_buffer));
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize the UARTE peripheral
 */
void
uarte_init(void)
{
#if defined(NRF_UARTE0_TX_PORT) && defined(NRF_UARTE0_TX_PIN) && \
    defined(NRF_UARTE0_RX_PORT) && defined(NRF_UARTE0_RX_PIN)
  const nrfx_uarte_config_t config = NRFX_UARTE_DEFAULT_CONFIG(
    NRF_GPIO_PIN_MAP(NRF_UARTE0_TX_PORT, NRF_UARTE0_TX_PIN),
    NRF_GPIO_PIN_MAP(NRF_UARTE0_RX_PORT, NRF_UARTE0_RX_PIN)
  );

  nrfx_uarte_init(&instance, &config, uarte_handler);
#else
  (void) uarte_handler;
#endif /* defined(NRF_UARTE0_TX_PORT) && defined(NRF_UARTE0_TX_PIN) && defined(NRF_UARTE0_RX_PORT) && defined(NRF_UARTE0_RX_PIN) */

  is_initialized = true;
}
/*---------------------------------------------------------------------------*/
#endif /* NRF_HAS_UARTE */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
