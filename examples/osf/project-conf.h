#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/*---------------------------------------------------------------------------*/
/* NULLTB Testbed Service */
/*---------------------------------------------------------------------------*/
#ifdef BUILD_WITH_TESTBED
#ifdef LENGTH
#define TB_CONF_NULLTB_DATA_LEN          LENGTH
#else
#define TB_CONF_NULLTB_DATA_LEN          32
#endif
#define TB_CONF_NULLTB_PERIOD            3 /* seconds */
// #define TB_CONF_NULLTB_PERIOD_MIN           5
// #define TB_CONF_NULLTB_PERIOD_MAX           10
#endif

/*---------------------------------------------------------------------------*/
/* OpenSF */
/*---------------------------------------------------------------------------*/
/* SF timesync / initiator (ONLY WORKS WITHOUT TESTBED) */
#ifdef TS
#define OSF_CONF_TS                         TS
#else
#define OSF_CONF_TS                         1
#endif /* TS */

/* Number of transmissions per flood */
#ifdef PROTO
#define OSF_CONF_PROTO                      PROTO
#endif /* PROTO */

/* Number of transmissions per flood */
#ifdef NTX
#define OSF_CONF_NTX                        NTX
#else
#define OSF_CONF_NTX                        6
#endif /* NTX */

/* Number of TA pairs in the STA/Crystal protocol */
#ifdef NTA
#define OSF_CONF_PROTO_STA_N_TA             NTA
#else
#define OSF_CONF_PROTO_STA_N_TA             6
#endif /* NTA */

/* Epoch period */
#ifdef PERIOD
#define OSF_CONF_PERIOD_MS                  PERIOD
#else
#define OSF_CONF_PERIOD_MS                  500 /* milliseconds */
#endif /* PERIOD */

/* Radio physical layer */
#ifdef PHY
#define OSF_CONF_PHY                        PHY
#endif /* PHY */

/* Channel hopping*/
#ifdef CHN
#define OSF_CONF_HOPPING                    CHN
#else
#define OSF_CONF_HOPPING                    OSF_CH_NONE
#endif /* CHN */

/* Radio TX power */
#ifdef PWR
#define OSF_CONF_TXPOWER                    PWR
#else
#define OSF_CONF_TXPOWER                    ZerodBm
#endif /* PWR */

/* Payload Length in bytes */
#ifdef LENGTH
#define OSF_CONF_DATA_LEN_MAX               LENGTH
#else
#define OSF_CONF_DATA_LEN_MAX               248
#endif /* LENGTH */

/* Number of mac buffer retransmisions in osf-buffer.c (if using FIFO) */
#ifdef RTX
#define OSF_CONF_BUF_RETRANSMISSIONS        RTX
#else
#define OSF_CONF_BUF_RETRANSMISSIONS        3
#endif /* OSF_RESEND_THRESHOLD */

/* Miss RXES testing */
#ifdef MISS_RXS
#define OSF_CONF_TEST_MISS_RXS              MISS_RXS
#endif /* MISS_RXS */

#ifdef TEST_NODE
#define OSF_CONF_TEST_NODE                  TEST_NODE
#endif /* TEST_NODE */

#if OSF_WITH_IPV6
/*---------------------------------------------------------------------------*/
/* High throughput options */
/*---------------------------------------------------------------------------*/
/* MAX slots should be same as NTX */
#define OSF_CONF_ROUND_S_MAX_SLOTS          (OSF_CONF_NTX)
#define OSF_CONF_ROUND_T_MAX_SLOTS          (OSF_CONF_NTX)
#define OSF_CONF_ROUND_A_MAX_SLOTS          (OSF_CONF_NTX)

/* Statlen=0 is used for add phy header with radio len */
#define OSF_CONF_ROUND_S_STATLEN            0
#define OSF_CONF_ROUND_T_STATLEN            0
#define OSF_CONF_ROUND_A_STATLEN            0

/* Maximum number of sources / destinations supported in the network */
#define OSF_CONF_MAX_NODES                  254

/* Number of failed/missed epochs before desync and rejoin */
#define OSF_CONF_RESYNC_THRESHOLD           10

#define OSF_CONF_ROUND_S_PAYLOAD            0   // we have own payload

#if OSF_SHRINK_S_A_ROUNDS
#define OSF_ROUND_S_PAYLOAD_LENGTH          32  // fixed size of the sync frame
#define OSF_ROUND_S_AIR_LENGTH              100 // do not change !
#define OSF_CONF_ROUND_A_PAYLOAD            0   // we have own payload
#define OSF_ROUND_A_PAYLOAD_LENGTH          3   // max payload ( HDR + LENGTH )
#define OSF_ROUND_A_AIR_LENGTH              12  // AIR length  > payload length
#else
#define OSF_ROUND_S_PAYLOAD_LENGTH          32  // depends from application 
#endif /* OSF_SHRINK_S_A_ROUNDS */

