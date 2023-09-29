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
 *         Open Synchronous Flooding (OSF) example
 * \author
 *         Michael Baddeley <michael@ssrc.tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 *
 */

#include "contiki.h"
#include "contiki-net.h"
#include "net/netstack.h"
/* #include "net/mac/osf/osf-packet.h" */
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-proto.h"
#include "net/mac/osf/osf-debug.h"

#include "sys/node-id.h"

#include "services/deployment/deployment.h"

#if BUILD_WITH_TESTBED
/* Take sources/destinations from the testbed conf */
#include "services/testbed/testbed.h"
#endif /* BUILD_WITH_TESTBED*/
#include <nrf_nvmc.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define PRINT_DATA_LEN (8)

#if HELLO_WORLD
static struct etimer timer;
static uint8_t count = 0;
/* Initialise a receive buffer. Again, testbed has it's own version of this. */
static char data_buf[64];
/* Create dummy sources/destinations here from the makeargs */
#define SOURCES(...)         static uint8_t sources[] = { __VA_ARGS__ };
#define N_SOURCES            sizeof(sources) / sizeof(uint8_t)
#define DESTINATIONS(...)    static uint8_t destinations[] = { __VA_ARGS__ };
#define N_DESTINATIONS       sizeof(destinations) / sizeof(uint8_t)
/* Actually initialise the sources and destinations */
SOURCES(TB_CONF_SOURCES);
DESTINATIONS(TB_CONF_DESTINATIONS)
#endif /* HELLO_WORLD */

/*---------------------------------------------------------------------------*/
PROCESS(opensf_process, "OSF Example");
AUTOSTART_PROCESSES(&opensf_process);

/*---------------------------------------------------------------------------*/
/* OSF callback to receive data. */
/*---------------------------------------------------------------------------*/
void
input_callback(uint8_t *data, uint8_t len)
{
#if HELLO_WORLD
  LOG_INFO("RX: %s\n", data);
#elif BUILD_WITH_TESTBED
  testbed.push(data, len);
  testbed.poll_write();
  // LOG_INFO("RX: ");
  // uint8_t i;
  // for(i = 0; i < MIN(len,PRINT_DATA_LEN); i++) {
  //   LOG_INFO_("%02x ", data[i]);
  // }
  // LOG_INFO_("\n");
#endif
}
/*---------------------------------------------------------------------------*/
/* Send data with HELLO_WORLD */
/*---------------------------------------------------------------------------*/
#if HELLO_WORLD
static void
hello_world_init()
{
  uint8_t i;
  uint8_t is_source = 0;
  for(i = 0; i < sizeof(sources); i++) {
    if(node_id == sources[i]) {
      is_source = 1;
      LOG_INFO("I am a SOURCE node :)\n");
    }
  }
  if(is_source) {
    /* Setup a periodic send timer. */
    etimer_set(&timer, HELLO_WORLD_PERIOD);
  }
}
/*---------------------------------------------------------------------------*/
static void
hello_world_send()
{
  uint8_t i;
  for(i = 0; i < sizeof(destinations); i++) {
    snprintf(data_buf, sizeof(data_buf), "TX: hello %d from %u", count, node_id);
    count++;
    LOG_INFO("%s\n", data_buf);
    osf_send((uint8_t *)data_buf, sizeof(data_buf), destinations[i]);
  }
}
#endif /* HELLO_WORLD */

/*---------------------------------------------------------------------------*/
/* Send data with TESTBED service (os/services/testbed) */
/*---------------------------------------------------------------------------*/
#if BUILD_WITH_TESTBED
static void
tesbed_callback(uint8_t *data, uint16_t len, uint8_t *dest, uint8_t n_dest)
{
  /* TODO: need to introduce the concept of destinations into OSF */
  if(len > 0xFF) {
    LOG_WARN("E2 data len is > uint8_t max!");
  }
  /* Send to each dest one by one */
  uint8_t i,j;
  for (i = 0; i < n_dest; i++) {
    osf_send(data, len, dest[i]);
    LOG_INFO("TX d:%u: ", dest[i]);
    for(j = 0; j < MIN(len,PRINT_DATA_LEN); j++) {
      // LOG_INFO_("%02x ", data[j]);
      LOG_INFO_("%u ", data[j]);
    }
    LOG_INFO_("\n");
  }
}

/*---------------------------------------------------------------------------*/
static void
testbed_init()
{
  /* Init the testbed eeprom processes */
  testbed.init();
  /* Register a send function to send data on successful testbed read */
  tb_register_read_callback(&tesbed_callback);
}
#endif /* BUILD_WITH_TESTBED */

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(opensf_process, ev, data)
{

  PROCESS_BEGIN();

  LOG_INFO("Starting...\n");

#if HELLO_WORLD
  /* Initialize HELLO_WORLD */
  LOG_INFO("Init HELLO_WORLD application...\n");
  hello_world_init();
#elif BUILD_WITH_TESTBED
  LOG_INFO("Init TESTBED application...\n");
  testbed_init();
  if (tb_node_type == NODE_TYPE_NONE){
    LOG_ERR("Node type not set!\n");
    PROCESS_EXIT();
  }
  volatile tb_pattern_t *p = tb_get_pattern();
  osf_configure((uint8_t *)p->source_id, tb_get_n_src(),
                (uint8_t *)p->destination_id, tb_get_n_dest(),
                NULL, 0);
#elif NRF52840_NATIVE_USB
  LOG_INFO("Init USB serial application ...\n");
#else
  #error "ERROR: You aren't using HELLO_WORLD || BUILD_WITH_TESTBED || NRF52850_NATIVE_USB ... What data are you tring to send?"
#endif

  /* Register a callback so we can receive from OSF */
  osf_register_input_callback(input_callback);

  /* Start OSF (will have been initialised in netstack.c) */
  NETSTACK_MAC.on();

  while(1) {
#if HELLO_WORLD
    /* If we are a source, wait for the periodic timer to expire and then
       restart the timer. */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
    hello_world_send();
#else
    /* Do nothing */
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
#endif /* HELLO_WORLD */
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
