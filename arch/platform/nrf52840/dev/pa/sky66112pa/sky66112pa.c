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
 *         sky66112 pa driver
 * \author
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

/**
 * \addtogroup nrf52840-devices Device drivers
 * @{
 */

#include "dev/nullpa.h"
#include "nrf_gpio.h"
#include "sky66112pa.h"

#define FEM_CSD_PIN         NRF_GPIO_PIN_MAP(1, 15)
#define FEM_CPS_PIN         NRF_GPIO_PIN_MAP(0, 2)
#define FEM_CRX_PIN         NRF_GPIO_PIN_MAP(0, 14)
#define FEM_CTX_PIN         NRF_GPIO_PIN_MAP(0, 16)
#define FEM_CHL_PIN         NRF_GPIO_PIN_MAP(1, 13)
#define FEM_ANT_SEL_PIN     NRF_GPIO_PIN_MAP(1, 10)

static void off(void);

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  nrf_gpio_cfg_output(FEM_CSD_PIN);
  nrf_gpio_cfg_output(FEM_CPS_PIN);
  nrf_gpio_cfg_output(FEM_CRX_PIN);
  nrf_gpio_cfg_output(FEM_CTX_PIN);
  nrf_gpio_cfg_output(FEM_CHL_PIN);
  nrf_gpio_cfg_output(FEM_ANT_SEL_PIN);

  off();

  /* Apply default configuration */
  set_antenna(SKY66112PA_ANT_RX);
  set_tx_gain(SKY66112PA_MODE_TX);
  set_rx_gain(SKY66112PA_MODE_RX);
  set_attenuator(SKY66112PA_TX_ATT);
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
  nrf_gpio_pin_clear(FEM_CRX_PIN);
  nrf_gpio_pin_clear(FEM_CTX_PIN);
  nrf_gpio_pin_clear(FEM_CHL_PIN);
  nrf_gpio_pin_set(FEM_CPS_PIN);

  if(default_gain == PA_TX_Plus10dBm) {
    nrf_gpio_pin_clear(FEM_CPS_PIN);
  } else if(default_gain == PA_TX_Plus20dBm){
     nrf_gpio_pin_clear(FEM_CPS_PIN);
     nrf_gpio_pin_set(FEM_CHL_PIN);
  }
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
  nrf_gpio_pin_clear(FEM_CTX_PIN);
  nrf_gpio_pin_clear(FEM_CHL_PIN);
  nrf_gpio_pin_clear(FEM_CRX_PIN);
  
  if(default_gain == PA_RX_LNA) {
    nrf_gpio_pin_clear(FEM_CPS_PIN);
  } else {
     nrf_gpio_pin_set(FEM_CPS_PIN);
  }
}

/*---------------------------------------------------------------------------*/
static void
tx_begin(void)
{
  nrf_gpio_pin_set(FEM_CTX_PIN);
  nrf_gpio_pin_set(FEM_CSD_PIN);
}

/*---------------------------------------------------------------------------*/
static void
rx_begin(void)
{
  nrf_gpio_pin_set(FEM_CRX_PIN);
  nrf_gpio_pin_set(FEM_CSD_PIN);
}

/*---------------------------------------------------------------------------*/
static void
off(void)
{
  nrf_gpio_pin_clear(FEM_CSD_PIN);
  nrf_gpio_pin_clear(FEM_CPS_PIN);
  nrf_gpio_pin_clear(FEM_CRX_PIN);
  nrf_gpio_pin_clear(FEM_CTX_PIN);
  nrf_gpio_pin_clear(FEM_CHL_PIN);
  nrf_gpio_pin_clear(FEM_ANT_SEL_PIN);
}

/*---------------------------------------------------------------------------*/
const struct pa_driver sky66112pa_driver = {
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
