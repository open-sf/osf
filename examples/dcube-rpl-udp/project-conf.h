#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define XSTR(x) STR(x)
#define STR(x) #x
// #pragma message "max_hops: " XSTR(ATM_CONF_PHASE_MAX_HOPS)

/*---------------------------------------------------------------------------*/
/* Netstack */
/*---------------------------------------------------------------------------*/
#define UIP_CONF_UDP_CONNS                          60 // dcube max src/dest
#define NETSTACK_MAX_ROUTE_ENTRIES                  60 // dcube max nodes
#define QUEUEBUF_CONF_NUM                           32 // dcube ~max dense area
#define NBR_TABLE_CONF_MAX_NEIGHBORS                32 // dcube ~max dense area

/*---------------------------------------------------------------------------*/
/* Multicast */
/*---------------------------------------------------------------------------*/
#include "net/ipv6/multicast/uip-mcast6-engines.h"
#ifdef MULTICAST
/* Change this to switch engines. Engine codes in uip-mcast6-engines.h */
// #define UIP_MCAST6_CONF_ENGINE                      UIP_MCAST6_ENGINE_SMRF
// #define UIP_MCAST6_CONF_ENGINE                      UIP_MCAST6_ENGINE_ROLL_TM
// #define UIP_MCAST6_CONF_ENGINE                      UIP_MCAST6_ENGINE_ESMRF
#define UIP_MCAST6_CONF_ENGINE                      UIP_MCAST6_ENGINE_MPL
/* For Imin: Use 16 over CSMA, 64 over Contiki MAC */
#define ROLL_TM_CONF_IMIN_1                         16
#define MPL_CONF_DATA_MESSAGE_IMIN                  16
#define MPL_CONF_CONTROL_MESSAGE_IMIN               16
#define UIP_MCAST6_ROUTE_CONF_ROUTES                1
#else
#define UIP_MCAST6_CONF_ENGINE                      UIP_MCAST6_ENGINE_NONE
#endif /* MULTICAST */
/*---------------------------------------------------------------------------*/
/* IEEE 802.15.4 */
/*---------------------------------------------------------------------------*/
#define IEEE802154_CONF_PANID                      0x1337
#define IEEE802154_CONF_DEFAULT_CHANNEL            15

/*---------------------------------------------------------------------------*/
/* CSMA */
/*---------------------------------------------------------------------------*/
#define CSMA_CONF_MAX_NEIGHBOR_QUEUES              NBR_TABLE_CONF_MAX_NEIGHBORS
#define CSMA_CONF_MAX_PACKET_PER_NEIGHBOR          QUEUEBUF_CONF_NUM

/*---------------------------------------------------------------------------*/
/* RPL */
/*---------------------------------------------------------------------------*/
// #define TB_CONF_CUSTOM_CONFIG                      12, 4096, 384
#define RPL_MRHOF_CONF_MAX_LINK_METRIC             4096 // dflt is 512
#define RPL_CONF_DAO_MAX_RETRANSMISSIONS           10

/*---------------------------------------------------------------------------*/
/* TSCH */
/*---------------------------------------------------------------------------*/
/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
#define TSCH_CONF_AUTOSTART                        0

#ifdef CONF_CH
/* Partial channel hopping */
#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE         TSCH_HOPPING_SEQUENCE_4_4
/* Full channel hopping */
// #define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE        TSCH_HOPPING_SEQUENCE_16_16
#else
#define TSCH_CONF_DEFAULT_HOPPING_SEQUENCE         TSCH_HOPPING_SEQUENCE_1_1
#endif

#ifdef CONF_6MIN_SF
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH          CONF_6MIN_SF
#else
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH          1
#endif

#define TSCH_CONF_ADAPTIVE_TIMESYNC                1

/*---------------------------------------------------------------------------*/
/* Testbed */
/*---------------------------------------------------------------------------*/
#define TB_CONF_NULLTB_PERIOD                      1
// #define TB_CONF_NULLTB_PERIOD_MIN                 1
// #define TB_CONF_NULLTB_PERIOD_MAX                 2
#define TB_CONF_NULLTB_DATA_LEN                    8

#if TESTBED_DEBUG
  /* NB: Only for atomic! */
  #define DEBUG_LEDS                               1
  #define DEBUG_GPIO                               1
  #define SHOW_CHANNEL_GPIO                        0 // 30
  #define SHOW_EVENTS_END_GPIO                     1 // 30
  #define SHOW_EVENTS_READY_GPIO                   0 // 30
  #define SHOW_EVENTS_ADDRESS_GPIO                 1 // 29
  #define SHOW_CH_TIMEOUTS_GPIO                    1 // 29
  #define SHOW_RX_TIMEOUTS_GPIO                    0 // 30
#endif


/*---------------------------------------------------------------------------*/
/* Border Router */
/*---------------------------------------------------------------------------*/
#if TESTBED_WITH_BORDER_ROUTER
#ifndef WEBSERVER_CONF_CFS_CONNS
#define WEBSERVER_CONF_CFS_CONNS                   2
#endif
#ifndef BORDER_ROUTER_CONF_WEBSERVER
#define BORDER_ROUTER_CONF_WEBSERVER               1
#endif
#if BORDER_ROUTER_CONF_WEBSERVER
#define UIP_CONF_TCP                               1
#endif
#endif /* TESTBED_WITH_BORDER_ROUTER */

/*---------------------------------------------------------------------------*/
/* Contiki Logging */
/*---------------------------------------------------------------------------*/
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#define TSCH_LOG_CONF_PER_SLOT                     0

#endif /* PROJECT_CONF_H_ */