/* Size of RX&TX queue */
#define OSF_CONF_BUF_MAX_SIZE               112
#define OSF_CONF_BUF_FIFO                   1

/*---------------------------------------------------------------------------*/
/* IPV6 */
/*---------------------------------------------------------------------------*/

/*The statistics is useful for debugging and to show the user */
#if (WEB_SERVER_STAT || USE_UDP_CONTROL)
#define UIP_CONF_STATISTICS      1
#endif

#if (USE_WEB_SERVER || USE_TCP_SERVER)
#define WEBSERVER_CONF_CFS_CONNS 2
#define UIP_CONF_TCP             1
#define UIP_CONF_TCP_CONNS       3
#endif

#if !USE_WEB_SERVER && !USE_TCP_SERVER
#define UIP_CONF_TCP             0
#define UIP_CONF_TCP_CONNS       0
#endif

/* Node can be used as router only */
#if !USE_WEB_SERVER && !USE_TCP_SERVER && !WITH_UDPSERVER && !WITH_UDPCONTROL
#define UIP_CONF_UDP             0
#endif

/*---------------------------------------------------------------------------*/
/* Nullrouting Netstack */
/*---------------------------------------------------------------------------*/
#ifdef OSF_CONF_PREFIX
#define UIP_CONF_DS6_DEFAULT_PREFIX         OSF_CONF_PREFIX
#else
#define UIP_CONF_DS6_DEFAULT_PREFIX         0xfd00
#endif

#define UIP_CONF_UDP_CONNS                  16  /* dcube max src/dest ? */
#define NETSTACK_MAX_ROUTE_ENTRIES          255  /* maxstatic routes, -> UIP_CONF_MAX_ROUTES */
#define QUEUEBUF_CONF_NUM                   64   /* dcube ~max dense area ? */
#define NBR_TABLE_CONF_MAX_NEIGHBORS        255  /* max */
#define UIP_CONF_BUFFER_SIZE                1500 /* 1280 - min */
#define UIP_DS6_NBR_CONF_MULTI_IPV6_ADDRS   0
#define UIP_DS6_NBR_CONF_MAX_6ADDRS_PER_NBR 2   /* 2 - default */

#define UIP_CONF_ROUTER                     1   /* all contiki nodes are routers by default */
#define UIP_CONF_ND6_SEND_RA                0
#define UIP_CONF_ND6_SEND_NS                0
#define UIP_CONF_ND6_SEND_NA                1
#define UIP_DS6_CONF_PERIOD                 (5 * CLOCK_SECOND)
#define UIP_CONF_ND6_AUTOFILL_NBR_CACHE     1 /* 0 - default if NS=0 */ /* 1 */

#define SICSLOWPAN_CONF_FRAG                1
#define SICSLOWPAN_CONF_COMPRESSION         SICSLOWPAN_COMPRESSION_IPHC
#define SICSLOWPAN_CONF_MAXAGE              9 /* 16 -> 1s; 8->0.5s - default  */
#define LLSEC802154_CONF_ENABLED            0 /*?*/
#define SICSLOWPAN_CONF_FRAGMENT_BUFFERS    112 /* 12 default */
#define SICSLOWPAN_CONF_REASS_CONTEXTS      16 /* 2 default */
#define SICSLOWPAN_CONF_FRAGMENT_SIZE       PACKETBUF_SIZE

#else
#error "Unknown configuration !"
#endif /* TESTBED or OSF_WITH_IPV6 */

/*---------------------------------------------------------------------------*/
/* Debug */
/*---------------------------------------------------------------------------*/

/* Debug logging */
#ifdef LOGGING
#define OSF_CONF_LOGGING                    LOGGING
#else
#define OSF_CONF_LOGGING                    0
#endif

/* Debug GPIO */
#ifdef GPIO
#define OSF_DEBUG_GPIO                      GPIO
#else
#define OSF_DEBUG_GPIO                      0
#endif

/* Debug LEDS */
#ifdef LEDS
#define OSF_DEBUG_LEDS                      LEDS
#else
#define OSF_DEBUG_LEDS                      0
#endif

/*---------------------------------------------------------------------------*/
/* Modules Debug */
/*---------------------------------------------------------------------------*/

#define LOG_CONF_LEVEL_RPL      LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_TCPIP    LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_IPV6     LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_6LOWPAN  LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_NULLNET  LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC      LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_FRAMER   LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAIN     LOG_LEVEL_NONE

#endif /* PROJECT_CONF_H_ */
