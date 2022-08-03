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
 
#ifndef OSF_TIMER_H_
#define OSF_TIMER_H_

/* TimerX is used for timing whole Radio activity. 1 tick - 62.5 ns */
#define NRF_TIMERX                  NRF_TIMER2
#define TIMERX_IRQHandler           TIMER2_IRQHandler
#define TIMERX_IRQn                 TIMER2_IRQn
#define RTIMERX_NOW()               rtimerx_now()

/* Defines for the capture/compare registers in TimerX */
#define TIMESTAMP_REG         1
#define SCHEDULE_REG          2
#define NOW_REG               3

#define TIMER_INTENSET_COMPARE_SCHEDULE_REG_Msk TIMER_INTENSET_COMPARE2_Msk // NB: MUST MATCH SCHEDULE_REG!
#define TIMER_INTENCLR_COMPARE_SCHEDULE_REG_Msk TIMER_INTENCLR_COMPARE2_Msk // NB: MUST MATCH SCHEDULE_REG!

/* Keep in sync prescaller and second definition */
#define RTIMERX_SECOND      (16000000UL)
#define RTIMERX_MILLISECOND (RTIMERX_SECOND/1000)
#define RTIMERX_MICROSECOND (RTIMERX_MILLISECOND/1000)

/* Macro for convert microseconds to timer ticks */
#define US_TO_RTIMERTICKSX(US)  ((US) >= 0 ? \
                                 (((uint64_t)(US)*(RTIMERX_SECOND)+500000) / 1000000L) : \
                                 ((uint64_t)(US)*(RTIMERX_SECOND)-500000) / 1000000L)

/* Macroses for convert timer ticks to milliseconds, microseconds and nanoseconds.
   Use for rough validation of timing values only !
   1 tick - 62.5 ns */
#define RTIMERTICKS_TO_MSX(T)  ((uint32_t)(((uint64_t)(T) * 1000UL + ((RTIMERX_SECOND) / 2)) / (RTIMERX_SECOND)))
#define RTIMERTICKS_TO_USX(T)  ((uint32_t)(((uint64_t)(T) * 1000000UL + ((RTIMERX_SECOND) / 2)) / (RTIMERX_SECOND)))
#define RTIMERTICKS_TO_NSX(T)  ((uint32_t)(((uint64_t)(T) * 1000000000UL + ((RTIMERX_SECOND) / 2)) / (RTIMERX_SECOND)))

/*---------------------------------------------------------------------------*/
/* Polling loops.
 * WDT update period need to be taken in account when tight loops are in use
 */
#define RTIMERX_BUSYWAIT_UNTIL(cond, max_time) \
  do { \
    volatile rtimer_clock_t t0; \
    t0 = RTIMERX_NOW(); \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMERX_NOW(), t0 + (max_time))) \
    {; } \
  } while(0);

#define RTIMERX_BUSYWAIT_UNTIL_ABS(cond, t0) \
  do { \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMERX_NOW(), t0)) \
    {; } \
  } while(0);

/*---------------------------------------------------------------------------*/
void           rtimerx_init();
rtimer_clock_t rtimerx_now();
void           rtimerx_set(rtimer_clock_t value, rtimer_callback_t cb);
uint8_t        rtimerx_set_fixed(rtimer_clock_t fixed_time, rtimer_callback_t cb, char* msg);
void           rtimerx_clear();

#endif /* OSF_TIMER_H_ */
