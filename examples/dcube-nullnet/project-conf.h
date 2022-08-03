#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/*---------------------------------------------------------------------------*/
/* Netstack */
/*---------------------------------------------------------------------------*/
#define UIP_CONF_UDP_CONNS                         8 // dcube max src/dest
#define NETSTACK_MAX_ROUTE_ENTRIES                 8 // dcube max nodes
#define QUEUEBUF_CONF_NUM                          8 // dcube ~max dense area
#define NBR_TABLE_CONF_MAX_NEIGHBORS               8 // dcube ~max dense area

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
#define RPL_MRHOF_CONF_MAX_LINK_METRIC             4096 // dflt is 512
#define RPL_CONF_DAO_MAX_RETRANSMISSIONS           10

/*---------------------------------------------------------------------------*/
/* Testbed */
/*---------------------------------------------------------------------------*/

#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_NONE
#define TSCH_LOG_CONF_PER_SLOT                     0

#endif /* PROJECT_CONF_H_ */
