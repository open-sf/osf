/**
 * \file
 *         D-Cube NullNet broadcast example
 * \author
*          Michael Baddeley <michael.g.baddeley@gmail.com>
 *
 */

#include "contiki.h"
#include "net/netstack.h"

#if NETSTACK_CONF_WITH_NULLNET
#include "net/nullnet/nullnet.h"
#endif

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

/* Configuration */
#define SEND_INTERVAL (CLOCK_SECOND/2)

static unsigned txcount = 0;

/*---------------------------------------------------------------------------*/
PROCESS(dcube_example_process, "D-Cube Broadcast Example");
AUTOSTART_PROCESSES(&dcube_example_process);

/*---------------------------------------------------------------------------*/
#if BUILD_WITH_TESTBED
static void
tesbed_callback(uint8_t *data, uint16_t len, uint8_t *dest, uint8_t n_dest)
{
  uint8_t i, j;
  LOG_INFO("D: TX ");
  for(j = 0; j < len; j++) {
    LOG_INFO_("%02x", ((uint8_t *)data)[j]);
  };
  LOG_INFO_(" l:%u s:%u d:", len, node_id);
  for(i = 0; i < n_dest-1; i++) {
    LOG_INFO_("%u,", dest[i]);
  };
  LOG_INFO_("%u\n", dest[n_dest-1]);
  memcpy(nullnet_buf, data, len);
  nullnet_len = len;
  NETSTACK_NETWORK.output(NULL);
}
#else
/*---------------------------------------------------------------------------*/
static void
send()
{
  LOG_INFO("Sending %u to ", txcount);
  LOG_INFO_LLADDR(NULL);
  LOG_INFO_("\n");

  memcpy(nullnet_buf, &txcount, sizeof(txcount));
  nullnet_len = sizeof(txcount);

  NETSTACK_NETWORK.output(NULL);
  txcount++;
}
#endif

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len) {
#if BUILD_WITH_TESTBED
    uint8_t i;
    if(testbed.push((uint8_t *)data, len)) {
      testbed.poll_write();
      LOG_INFO("D: RX ");
      for(i = 0; i < len; i++) {
        LOG_INFO_("%02x", ((uint8_t *)data)[i]);
      };
      LOG_INFO_(" l:%u s:%-3u d:%-3u\n", len, deployment_id_from_lladdr(src), node_id);
    } else {
      LOG_ERR("Could not push to testbed!\n");
    }
#else
    unsigned rxcount;
    memcpy(&rxcount, data, sizeof(rxcount));
    LOG_INFO("Received %u from ", rxcount);
    LOG_INFO_LLADDR(src);
    LOG_INFO_(" (id=%u)\n", deployment_id_from_lladdr(src));
#endif /* BUILD_WITH_TESTBED */
  } else {
    LOG_ERR("Unknown\n");
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dcube_example_process, ev, data)
{
  static struct etimer periodic_timer;

  PROCESS_BEGIN();

  LOG_INFO("Starting...\n");

#if BUILD_WITH_TESTBED
  /* Initialise the testbed. If this returns 0 we have no node type assigned. */
  testbed.init();
  /* Register a send function to send data on successful testbed read */
  tb_register_read_callback(&tesbed_callback);
#endif

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&txcount;
  nullnet_len = sizeof(txcount);
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    /* Send every timer */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
#if BUILD_WITH_TESTBED
    testbed.poll_read();
#else
    send();
#endif
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
