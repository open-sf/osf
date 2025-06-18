/*
 * Copyright (c) 2024, technology Innovation Institute (TII).
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
 *         se2436l pa driver
 * \author
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

/**
 * \addtogroup nrf52840-devices Device drivers
 * @{
 */

#include "dev/nullpa.h"
#include "nrf_gpio.h"
#include "se2436lpa.h"

#define FEM_CSD_PIN         NRF_GPIO_PIN_MAP(1, 15)
#define FEM_CPS_PIN         NRF_GPIO_PIN_MAP(0, 2)
#define FEM_CRX_PIN         NRF_GPIO_PIN_MAP(0, 14)
#define FEM_CTX_PIN         NRF_GPIO_PIN_MAP(0, 16)
/*
 * FEM_nRF52_CHL which is now driven to DCDC converter,
 * chooses either 4V2 or 3v3 for FEM. 
 * It is connected to P1.13 and should be driven high by default.
*/
#define FEM_CHL_PIN         NRF_GPIO_PIN_MAP(1, 13) // DCDC converter
#define FEM_ANT_SEL_PIN     NRF_GPIO_PIN_MAP(1, 10) // control antenna

#define DA_V1              NRF_GPIO_PIN_MAP(1,1)
#define DA_V2              NRF_GPIO_PIN_MAP(1,2)
#define DA_V3              NRF_GPIO_PIN_MAP(1,3)
#define DA_V4              NRF_GPIO_PIN_MAP(1,4)

#define PIN1               NRF_GPIO_PIN_MAP(0,12)
#define PIN2               NRF_GPIO_PIN_MAP(0,24)

/* Forward declarations of static functions */
static void off(void);
static void set_antenna(pa_lna_ant_t default_ant);
static void set_tx_gain(pa_tx_gain_t default_gain);
static void set_rx_gain(pa_rx_gain_t default_gain);
static void set_attenuator(uint8_t mode);

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
  
  nrf_gpio_cfg_input(PIN1, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(PIN2, NRF_GPIO_PIN_NOPULL);

  nrf_gpio_pin_set(FEM_CHL_PIN); // DCDC converter
  
  nrf_gpio_cfg_output(DA_V1);
  nrf_gpio_cfg_output(DA_V2);
  nrf_gpio_cfg_output(DA_V3);
  nrf_gpio_cfg_output(DA_V4);
  nrf_gpio_pin_set(DA_V1);
  nrf_gpio_pin_set(DA_V2);
  nrf_gpio_pin_set(DA_V3);
  nrf_gpio_pin_set(DA_V4);
  
  off();

  /* Apply default configuration */
  set_antenna(SE2436LPA_ANT_RX);
  set_tx_gain(SE2436LPA_MODE_TX);
  set_rx_gain(SE2436LPA_MODE_RX);
  set_attenuator(SE2436LPA_TX_ATT);
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
  nrf_gpio_pin_clear(FEM_CPS_PIN);

  if(default_gain > PA_TX_Bypass){
    nrf_gpio_pin_set(FEM_CPS_PIN);
  }
}

/*---------------------------------------------------------------------------*/
static void
set_attenuator(uint8_t mode)
{
  mode =  mode & 0x1F;
  // control DCDC convertor
  nrf_gpio_pin_write(FEM_CHL_PIN, (mode >> 4)&0x1);
  // set attenuation value
  nrf_gpio_pin_write(DA_V1, (mode)&0x1);   // P1.01
  nrf_gpio_pin_write(DA_V2, (mode>>1)&0x1);// P1.02
  nrf_gpio_pin_write(DA_V3, (mode>>2)&0x1);// P1.03
  nrf_gpio_pin_write(DA_V4, (mode>>3)&0x1);// P1.04
}

/*---------------------------------------------------------------------------*/
static void
set_rx_gain(pa_rx_gain_t default_gain)
{
  nrf_gpio_pin_clear(FEM_CTX_PIN);
  nrf_gpio_pin_clear(FEM_CRX_PIN);
  nrf_gpio_pin_clear(FEM_CPS_PIN);
  
  if(default_gain == PA_RX_LNA) {
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
  
  nrf_gpio_pin_clear(FEM_ANT_SEL_PIN);
}

/*---------------------------------------------------------------------------*/
const struct pa_driver se2436lpa_driver = {
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
