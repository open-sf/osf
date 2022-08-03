 /**
  * \file
  *         D-Cube rpl-udp example (CSMA, TSCH, MCAST, BR).
  * \author
  *         Michael Baddeley <michael.g.baddeley@gmail.com>
  */

#include <stdlib.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/netstack.h"
#include "net/routing/routing.h"
#include "sys/node-id.h"
#include "dev/leds.h"

#include "net/ipv6/uip-ds6-route.h"

/* MUST INCLUDE THESE FOR NODE IDS AND TESTBED PATTERNS */
#include "services/deployment/deployment.h"
#if BUILD_WITH_TESTBED
/* Take sources/destinations from the testbed conf */
#include "services/testbed/testbed.h"
#endif /* BUILD_WITH_TESTBED*/

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#define LOG_N_BYTES 4

#define UDP_PORT	                  8765
#define READ_INTERVAL		            (CLOCK_SECOND/2)

const uip_ipaddr_t *default_prefix;
static uip_ipaddr_t ipaddr;
static struct etimer periodic_timer;
static struct simple_udp_connection udp_conn;

PROCESS(udp_process, "D-Cube RPL UDP Process");
AUTOSTART_PROCESSES(&udp_process);

/*---------------------------------------------------------------------------*/
#if MULTICAST
#if UIP_MCAST6_CONF_ENGINE != UIP_MCAST6_ENGINE_MPL
#include "net/ipv6/multicast/uip-mcast6.h"
static uip_ds6_maddr_t *
join_mcast_group(void)
{
  uip_ipaddr_t addr;
  uip_ds6_maddr_t *rv;
  const uip_ipaddr_t *default_prefix = uip_ds6_default_prefix();

  /* First, set our v6 global */
  uip_ip6addr_copy(&addr, default_prefix);
  uip_ds6_set_addr_iid(&addr, &uip_lladdr);
  uip_ds6_addr_add(&addr, 0, ADDR_AUTOCONF);

  /*
   * IPHC will use stateless multicast compression for this destination
   * (M=1, DAC=0), with 32 inline bits (1E 89 AB CD)
   */
  uip_ip6addr(&addr, 0xFF1E,0,0,0,0,0,0x89,0xABCD);
  rv = uip_ds6_maddr_add(&addr);

  if(rv) {
    LOG_INFO("Joined multicast group ");
    LOG_INFO_6ADDR(&uip_ds6_maddr_lookup(&addr)->ipaddr);
    LOG_INFO_("\n");
  }
  return rv;
}
#endif
#endif

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
#if MULTICAST
#if UIP_MCAST6_CONF_ENGINE == UIP_MCAST6_ENGINE_MPL
  uip_ip6addr(&ipaddr, 0xFF03,0,0,0,0,0,0,0xFC);
#else
  uip_ip6addr(&ipaddr, 0xFF1E,0,0,0,0,0,0x89,0xABCD);
#endif
  simple_udp_sendto(&udp_conn, data, len, &ipaddr);
  LOG_INFO("D: TX ");
  for(j = 0; j < LOG_N_BYTES; j++) {
    LOG_INFO_("%02x", data[j]);
  };
  LOG_INFO_(" l:%u s:%u d:", len, node_id);
  for(i = 0; i < n_dest-1; i++) {
    LOG_INFO_("%u,", dest[i]);
  };
  LOG_INFO_("%u\n", dest[n_dest-1]);
#else
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
#endif
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_process, ev, data)
{
  static uint8_t is_root;

  PROCESS_BEGIN();

  /* Initialise the testbed. If this returns 0 we have no node type assigned
     so exit Contiki */
  if(!testbed.init()) {
    LOG_ERR("Testbed could not init. Exiting Contiki!\n");
    exit(0);
  }
  /* Register a send function to send data on successful testbed read */
  tb_register_read_callback(&tesbed_callback);

  if(tb_get_pattern()->traffic_pattern == MP2P) {
    is_root = (tb_node_type == NODE_TYPE_DESTINATION);
    LOG_INFO("Root will be NODE_TYPE_DESTINATION\n");
  } else {
    is_root = (tb_node_type == NODE_TYPE_SOURCE);
    LOG_INFO("Root set through NODE_TYPE_SOURCE\n");
  }
  LOG_INFO("I AM ROOT... %u\n", is_root ? 1 : 0);


#if TESTBED_WITH_BORDER_ROUTER
  if(tb_node_is_br) {
    leds_on(LEDS_ALL);
  #if BORDER_ROUTER_CONF_WEBSERVER
    /* Initialise webserver */
    PROCESS_NAME(webserver_nogui_process);
    process_start(&webserver_nogui_process, NULL);
  #endif /* BORDER_ROUTER_CONF_WEBSERVER */
  }
#else
  if(is_root) {
    leds_on(LEDS_ALL);
    NETSTACK_ROUTING.root_start();
  }
#endif /* TESTBED_WITH_BORDER_ROUTER */

#if MAC_CONF_WITH_TSCH
  NETSTACK_MAC.on();
#endif

  /* Initialize UDP connection */
  if(tb_node_type == NODE_TYPE_DESTINATION) {
    LOG_INFO("Register UDP SERVER and wait for RPL DODAG to form\n");
#if MULTICAST
#if UIP_MCAST6_CONF_ENGINE != UIP_MCAST6_ENGINE_MPL
    if(join_mcast_group() == NULL) {
      LOG_INFO("Failed to join multicast group\n");
      PROCESS_EXIT();
    }
#endif
#endif
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
    /* Set ipaddr with DODAG ID, so we get the prefix */
    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&ipaddr)) {
      /* Send testbed data to DAG root */
      testbed.poll_read();
    } else {
      LOG_INFO("Not reachable yet\n");
    }
    etimer_set(&periodic_timer, READ_INTERVAL);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
