/**
 * \file
 *         D-Cube NullNet broadcast example
 * \author
*          Michael Baddeley <michael.g.baddeley@gmail.com>
 *
 */

#include <stdlib.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/netstack.h"
#include "sys/node-id.h"
#include "dev/leds.h"

#include "net/ipv6/uip-ds6-route.h"


/* MUST INCLUDE THESE FOR NODE IDS AND TESTBED PATTERNS */
#include "services/deployment/deployment.h"
#if BUILD_WITH_TESTBED
/* Take sources/destinations from the testbed conf */
#include "services/testbed/testbed.h"
#endif /* BUILD_WITH_TESTBED*/

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#define LOG_N_BYTES 4

#define UDP_PORT	                  8765
#define READ_INTERVAL		            (CLOCK_SECOND/2)

const uip_ipaddr_t *default_prefix;
// static const uip_ipaddr_t default_route;
static uip_ipaddr_t ipaddr;
static struct etimer periodic_timer;
static struct simple_udp_connection udp_conn;

/*---------------------------------------------------------------------------*/
PROCESS(dcube_null_udp_process, "D-Cube NullRouting UDP Example");
AUTOSTART_PROCESSES(&dcube_null_udp_process);

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  uint8_t i;
  leds_on(LEDS_ALL);
  if(testbed.push((uint8_t *)data, datalen)) {
    testbed.poll_write();
    LOG_INFO("D: RX ");
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_(" ");
    for(i = 0; i < LOG_N_BYTES; i++) {
      LOG_INFO_("%02x", data[i]);
    };
    LOG_INFO_(" l:%u s:%u d:%u\n", datalen, deployment_id_from_iid(sender_addr), node_id);
  } else {
    LOG_ERR("Could not write to testbed!\n");
  }
  leds_off(LEDS_ALL);
}

/*---------------------------------------------------------------------------*/
static void
tesbed_callback(uint8_t *data, uint16_t len, uint8_t *dest, uint8_t n_dest)
{
  uint8_t i, j;
  if(len > 0xFF) {
    LOG_WARN("E2 data len is > uint8_t max!");
  }
  /* Send to each of the destinations */
  for(i = 0; i < n_dest; i++) {
    default_prefix = uip_ds6_default_prefix();
    uip_ip6addr_copy(&ipaddr, default_prefix);
    deployment_iid_from_id(&ipaddr, dest[i]);
    LOG_INFO("D: TX ");
    LOG_INFO_6ADDR(&ipaddr);
    LOG_INFO_(" ");
    for(j = 0; j < LOG_N_BYTES; j++) {
      LOG_INFO_("%02x", data[j]);
    };
    LOG_INFO_(" l:%u s:%u d:%u\n", len, node_id, dest[i]);
    simple_udp_sendto(&udp_conn, data, len, &ipaddr);
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dcube_null_udp_process, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("Starting...\n");

  /* Initialise the testbed. If this returns 0 we have no node type assigned
     so exit Contiki */
  if(!testbed.init()) {
    LOG_ERR("Testbed could not init. Exiting Contiki!\n");
    exit(0);
  }
  /* Register a send function to send data on successful testbed read */
  tb_register_read_callback(&tesbed_callback);

#if TESTBED_WITH_BORDER_ROUTER
  if(tb_node_is_br) {
    leds_on(LEDS_ALL);
  #if BORDER_ROUTER_CONF_WEBSERVER
    /* Initialise webserver */
    PROCESS_NAME(webserver_nogui_process);
    process_start(&webserver_nogui_process, NULL);
  #endif /* BORDER_ROUTER_CONF_WEBSERVER */
  } else {
    uint8_t i;
    uint8_t state;
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
    /* Add the BR (from the testbed) as a default route */
    uint8_t *brs = tb_get_brs();
    deployment_iid_from_id(&ipaddr, brs[0]);
    // uip_ipaddr_copy(&default_route, &ipaddr);
    uip_ds6_defrt_add(&ipaddr,0);
    LOG_INFO("Default route:");
    LOG_INFO_6ADDR(&ipaddr);
    LOG_INFO_("\n");
  }
#endif

  /* Initialize UDP connection */
  if(tb_node_type == NODE_TYPE_DESTINATION) {
    LOG_INFO("Register UDP SERVER\n");
    simple_udp_register(&udp_conn, UIP_HTONS(UDP_PORT), NULL,
      UIP_HTONS(0), udp_rx_callback);
  } else {
    LOG_INFO("Register UDP CLIENT\n");
    simple_udp_register(&udp_conn, UIP_HTONS(0), NULL,
       UIP_HTONS(UDP_PORT), udp_rx_callback);
    etimer_set(&periodic_timer, READ_INTERVAL);
  }

  while(tb_node_type == NODE_TYPE_SOURCE) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    testbed.poll_read();
    etimer_set(&periodic_timer, READ_INTERVAL);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
