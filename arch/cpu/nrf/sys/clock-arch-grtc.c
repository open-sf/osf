/*
 * clock-arch.c - Contiki-NG clock implementation for nRF54L15 (Application CPU, Cortex-M33)
 * Uses NRFX GRTC driver for system tick.
 */

#include "contiki.h"
#include "sys/clock.h"
#include <nrf.h>
#include <stdint.h>
#include <stdbool.h>
#include <nrfx_grtc.h>

/*---------------------------------------------------------------------------*/
#define CLOCK_TICK_HZ     CLOCK_SECOND
static volatile clock_time_t tick_count = 0;
static volatile unsigned long second_count = 0;

static nrfx_grtc_channel_t grtc_channel = {
  .channel = 0,
  .handler = NULL,
  .p_context = NULL
};

static void grtc_cc_handler(int32_t id, uint64_t cc_value, void * p_context)
{
  (void)id;
  (void)cc_value;
  (void)p_context;

  tick_count++;
  if(tick_count % CLOCK_SECOND == 0) {
    second_count++;
  }

  uint64_t now;
  nrfx_grtc_syscounter_get(&now);
  nrfx_grtc_syscounter_cc_absolute_set(&grtc_channel, now + (1000000 / CLOCK_TICK_HZ), true);
  process_poll(&etimer_process);
}

/*---------------------------------------------------------------------------*/
void
clock_init(void)
{
  nrfx_err_t err = nrfx_grtc_init(0);
  if (err != NRFX_SUCCESS && err != NRFX_ERROR_ALREADY) {
    return;
  }

  if (!nrfx_grtc_ready_check()) {
    uint8_t chan;
    err = nrfx_grtc_syscounter_start(true, &chan);
    if (err != NRFX_SUCCESS) {
      return;
    }
  }

  grtc_channel.handler = grtc_cc_handler;
  grtc_channel.p_context = NULL;
  grtc_channel.channel = 0;

  err = nrfx_grtc_channel_alloc(&grtc_channel.channel);
  if (err != NRFX_SUCCESS) {
    return;
  }

  nrfx_grtc_channel_callback_set(grtc_channel.channel, grtc_cc_handler, NULL);

  uint64_t now;
  nrfx_grtc_syscounter_get(&now);
  nrfx_grtc_syscounter_cc_absolute_set(&grtc_channel, now + (1000000 / CLOCK_TICK_HZ), true);
}

/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return tick_count;
}

/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  return second_count;
}

/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
  second_count = sec;
}

/*---------------------------------------------------------------------------*/
void
clock_wait(clock_time_t t)
{
  clock_time_t start = clock_time();
  while(clock_time() - start < t);
}

/*---------------------------------------------------------------------------*/
void
clock_delay_usec(uint16_t dt)
{
  uint32_t cycles = SystemCoreClock / 1000000 * dt / 5;
  while(cycles--) {
    __NOP();
  }
}

/*---------------------------------------------------------------------------*/
void
clock_delay_msec(uint32_t volatile dt)
{
  while(dt--) {
    clock_delay_usec(1000);
  }
}

/*---------------------------------------------------------------------------*/
int
clock_fine_max(void)
{
  return 0xffff;
}

/*---------------------------------------------------------------------------*/
unsigned short
clock_fine(void)
{
  uint64_t now;
  nrfx_grtc_syscounter_get(&now);
  return (unsigned short)(now & 0xffff);
}

/*---------------------------------------------------------------------------*/
void
clock_delay(unsigned int delay)
{
  clock_delay_usec(delay);
}
/*---------------------------------------------------------------------------*/
