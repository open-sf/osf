/*
 * Copyright (c) 2020, George Oikonomou - https://spd.gr
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

/**
 * \addtogroup nrf
 * @{
 *
 * \addtogroup nrf-dev Device drivers
 * @{
 *
 * \addtogroup nrf-gpio GPIO HAL driver
 * @{
 * 
 * \file
 *     GPIO HAL implementation for the nRF
 * \author
 *     Yago Fontoura do Rosario <yago.rosario@hotmail.com.br>
 *
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"

#include "dev/gpio-hal.h"

#include "nrfx_gpiote.h"

#include "hal/nrf_gpio.h"

#ifndef NRF_GPIOTE_INSTANCE_ID
#define NRF_GPIOTE_INSTANCE_ID 0
#endif

/* TODO: there is for sure a macro for this */
#ifndef NRF_GPIOTE_NAME
#define NRF_GPIOTE_NAME NRF_GPIOTE
#endif

const nrfx_gpiote_t gpiote = NRFX_GPIOTE_INSTANCE(NRF_GPIOTE_INSTANCE_ID);

/*---------------------------------------------------------------------------*/
#define PIN_TO_PORT(pin) (pin >> 5)
#define PIN_TO_NUM(pin) (pin & 0x1F)
/*---------------------------------------------------------------------------*/
/**
 * @brief GPIO event handler
 * 
 * @param pin GPIO pin
 * @param action Action
 */
