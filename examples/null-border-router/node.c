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

/*---------------------------------------------------------------------------*/
PROCESS(null_border_router_process, "NULL Border Router Example");
AUTOSTART_PROCESSES(&null_border_router_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(null_border_router_process, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("Starting...\n");

#if BORDER_ROUTER_CONF_WEBSERVER
  /* Initialise webserver */
  PROCESS_NAME(webserver_nogui_process);
  process_start(&webserver_nogui_process, NULL);
#endif /* BORDER_ROUTER_CONF_WEBSERVER */

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
