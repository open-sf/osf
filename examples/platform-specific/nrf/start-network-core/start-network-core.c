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
 *
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
#include "contiki.h"

#include "hal/nrf_gpio.h"
#include "hal/nrf_reset.h"
#include "hal/nrf_spu.h"
/*---------------------------------------------------------------------------*/
PROCESS(empty_application, "Empty Application");
AUTOSTART_PROCESSES(&empty_application);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(empty_application, ev, data)
{
  PROCESS_BEGIN();

  /* Forward buttons */
  nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(NRF_BUTTON1_PORT, NRF_BUTTON1_PIN),
                              NRF_GPIO_PIN_SEL_NETWORK);
  nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(NRF_BUTTON2_PORT, NRF_BUTTON2_PIN),
                              NRF_GPIO_PIN_SEL_NETWORK);
  nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(NRF_BUTTON3_PORT, NRF_BUTTON3_PIN),
                              NRF_GPIO_PIN_SEL_NETWORK);
  nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(NRF_BUTTON4_PORT, NRF_BUTTON4_PIN),
                              NRF_GPIO_PIN_SEL_NETWORK);

  /* Forward LEDS */
  nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(NRF_LED1_PORT, NRF_LED1_PIN),
                              NRF_GPIO_PIN_SEL_NETWORK);
  nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(NRF_LED2_PORT, NRF_LED2_PIN),
                              NRF_GPIO_PIN_SEL_NETWORK);
  nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(NRF_LED3_PORT, NRF_LED3_PIN),
                              NRF_GPIO_PIN_SEL_NETWORK);
  nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(NRF_LED4_PORT, NRF_LED4_PIN),
                              NRF_GPIO_PIN_SEL_NETWORK);

  /* Forward UARTE */
  nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(NRF_UARTE0_TX_PORT, NRF_UARTE0_TX_PIN),
                              NRF_GPIO_PIN_SEL_NETWORK);
  nrf_gpio_pin_control_select(NRF_GPIO_PIN_MAP(NRF_UARTE0_RX_PORT, NRF_UARTE0_RX_PIN),
                              NRF_GPIO_PIN_SEL_NETWORK);

  /* Network MCU in Secure domain */
  nrf_spu_extdomain_set(NRF_SPU, 0, true, false);

  /* Release the Network MCU, 'Release force off signal' */
  nrf_reset_network_force_off(NRF_RESET, false);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
