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

 /**
 * \file
 *         OSF debug.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

#ifndef OSF_DEBUG_H_
#define OSF_DEBUG_H_
/*---------------------------------------------------------------------------*/
/* LEDs */
/*---------------------------------------------------------------------------*/

/* Callback for implement led indication in application */
typedef void (*osf_led_on_t)(uint8_t led_n);
typedef void (*osf_led_off_t)(uint8_t led_n);
extern osf_led_on_t osf_led_on;
extern osf_led_off_t osf_led_off;

#if 0
#define TS_LED                          LED_1
#define ROUND_LED                       LED_2
#define SYNCED_LED                      LED_3
#define CRCERR_LED                      LED_4
#else
#define TS_LED                          1
#define ROUND_LED                       2
#define SYNCED_LED                      3
#define CRCERR_LED                      4
#endif

#if OSF_DEBUG_LEDS
//#define DEBUG_LEDS_ON(n)                nrf_gpio_pin_clear(n)
//#define DEBUG_LEDS_OFF(n)               nrf_gpio_pin_set(n)
#define DEBUG_LEDS_ON(n)                if(NULL!=osf_led_on){osf_led_on(n);}
#define DEBUG_LEDS_OFF(n)               if(NULL!=osf_led_off){osf_led_off(n);}

#else /* OSF_DEBUG_LEDS */

#define DEBUG_LEDS_ON(n)
#define DEBUG_LEDS_OFF(n)

#endif /* OSF_DEBUG_LEDS */

/*---------------------------------------------------------------------------*/
/* GPIO */
/*---------------------------------------------------------------------------*/
/* GPIOTE channel assignments, max 8 */
#define RADIO_TXRX_GPIOTE_CH            0UL
#define RADIO_READY_EVENT_GPIOTE_CH     1UL
#define RADIO_ADDRESS_EVENT_GPIOTE_CH   2UL
#define RADIO_PAYLOAD_EVENT_GPIOTE_CH   3UL
#define RADIO_END_EVENT_GPIOTE_CH       4UL
#define RADIO_BCMATCH_EVENT_GPIOTE_CH   5UL
#define RADIO_TXEN_GPIOTE_CH            6UL
#define RADIO_RXEN_GPIOTE_CH            7UL

/* PPI channel assignments, max 20. */
#define RADIO_READY_EVENT_PPI_CH        0UL
#define RADIO_ADDRESS_EVENT_PPI_CH      1UL
#define RADIO_PAYLOAD_EVENT_PPI_CH      2UL
#define RADIO_END_EVENT_PPI_CH          3UL
#define RADIO_BCMATCH_EVENT_PPI_CH      4UL
#define RADIO_TXRX_PPI_CH0              5UL
#define RADIO_TXRX_PPI_CH1              6UL
#define RADIO_T0_TX_EVENT_PPI_CH        7UL
#define RADIO_T0_RX_EVENT_PPI_CH        8UL
#define RADIO_FRAME_EVENT_PPI_CH        9UL

#define RADIO_TXEN_CH                   10UL
#define RADIO_RXEN_CH                   11UL
#define RADIO_READY_CH                  12UL
#define RADIO_ADDRESS_CH                13UL
#define RADIO_END_CH                    14UL
#define RADIO_BCMATCH_CH                15UL

#if OSF_DEBUG_GPIO

/* Enable individual event indication */
#define RADIO_TXRX_PIN                  NRF_GPIO_PIN_MAP(0, 31) // high on READY, low on END
#define RADIO_IRQ_EVENT_PIN             NRF_GPIO_PIN_MAP(0, 30) // high on IRQ in, low - on out
// #define DBG_PIN1                        NRF_GPIO_PIN_MAP(1, 5)  // DBG1
#define DBG_PIN2                        NRF_GPIO_PIN_MAP(1, 6)  // DBG2
#define DBG_PIN3                        NRF_GPIO_PIN_MAP(1, 7)  // DBG3
// #define DBG_PIN4                        NRF_GPIO_PIN_MAP(1, 8)  // DBG4
#define DBG_PIN4                        NRF_GPIO_PIN_MAP(0, 29)  // DBG4
// #define RADIO_READY_EVENT_PIN           NRF_GPIO_PIN_MAP(0, 29) // READY
#define RADIO_ADDRESS_EVENT_PIN         NRF_GPIO_PIN_MAP(1, 5) // ADDRESS
// #define RADIO_PAYLOAD_EVENT_PIN         NRF_GPIO_PIN_MAP(1, 6) // PAYLOAD
// #define RADIO_END_EVENT_PIN             NRF_GPIO_PIN_MAP(0, 29) // END
// #define RADIO_BCMATCH_EVENT_PIN         NRF_GPIO_PIN_MAP(0, 29) // BCMATCH
// #define RADIO_TXEN_PIN                  NRF_GPIO_PIN_MAP(1, 12) // TXEN
// #define RADIO_RXEN_PIN                  NRF_GPIO_PIN_MAP(1, 13) // RXEN

#define DEBUG_GPIO_ON(pin)              nrf_gpio_pin_set(pin)
#define DEBUG_GPIO_OFF(pin)             nrf_gpio_pin_clear(pin)
#define DEBUG_SPIKE_GPIO(pin)           do { DEBUG_GPIO_ON(pin); DEBUG_GPIO_OFF(pin); } while(0)
#define DEBUG_GROOVE_GPIO(pin)          do { DEBUG_GPIO_OFF(pin); DEBUG_GPIO_ON(pin); } while(0)
#define DEBUG_TOGGLE_GPIO(pin)          nrf_gpio_pin_toggle(pin)
#define DEBUG_FLASH_GPIO(n, pin)        { uint8_t i = n + 1; while(i) { \
                                            nrf_gpio_pin_toggle(pin); \
                                            nrf_gpio_pin_toggle(pin); \
                                            i--; } }

#if RADIO_TXEN_PIN
#define DEBUG_GPIO_OUTPUT_TXEN() do { NRF_GPIOTE->TASKS_OUT[RADIO_TXEN_GPIOTE_CH] = 1UL; } while(0)
#else
#define DEBUG_GPIO_OUTPUT_TXEN()
#endif /* RADIO_TXEN_PIN */

#if RADIO_RXEN_PIN
#define DEBUG_GPIO_OUTPUT_RXEN() do { NRF_GPIOTE->TASKS_OUT[RADIO_RXEN_GPIOTE_CH] = 1UL; } while(0)
#else
#define DEBUG_GPIO_OUTPUT_RXEN()
#endif /* RADIO_RXEN_PIN */

#else /* OSF_DEBUG_GPIO */

#define DEBUG_GPIO_ON(pin)
#define DEBUG_GPIO_OFF(pin)
#define DEBUG_SPIKE_GPIO(pin)
#define DEBUG_GROOVE_GPIO(pin)
#define DEBUG_FLASH_GPIO(n, pin)
#define DEBUG_GPIO_OUTPUT_TXEN()
#define DEBUG_GPIO_OUTPUT_RXEN()

#endif /* OSF_DEBUG_GPIO */

/*---------------------------------------------------------------------------*/
/**/
/*---------------------------------------------------------------------------*/
void osf_debug_gpio_init();
void osf_debug_configure_pins();
void osf_debug_clear_pins();

#endif /* OSF_DEBUG_H_ */
