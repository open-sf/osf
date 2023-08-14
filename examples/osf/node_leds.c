/*
 * Copyright (c) 2022, technology Innovation Institute (TII).
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Different variants of LEDs usage
 * \author
 *         Michael Baddeley <michael@ssrc.tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 *
 */
#include "contiki.h"
#include "contiki-net.h"
#include "nrf_gpio.h"
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-debug.h"

void led_on(uint8_t led_n);
void led_off(uint8_t led_n);

#if HWVAR == 1
#define SF0_LEDG                      NRF_GPIO_PIN_MAP(0, 13)
#define SF0_LEDR                      NRF_GPIO_PIN_MAP(0, 7)
#define SF1_LEDG                      NRF_GPIO_PIN_MAP(1, 8)
#define SF1_LEDR                      NRF_GPIO_PIN_MAP(1, 9)
#define SF2_LEDG                      NRF_GPIO_PIN_MAP(0, 15)
#define SF2_LEDR                      NRF_GPIO_PIN_MAP(0, 11)
#else
static const uint32_t dk_leds[] = {0, LED_1, LED_2, LED_3, LED_4, 0};
#endif

extern uint8_t node_is_timesync;

/*---------------------------------------------------------------------------*/
/* Application specific LEDs */
/*---------------------------------------------------------------------------*/
void 
osf_leds_init(void)
{
  osf_led_on = led_on;
  osf_led_off = led_off;

#if HWVAR== 1
  nrf_gpio_cfg_output(SF0_LEDG);
  nrf_gpio_pin_set(SF0_LEDG);

  nrf_gpio_cfg_output(SF0_LEDR);
  nrf_gpio_pin_set(SF0_LEDR);

  nrf_gpio_cfg_output(SF1_LEDG);
  nrf_gpio_pin_set(SF1_LEDG);

  nrf_gpio_cfg_output(SF1_LEDR);
  nrf_gpio_pin_set(SF1_LEDR);

  nrf_gpio_cfg_output(SF2_LEDG);
  nrf_gpio_pin_set(SF2_LEDG);

  nrf_gpio_cfg_output(SF2_LEDR);
  nrf_gpio_pin_set(SF2_LEDR);
#else
  nrf_gpio_cfg_output(LED_1);
  nrf_gpio_pin_set(LED_1);

  nrf_gpio_cfg_output(LED_2);
  nrf_gpio_pin_set(LED_2);

  nrf_gpio_cfg_output(LED_3);
  nrf_gpio_pin_set(LED_3);

  nrf_gpio_cfg_output(LED_4);
  nrf_gpio_pin_set(LED_4);
#endif  
}
/*---------------------------------------------------------------------------*/
void
led_on(uint8_t led_n)
{
#if HWVAR == 1
  switch(led_n) {
    case TS_LED:
    if(node_is_timesync) {
      nrf_gpio_pin_clear(SF2_LEDR);
      nrf_gpio_pin_clear(SF2_LEDG);
    }
    break;
    case ROUND_LED:
     nrf_gpio_pin_clear(SF1_LEDG);
    if(node_is_timesync) {     
      nrf_gpio_pin_clear(SF1_LEDR);
    }
    break;
    case SYNCED_LED:
    if(!node_is_timesync) {
      nrf_gpio_pin_clear(SF2_LEDG);
    }
    break;
    case CRCERR_LED:
      nrf_gpio_pin_clear(SF0_LEDR);
    break;
    default:
    break;
  }
#else  
  nrf_gpio_pin_clear(dk_leds[led_n]);
#endif  
}
/*---------------------------------------------------------------------------*/
void
led_off(uint8_t led_n)
{
#if HWVAR == 1
 switch(led_n) {
    case TS_LED:
    if(node_is_timesync) {
      nrf_gpio_pin_set(SF2_LEDR);
      nrf_gpio_pin_set(SF2_LEDG);
    }
    break;
    case ROUND_LED:
     nrf_gpio_pin_set(SF1_LEDG);
    if(node_is_timesync) {     
      nrf_gpio_pin_set(SF1_LEDR);
    }
    break;
    case SYNCED_LED:
    if(!node_is_timesync) {
      nrf_gpio_pin_set(SF2_LEDG);
    }
    break;
    case CRCERR_LED:
      nrf_gpio_pin_set(SF0_LEDR);
    break;
    default:
    break;
  }
#else  
  nrf_gpio_pin_set(dk_leds[led_n]);
#endif  
}
/*---------------------------------------------------------------------------*/
void
leds_all_on(void)
{
#if HWVAR == 1  
  nrf_gpio_pin_clear(SF0_LEDG);
  nrf_gpio_pin_clear(SF0_LEDR);
  nrf_gpio_pin_clear(SF1_LEDG);
  nrf_gpio_pin_clear(SF1_LEDR);
  nrf_gpio_pin_clear(SF2_LEDG);
  nrf_gpio_pin_clear(SF2_LEDR);
#else
  nrf_gpio_pin_clear(LED_1);
  nrf_gpio_pin_clear(LED_2);
  nrf_gpio_pin_clear(LED_3);
  nrf_gpio_pin_clear(LED_4);
#endif  
}
/*---------------------------------------------------------------------------*/
