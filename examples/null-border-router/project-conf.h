#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/*---------------------------------------------------------------------------*/
/* Netstack */
/*---------------------------------------------------------------------------*/
#define UIP_CONF_UDP_CONNS                         60 // dcube max src/dest
// #define NETSTACK_MAX_ROUTE_ENTRIES                 60 // dcube max nodes. no routing in nullrouting, so not needed.
#define QUEUEBUF_CONF_NUM                          32 // dcube ~max dense area
#define NBR_TABLE_CONF_MAX_NEIGHBORS               32 // dcube ~max dense area

// #define UIP_CONF_ROUTER                            1 // all contiki nodes are routers by default
#define UIP_CONF_ND6_SEND_RA                       1
// #define UIP_CONF_ND6_SEND_RS                       1 // all contiki nodes are routers, so we don't send RS... (UIP_CONF_ROUTER)
#define UIP_CONF_ND6_SEND_NS                       1
#define UIP_CONF_ND6_SEND_NA                       1
#define UIP_DS6_CONF_PERIOD                       (5 * CLOCK_SECOND)
// #define UIP_CONF_ND6_AUTOFILL_NBR_CACHE            1 // 0 by default

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
/* Logging */
/*---------------------------------------------------------------------------*/
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#define TSCH_LOG_CONF_PER_SLOT                     0

#endif /* PROJECT_CONF_H_ */
