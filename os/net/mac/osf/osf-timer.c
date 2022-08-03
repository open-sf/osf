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
 *         OSF timer.
 * \author
 *         Michael Baddeley <michael@ssrc.tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 *
 */

#include "contiki.h"
#include "node-id.h"
#include "nrf_clock.h"
#include "nrf_timer.h"
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-timer.h"
#include "net/mac/osf/osf-debug.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "OSF-TIMER"
#define LOG_LEVEL LOG_LEVEL_ERR

static volatile rtimer_callback_t cb_timer;

// TODO: All this needs to be moved to the nRF platform. We need a generic OSF
//       timer interface (like rtimer) to handle all this stuff.
/*---------------------------------------------------------------------------*/
void
rtimerx_init()
{
  /* Clear timer interrupts */
  NVIC_DisableIRQ(TIMERX_IRQn);
  NVIC_ClearPendingIRQ(TIMERX_IRQn);
  NVIC_SetPriority(TIMERX_IRQn, 1);

  /* Set the timer in Timer Mode */
  NRF_TIMERX->MODE = TIMER_MODE_MODE_Timer;
  /* Clear the task to make sure the timer is stopped */
  NRF_TIMERX->TASKS_STOP = 1UL;
  NRF_TIMERX->TASKS_CLEAR = 1UL;
  /* Prescaler 0 produces 16MHz timer tick frequency */
  NRF_TIMERX->PRESCALER = NRF_TIMER_FREQ_16MHz;
  /* 32 bit mode */
  NRF_TIMERX->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
  /* Enable shorts */
  NRF_TIMERX->SHORTS = 0UL;

  NVIC_EnableIRQ(TIMERX_IRQn);

  /* Start Timer */
  NRF_TIMERX->TASKS_START = 1UL;
}

/*----------------------------------------------------------------------------*/
rtimer_clock_t
rtimerx_now()
{
  NRF_TIMERX->TASKS_CAPTURE[NOW_REG] = 1UL;
  return NRF_TIMERX->CC[NOW_REG];
}

/*---------------------------------------------------------------------------*/
void
rtimerx_set(rtimer_clock_t value, rtimer_callback_t cb)
{
  cb_timer = cb;
  NRF_TIMERX->EVENTS_COMPARE[SCHEDULE_REG] = 0UL;
  NRF_TIMERX->CC[SCHEDULE_REG] = value;
  NRF_TIMERX->INTENSET = TIMER_INTENSET_COMPARE_SCHEDULE_REG_Msk;
}

/*---------------------------------------------------------------------------*/
uint8_t
rtimerx_set_fixed(rtimer_clock_t fixed_time, rtimer_callback_t cb, char* msg)
{
  uint8_t r;
  rtimer_clock_t now = rtimerx_now();
  if(RTIMER_CLOCK_LT(fixed_time, now)) {
    LOG_DBG("{%u|ep-%-4u} rt miss %s slot:%u %s | d:+%lu us e_ref:+%lu us t_ref:+%lu us\n",
      node_id, osf.epoch, OSF_ROUND_TO_STR(osf.round->type), osf.slot,
      msg, RTIMERTICKS_TO_USX(now - fixed_time),
      RTIMERTICKS_TO_USX(now - osf.t_epoch_ref), RTIMERTICKS_TO_USX(now - t_ref));
    r = RTIMER_ERR_TIME;
  } else {
    rtimerx_set(fixed_time, cb);
    r = RTIMER_OK;
  }
  return r;
}

/*---------------------------------------------------------------------------*/
void
rtimerx_clear()
{
  NRF_TIMERX->INTENCLR = TIMER_INTENCLR_COMPARE_SCHEDULE_REG_Msk;
}

/*---------------------------------------------------------------------------*/
void
TIMERX_IRQHandler(void)
{
  DEBUG_GPIO_ON(RADIO_IRQ_EVENT_PIN); // 1.5 - 66 us
  if(NRF_TIMERX->EVENTS_COMPARE[SCHEDULE_REG]) {
    // if(RTIMER_CLOCK_LT(NRF_TIMERX->CC[SCHEDULE_REG],RTIMERX_NOW())) {
      NRF_TIMERX->CC[SCHEDULE_REG] = 0UL;
      NRF_TIMERX->EVENTS_COMPARE[SCHEDULE_REG] = 0UL;
      NRF_TIMERX->INTENCLR = TIMER_INTENCLR_COMPARE_SCHEDULE_REG_Msk;
      cb_timer(NULL, NULL);
    // } else {
      // LOG_ERR("Timer was too late!\n");
    // }
  }
  DEBUG_GPIO_OFF(RADIO_IRQ_EVENT_PIN);
}
