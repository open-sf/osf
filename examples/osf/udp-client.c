#include "contiki.h"
#include "contiki-net.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "services/deployment/deployment.h"

#include "sys/log.h"
#define LOG_MODULE "UDPC"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define SEND_INTERVAL   (7 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;
static unsigned udp_send = 0;
static unsigned udp_recv = 0;


#if DL
static uint16_t dlen = DL;
#else
static uint16_t dlen = 16;
#endif

#if DS
static uint16_t server_node_id = DS;
#else
static uint16_t server_node_id = 3;
#endif

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");

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
  udp_recv++;
  LOG_INFO("Received response %u ( %u ) '%.*s' from ", \
    udp_recv, udp_send, datalen, (char *)data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count;
  static char str[UIP_BUFSIZE + 128];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();

  /* Initialize UDP connection */
  (void)simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
              UIP_HTONS(0), udp_rx_callback);                                                                  

  LOG_INFO("Register UDP client, port %d\r\n", UDP_CLIENT_PORT);

  /* 1s delay */
  etimer_set(&periodic_timer, CLOCK_SECOND);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  etimer_stop(&periodic_timer);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable()/* && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)*/) {
       
        memset(&dest_ipaddr, 0x00, sizeof(uip_ipaddr_t));
        deployment_iid_from_id(&dest_ipaddr, server_node_id);
        dest_ipaddr.u8[0] = UIP_DS6_DEFAULT_PREFIX_0;
        dest_ipaddr.u8[1] = UIP_DS6_DEFAULT_PREFIX_1;
 
        if(node_id != server_node_id) {
          /* Send to DAG root */
          LOG_INFO("Sending request %u to ", count);
          LOG_INFO_6ADDR(&dest_ipaddr);
          LOG_INFO_("\n");
          memset(str, 0x00, sizeof(str));
          snprintf(str, 16, "hello %d", count);
          simple_udp_sendto_port(&udp_conn, str, dlen, &dest_ipaddr, UDP_SERVER_PORT);
          count++;
          udp_send++;
        }
      } else {
        LOG_INFO("Not reachable yet\n");
      }

    /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL
               - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