static void
pin_event_handler(nrfx_gpiote_pin_t pin, nrfx_gpiote_trigger_t trigger, void *context)
{
  gpio_hal_port_t port;
  gpio_hal_pin_mask_t pin_mask;

  port = PIN_TO_PORT(pin);
  pin_mask = gpio_hal_pin_to_mask(PIN_TO_NUM(pin));

  gpio_hal_event_handler(port, pin_mask);
}
/*---------------------------------------------------------------------------*/
void
gpio_hal_arch_init(void)
{
  //if(!nrfx_gpiote_is_init()) { /* TODO: track init */
  {
    nrfx_gpiote_init(&gpiote,NRFX_GPIOTE_DEFAULT_CONFIG_IRQ_PRIORITY);
  }
}
/*---------------------------------------------------------------------------*/
void
gpio_hal_arch_port_pin_cfg_set(gpio_hal_port_t port, gpio_hal_pin_t pin, gpio_hal_pin_cfg_t cfg)
{
  gpio_hal_pin_cfg_t tmp;

  uint8_t in_channel;
  nrfx_gpiote_channel_alloc(&gpiote, &in_channel); /* TODO check return value */

  nrf_gpio_pin_pull_t  pull_config;

  nrfx_gpiote_trigger_config_t trigger_config;
  trigger_config.p_in_channel = &in_channel;

  nrfx_gpiote_handler_config_t handler_config = {
		.handler = pin_event_handler,
	};

  uint32_t pin_number = NRF_GPIO_PIN_MAP(port, pin);

  tmp = cfg & GPIO_HAL_PIN_CFG_EDGE_BOTH;
  if(tmp == GPIO_HAL_PIN_CFG_EDGE_NONE) {
    trigger_config.trigger = NRFX_GPIOTE_TRIGGER_NONE;
  } else if(tmp == GPIO_HAL_PIN_CFG_EDGE_RISING) {
    trigger_config.trigger = NRFX_GPIOTE_TRIGGER_LOTOHI;
  } else if(tmp == GPIO_HAL_PIN_CFG_EDGE_FALLING) {
    trigger_config.trigger = NRFX_GPIOTE_TRIGGER_HITOLO;
  } else if(tmp == GPIO_HAL_PIN_CFG_EDGE_BOTH) {
    trigger_config.trigger = NRFX_GPIOTE_TRIGGER_TOGGLE;
  }

  tmp = cfg & GPIO_HAL_PIN_CFG_PULL_MASK;
  if(tmp == GPIO_HAL_PIN_CFG_PULL_NONE) {
    pull_config = NRF_GPIO_PIN_NOPULL;
  } else if(tmp == GPIO_HAL_PIN_CFG_PULL_DOWN) {
    pull_config = NRF_GPIO_PIN_PULLDOWN;
  } else if(tmp == GPIO_HAL_PIN_CFG_PULL_UP) {
    pull_config = NRF_GPIO_PIN_PULLUP;
  }

  nrfx_gpiote_input_pin_config_t input_config = {
		.p_pull_config = &pull_config,
		.p_trigger_config = &trigger_config,
		.p_handler_config = &handler_config
	};

  nrfx_gpiote_input_configure(&gpiote, pin_number, &input_config);

  tmp = cfg & GPIO_HAL_PIN_CFG_INT_MASK;
  if(tmp == GPIO_HAL_PIN_CFG_INT_DISABLE) {
    nrfx_gpiote_trigger_disable(&gpiote, pin_number);
  } else if(tmp == GPIO_HAL_PIN_CFG_INT_ENABLE) {
    nrfx_gpiote_trigger_enable(&gpiote, pin_number, true);
  }
}
/*---------------------------------------------------------------------------*/
gpio_hal_pin_cfg_t
gpio_hal_arch_port_pin_cfg_get(gpio_hal_port_t port, gpio_hal_pin_t pin)
{
  uint8_t i;
  uint32_t pin_number;
  gpio_hal_pin_cfg_t cfg = GPIO_HAL_PIN_CFG_PULL_NONE |
    GPIO_HAL_PIN_CFG_EDGE_NONE |
    GPIO_HAL_PIN_CFG_INT_DISABLE;
  nrf_gpio_pin_pull_t pull;
  nrf_gpiote_polarity_t polarity;

  pin_number = NRF_GPIO_PIN_MAP(port, pin);

  /* First, check if the pin is configured as output */
  if(nrf_gpio_pin_dir_get(pin_number) == NRF_GPIO_PIN_DIR_OUTPUT) {
    return 0;
  }

  /*
   * Input pin. Check all GPIOTE channel configurations and figure out which
   * channel corresponds to our pin of interest. For that channel, read out
   * the GPIOTE configuration
   */
  for(i = 0; i < GPIOTE_CH_NUM; i++) {
    if(nrf_gpiote_event_pin_get(NRF_GPIOTE_NAME, i) == pin_number) {
      polarity = nrf_gpiote_event_polarity_get(NRF_GPIOTE_NAME, i);

      if(polarity == NRF_GPIOTE_POLARITY_LOTOHI) {
        cfg |= GPIO_HAL_PIN_CFG_EDGE_BOTH;
      } else if(polarity == NRF_GPIOTE_POLARITY_HITOLO) {
        cfg |= GPIO_HAL_PIN_CFG_EDGE_BOTH;
      } else if(polarity == NRF_GPIOTE_POLARITY_TOGGLE) {
        cfg |= GPIO_HAL_PIN_CFG_EDGE_BOTH;
      }

      pull = nrf_gpio_pin_pull_get(pin_number);

      if(pull == NRF_GPIO_PIN_PULLDOWN) {
        cfg |= GPIO_HAL_PIN_CFG_PULL_DOWN;
      } else if(pull == NRF_GPIO_PIN_PULLUP) {
        cfg |= GPIO_HAL_PIN_CFG_PULL_UP;
      }

      if(nrf_gpiote_int_enable_check(NRF_GPIOTE_NAME, 1 << i)) {
        cfg |= GPIO_HAL_PIN_CFG_INT_ENABLE;
      }
      return cfg;
    }
  }

  /* Did not find a GPIOTE channel configured for this pin */
  return 0;
}
/*---------------------------------------------------------------------------*/
uint8_t
gpio_hal_arch_port_read_pin(gpio_hal_port_t port, gpio_hal_pin_t pin)
{
  return (uint8_t)nrf_gpio_pin_read(NRF_GPIO_PIN_MAP(port, pin));
}
/*---------------------------------------------------------------------------*/
void
gpio_hal_arch_port_set_pins(gpio_hal_port_t port, gpio_hal_pin_mask_t pins)
{
  NRF_GPIO_Type *gpio_regs[GPIO_COUNT] = GPIO_REG_LIST;

  if(port >= GPIO_COUNT) {
    return;
  }

  nrf_gpio_port_out_set(gpio_regs[port], pins);
}
/*---------------------------------------------------------------------------*/
void
gpio_hal_arch_port_clear_pins(gpio_hal_port_t port, gpio_hal_pin_mask_t pins)
{
  NRF_GPIO_Type *gpio_regs[GPIO_COUNT] = GPIO_REG_LIST;

  if(port >= GPIO_COUNT) {
    return;
  }

  nrf_gpio_port_out_clear(gpio_regs[port], pins);
}
/*---------------------------------------------------------------------------*/
void
gpio_hal_arch_port_toggle_pins(gpio_hal_port_t port, gpio_hal_pin_mask_t pins)
{
  if(port >= GPIO_COUNT) {
    return;
  }
  gpio_hal_arch_write_pins(port, pins, ~gpio_hal_arch_read_pins(port, pins));
}
/*---------------------------------------------------------------------------*/
gpio_hal_pin_mask_t
gpio_hal_arch_port_read_pins(gpio_hal_port_t port, gpio_hal_pin_mask_t pins)
{
  NRF_GPIO_Type *gpio_regs[GPIO_COUNT] = GPIO_REG_LIST;

  if(port >= GPIO_COUNT) {
    return 0;
  }

  return nrf_gpio_port_in_read(gpio_regs[port]);
}
/*---------------------------------------------------------------------------*/
void
gpio_hal_arch_port_write_pins(gpio_hal_port_t port, gpio_hal_pin_mask_t pins,
                              gpio_hal_pin_mask_t value)
{
  NRF_GPIO_Type *gpio_regs[GPIO_COUNT] = GPIO_REG_LIST;

  if(port >= GPIO_COUNT) {
    return;
  }

  nrf_gpio_port_out_write(gpio_regs[port], value);
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
