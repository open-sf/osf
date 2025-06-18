/*
 * Copyright (c) 2021, technology Innovation Institute (TII).
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
 *         nrf21540 pa driver
 * \author
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

/**
 * \addtogroup nrf52840-devices Device drivers
 * @{
 */

#include "dev/nullpa.h"
#include "nrf_gpio.h"
#include "nrf21540pa.h"

#define FEM_MODE_PIN         NRF_GPIO_PIN_MAP(0, 17)
#define FEM_RX_EN_PIN        NRF_GPIO_PIN_MAP(0, 19)
#define FEM_TX_EN_PIN        NRF_GPIO_PIN_MAP(0, 22)
#define FEM_ANT_SEL_PIN      NRF_GPIO_PIN_MAP(0, 20)
#define FEM_NPDN_PIN         NRF_GPIO_PIN_MAP(0, 23)
#define FEM_NCSN_PIN         NRF_GPIO_PIN_MAP(0, 21)

/* Forward declarations of static functions */
static void off(void);
static void set_antenna(pa_lna_ant_t default_ant);
static void set_tx_gain(pa_tx_gain_t default_gain);
static void set_rx_gain(pa_rx_gain_t default_gain);

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  nrf_gpio_cfg_output(FEM_MODE_PIN);
  nrf_gpio_cfg_output(FEM_RX_EN_PIN);
  nrf_gpio_cfg_output(FEM_TX_EN_PIN);
  nrf_gpio_cfg_output(FEM_ANT_SEL_PIN);
  nrf_gpio_cfg_output(FEM_NPDN_PIN);
  nrf_gpio_cfg_output(FEM_NCSN_PIN);

  off();

  // Set up default FEM configuration using #defines
  set_antenna(NRF21540PA_ANT_TX);     // Set default TX antenna
  set_tx_gain(NRF21540PA_MODE_TX);    // Set default TX gain
  set_rx_gain(NRF21540PA_MODE_RX);    // Set default RX gain
}

/*---------------------------------------------------------------------------*/
static void
set_antenna(pa_lna_ant_t default_ant)
{
  nrf_gpio_pin_write(FEM_ANT_SEL_PIN, default_ant & 0x1);
}

/*---------------------------------------------------------------------------*/
static void
set_tx_gain(pa_tx_gain_t default_gain)
{
  nrf_gpio_pin_write(FEM_MODE_PIN, default_gain & 0x1);
}

/*---------------------------------------------------------------------------*/
static void
set_attenuator(uint8_t mode)
{
  // N/A
}

/*---------------------------------------------------------------------------*/
static void
set_rx_gain(pa_rx_gain_t default_gain)
{
  // LNA
}

/*---------------------------------------------------------------------------*/
static void
tx_begin(void)
{
  nrf_gpio_pin_set(FEM_NPDN_PIN);
  nrf_gpio_pin_clear(FEM_NCSN_PIN);

  nrf_gpio_pin_clear(FEM_RX_EN_PIN);
  nrf_gpio_pin_set(FEM_TX_EN_PIN);
}

/*---------------------------------------------------------------------------*/
static void
rx_begin(void)
{
  nrf_gpio_pin_set(FEM_NPDN_PIN);
  nrf_gpio_pin_clear(FEM_NCSN_PIN);

  nrf_gpio_pin_clear(FEM_TX_EN_PIN);
  nrf_gpio_pin_set(FEM_RX_EN_PIN);
}

/*---------------------------------------------------------------------------*/
static void
off(void)
{
  nrf_gpio_pin_clear(FEM_NPDN_PIN);
  nrf_gpio_pin_set(FEM_NCSN_PIN);

  nrf_gpio_pin_clear(FEM_RX_EN_PIN);
  nrf_gpio_pin_clear(FEM_TX_EN_PIN);

  nrf_gpio_pin_clear(FEM_ANT_SEL_PIN);
}

/*---------------------------------------------------------------------------*/
const struct pa_driver nrf21540pa_driver = {
  init,
  set_antenna,
  set_tx_gain,
  set_attenuator,
  set_rx_gain,
  tx_begin,
  rx_begin,
  off,
};
/*---------------------------------------------------------------------------*/
/** @} */
