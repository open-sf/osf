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
#if BUILD_WITH_DEPLOYMENT
#include "services/deployment/deployment.h"
#endif
#include "services/osf-border-router/osf-border-router.h"

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

#if OSF_WITH_IPV6
/* Assign node_id act as Border Router, if == 255, all nodes are routers */
#if OSF_CONF_BR_NODE
uint8_t brn = OSF_CONF_BR_NODE;
#else
uint8_t brn = 1;
#endif
extern uint16_t node_id;
#define IS_NODE_BR()  ((brn == node_id))
/* Assign node_id for acceses to Global Internet */
#if OSF_CONF_BR_ISN
uint8_t isn = OSF_CONF_BR_ISN;
#else
uint8_t isn = 0;
#endif
/* Routes via ISN node */
#ifdef OSF_CONF_ROUTES
static uint8_t isn_routes[]={OSF_CONF_ROUTES};
#else
static uint8_t isn_routes[]={0x26,0x24,0x20,0x2A};
#endif

#endif /* OSF_WITH_IPV6 */

/*---------------------------------------------------------------------------*/
PROCESS(opensf_process, "OSF Example");
AUTOSTART_PROCESSES(&opensf_process);

/*---------------------------------------------------------------------------*/
/* OSF callback to receive data. */
/*---------------------------------------------------------------------------*/
#if BUILD_WITH_TESTBED
void
input_callback(uint8_t *data, uint8_t len)
{

  testbed.push(data, len);
  testbed.poll_write();
  LOG_INFO("RX l:%u: ", len);
  uint8_t i;
  for(i = 0; i < MIN(len,PRINT_DATA_LEN); i++) {
    LOG_INFO_("%02x ", data[i]);
  }
  LOG_INFO_("\n");
}
#endif /* BUILD_WITH_TESTBED */

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
    LOG_INFO("TX l:%u d:%u: ", len, dest[i]);
    for(j = 0; j < MIN(len,PRINT_DATA_LEN); j++) {
      LOG_INFO_("%02x ", data[j]);
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
/* IPV6 */
/*---------------------------------------------------------------------------*/
#if OSF_WITH_IPV6
static void
ipv6_init()
{
  LOG_INFO("Node ID: %d\r\n", node_id);
  LOG_INFO("Node is BR : %d\r\n", IS_NODE_BR() || (brn == OSF_BR_NODE_ALL));
  LOG_INFO("ISN node ID : %d\r\n", isn);

  if(!IS_NODE_BR()) {
    uint8_t i;
    uint8_t state;
    const uip_ipaddr_t *default_prefix;
    uip_ipaddr_t ipaddr;
    /* Give ourselves IPv6 addresses */
    default_prefix = uip_ds6_default_prefix();
    uip_ip6addr_copy(&ipaddr, default_prefix);
    uip_ds6_prefix_add(&ipaddr, UIP_DEFAULT_PREFIX_LEN, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
    LOG_INFO("IPv6 addresses:\n");
    for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
      state = uip_ds6_if.addr_list[i].state;
      if(uip_ds6_if.addr_list[i].isused &&
         (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
        LOG_INFO("-- ");
        LOG_INFO_6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
        LOG_INFO_("\n");
      }
    }
    if(brn != OSF_BR_NODE_ALL) {
      /* Add the BR as a default route */
      deployment_iid_from_id(&ipaddr, brn);
      /* uip_ipaddr_copy(&default_route, &ipaddr); */
      uip_ds6_defrt_add(&ipaddr, 0);
      LOG_INFO("Default route :");
      LOG_INFO_6ADDR(&ipaddr);
      LOG_INFO_("\r\n");
    } else {
      LOG_INFO("Node count : %d\r\n", deployment_node_count());
      uip_ipaddr_t ipaddr;
      uip_ipaddr_t prefix;
      uip_ipaddr_t next_hop;
      uip_lladdr_t lladdr;
      uint8_t prefix_len = 0;
      for(int i = 1; i <= deployment_node_count(); i++) {
        if(i != node_id) {
          memset(&ipaddr, 0x00, sizeof(uip_ipaddr_t));
          memset(&lladdr, 0x00, sizeof(uip_lladdr_t));
          deployment_iid_from_id(&ipaddr, i);
          ipaddr.u8[0] = UIP_DS6_DEFAULT_PREFIX_0;
          ipaddr.u8[1] = UIP_DS6_DEFAULT_PREFIX_1;
          /*uip_ds6_set_lladdr_from_iid(&lladdr, &next_hop); */
          deployment_lladdr_from_id((void *)&lladdr, i);
          if(NULL != uip_ds6_nbr_add(&ipaddr, &lladdr, 0, NBR_REACHABLE, NBR_TABLE_REASON_IPV6_ND_AUTOFILL, NULL)) {
            LOG_INFO("Autofill neighbor cache for host : ");
            LOG_INFO_6ADDR(&ipaddr);
            LOG_INFO_(", link-layer addr ");
            LOG_INFO_LLADDR((linkaddr_t *)&lladdr);
            LOG_INFO_("\n");
          }

          memset(&next_hop, 0x00, sizeof(uip_ipaddr_t));
          next_hop.u8[0] = UIP_DS6_DEFAULT_PREFIX_0;
          next_hop.u8[1] = i;
          next_hop.u8[15] = 0x1;
          uip_ds6_set_lladdr_from_iid(&lladdr, &next_hop);
          if(NULL != uip_ds6_nbr_add(&next_hop, &lladdr, 1, NBR_REACHABLE, NBR_TABLE_REASON_IPV6_ND_AUTOFILL, NULL)) {
            LOG_INFO("Autofill neighbor cache for host : ");
            LOG_INFO_6ADDR(&next_hop);
            LOG_INFO_(", link-layer addr ");
            LOG_INFO_LLADDR((linkaddr_t *)&lladdr);
            LOG_INFO_("\n");
          }

          memset(&prefix, 0x00, sizeof(uip_ipaddr_t));
          memset(&next_hop, 0x00, sizeof(uip_ipaddr_t));
          prefix_len = 64;
          prefix.u8[0] = UIP_DS6_DEFAULT_PREFIX_0;
          prefix.u8[1] = i;
          memcpy(&next_hop, &ipaddr, sizeof(uip_ipaddr_t));
          next_hop.u8[0] = UIP_DS6_DEFAULT_PREFIX_0;
          next_hop.u8[1] = UIP_DS6_DEFAULT_PREFIX_1;
          if(NULL != uip_ds6_route_add(&prefix, prefix_len, &next_hop)) {
            LOG_INFO("Added a route to ");
            LOG_INFO_6ADDR(&prefix);
            LOG_INFO_("/%d via ", prefix_len);
            LOG_INFO_6ADDR(&next_hop);
            LOG_INFO_("\r\n");
          }
          LOG_INFO("\r\n");
        }
      }
    }
  }

  /* Add route to Global Internet via ISN node */
  if(isn > 0) {
    for(uint8_t i=0; i<(sizeof(isn_routes)/sizeof(uint8_t)); i++) {
      uip_ipaddr_t next_hop;
      uip_lladdr_t lladdr;
      uip_ipaddr_t prefix;
      uint8_t prefix_len = 8; /* 64 */
      memset(&prefix, 0x00, sizeof(uip_ipaddr_t));

      prefix.u8[0] = isn_routes[i]; //0x26; /* 26 */
      prefix.u8[1] = 0x00;
      prefix.u8[15] = 0x0;
      uip_ds6_set_lladdr_from_iid(&lladdr, &prefix);
      if(NULL != uip_ds6_nbr_add(&prefix, &lladdr, 0, NBR_REACHABLE, NBR_TABLE_REASON_IPV6_ND_AUTOFILL, NULL)) {
        LOG_INFO("Autofill neighbor cache for host : ");
        LOG_INFO_6ADDR(&prefix);
        LOG_INFO_(", link-layer addr ");
        LOG_INFO_LLADDR((linkaddr_t *)&lladdr);
        LOG_INFO_("\n");
      }

      memset(&next_hop, 0x00, sizeof(uip_ipaddr_t));
      deployment_iid_from_id(&next_hop, isn);
      next_hop.u8[0] = UIP_DS6_DEFAULT_PREFIX_0;
      next_hop.u8[1] = UIP_DS6_DEFAULT_PREFIX_1;
      deployment_lladdr_from_id((void *)&lladdr, isn);

      if(NULL != uip_ds6_route_add(&prefix, prefix_len, &next_hop)) {
        LOG_INFO("Added a route to ");
        LOG_INFO_6ADDR(&prefix);
        LOG_INFO_("/%d via ", prefix_len);
        LOG_INFO_6ADDR(&next_hop);
        LOG_INFO_("\r\n");
      }
      LOG_INFO("\r\n");
    }
  }
}
#endif /* OSF_WITH_IPV6 */

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(opensf_process, ev, data)
{

  PROCESS_BEGIN();

  LOG_INFO("Starting...\n");

  /* Init ICACHE */
#ifdef NVMC_ICACHECNF_CACHEEN_Msk
  nrf_nvmc_icache_config_set(NRF_NVMC, NRF_NVMC_ICACHE_ENABLE);
#endif // NVMC_ICACHECNF_CACHEEN_Msk

#if BUILD_WITH_TESTBED
  LOG_INFO("Init TESTBED application...\n");
  testbed_init();
  volatile tb_pattern_t *p = tb_get_pattern();
  osf_configure((uint8_t *)p->source_id, tb_get_n_src(),
                (uint8_t *)p->destination_id, tb_get_n_dest(),
                NULL, 0);
#elif NRF52840_NATIVE_USB
  LOG_INFO("Init USB serial application ...\n");
#endif

#if BUILD_WITH_TESTBED
  /* Register a callback so we can receive from OSF */
  osf_register_input_callback(input_callback);
#endif

#if LEDS
  /* Use LEDs for indicate transfer status */
  extern void osf_leds_init(void);
  extern void leds_all_on(void);
  osf_leds_init();
  leds_all_on();
  static struct etimer timer_etimer;
  etimer_set(&timer_etimer, CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer_etimer));
  etimer_stop(&timer_etimer);
  osf_leds_init();
#endif

  /* Start OSF (will have been initialised in netstack.c) */
  NETSTACK_MAC.on();

#if OSF_WITH_IPV6
  /* Initialise static ip configuration (addresses, routes etc)
     FIXME: Need to have a very good think about how IP works in OSF. */
  ipv6_init();
#if USE_UDP_SERVER == 1
  PROCESS_NAME(udp_server_process);
  process_start(&udp_server_process, NULL);
#if USE_UDP_CLIENT == 1
  PROCESS_NAME(udp_client_process);
  process_start(&udp_client_process, NULL);
#endif
#endif /* USE_UDP_SERVER == 1 */

#if USE_TCP_SERVER == 1
  PROCESS_NAME(tcp_server_process);
  process_start(&tcp_server_process, NULL);
#endif /* USE_UDP_SERVER == 1 */

#if USE_WEB_SERVER == 1
  PROCESS_NAME(webserver_nogui_process);
  process_start(&webserver_nogui_process, NULL);
#endif /* USE_WEBSERVER == 1 */

#if USE_UDP_CONTROL == 1
  PROCESS_NAME(udp_control_process);
  process_start(&udp_control_process, NULL);
#endif /* USE_UDP_CONTROL == 1 */
#endif /* OSF_WITH_IPV6 */

  while(1) {
    
    /* Do nothing */
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
