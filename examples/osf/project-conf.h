#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/*---------------------------------------------------------------------------*/
/* HELLO_WORLD Application */
/*---------------------------------------------------------------------------*/
#ifdef HELLO_WORLD
#define HELLO_WORLD_PERIOD                  (CLOCK_SECOND * 5)
#endif

/*---------------------------------------------------------------------------*/
/* NULLTB Testbed Service */
/*---------------------------------------------------------------------------*/
#ifdef BUILD_WITH_TESTBED
#ifdef LENGTH
#define TB_CONF_NULLTB_DATA_LEN             LENGTH /* bytes */
#else
#define TB_CONF_NULLTB_DATA_LEN             8 /* bytes */
#endif
#define TB_CONF_NULLTB_PERIOD               3 /* seconds */
// #define TB_CONF_NULLTB_PERIOD_MIN           5
// #define TB_CONF_NULLTB_PERIOD_MAX           10
#endif

/*---------------------------------------------------------------------------*/
/* Data and Buffer */
/*---------------------------------------------------------------------------*/
/* Payload Length in bytes */
#ifdef LENGTH
#define OSF_CONF_DATA_LEN_MAX               LENGTH
#else
#ifdef HELLO_WORLD
#define OSF_CONF_DATA_LEN_MAX               64
#else
#define OSF_CONF_DATA_LEN_MAX               8
#endif
#endif /* LENGTH */

/* Number of (re)transmissions per packet at the buffer level */
#define OSF_CONF_BUF_RETRANSMISSIONS        0

/*---------------------------------------------------------------------------*/
/* Basic OSF Configuration */
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
#endif /* NTX */

/* Number of transmissions per flood */
#ifdef NTX
#define OSF_CONF_NTX                        NTX
#else
#define OSF_CONF_NTX                        6
#endif /* NTX */

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

/* Multi-PHY Radio physical layer */
#ifdef MPHY
#define OSF_CONF_MPHY                       MPHY
#endif /* MPHY */

/* Channel hopping*/
#ifdef CHN
#define OSF_CONF_HOPPING                    CHN
#endif /* CHN */

/* Radio TX power */
#ifdef PWR
#define OSF_CONF_TXPOWER                    PWR
#else
#define OSF_CONF_TXPOWER                    ZerodBm
#endif /* PWR */

/*---------------------------------------------------------------------------*/
/* D-Cube experimental testing options */
/*---------------------------------------------------------------------------*/
/* Max # of slots in a flood */
#ifdef NSLOTS
#define OSF_CONF_ROUND_S_MAX_SLOTS          NSLOTS
#define OSF_CONF_ROUND_T_MAX_SLOTS          NSLOTS
#define OSF_CONF_ROUND_A_MAX_SLOTS          NSLOTS
#else
#define OSF_CONF_ROUND_S_MAX_SLOTS          (2 * OSF_CONF_NTX)
#define OSF_CONF_ROUND_T_MAX_SLOTS          (2 * OSF_CONF_NTX)
#define OSF_CONF_ROUND_A_MAX_SLOTS          (2 * OSF_CONF_NTX)
#endif /* NSLOTS */

#define OSF_CONF_ROUND_S_STATLEN            1
#define OSF_CONF_ROUND_T_STATLEN            1
#define OSF_CONF_ROUND_A_STATLEN            1

/* Underlying primitive (Glossy/RoF) */
#define ROF                                 OSF_PRIMITIVE_ROF
#define GLOSSY                              OSF_PRIMITIVE_GLOSSY
#ifdef PRIMITIVE
#define OSF_CONF_PRIMITIVE                  PRIMITIVE
#else
#define OSF_CONF_PRIMITIVE                  OSF_PRIMITIVE_ROF
#endif /* NTX */

/* Number of TA pairs in the STA/Crystal protocol */
#ifdef NTA
#define OSF_CONF_PROTO_STA_NTA              NTA
#endif /* NTA */

/* Exit the protocol after 4 empty rounds (x2 TA pairs) */
#ifdef EMPTY
#define OSF_CONF_PROTO_STA_EMPTY            EMPTY
#endif

/* ACK bit-toggling in the STA/Crystal protocol */
#ifdef TOG
#define OSF_CONF_PROTO_STA_ACK_TOGGLING     TOG
#endif
/* ALWAYS ACK in the STA/Crystal protocol. NB: Can only be used with SINGLE destination */
#ifdef ALWAYS_ACK
#define OSF_CONF_ROUND_A_ALWAYS_ACK         ALWAYS_ACK
#endif

/* Noise Detection protocol extension */
#ifdef ND
#define OSF_CONF_EXT_ND                     ND
#endif

/* Random NTX */
#ifdef RNTX
#define OSF_CONF_EXT_RNTX                   RNTX
#endif

/* Random TX Backoff */
#ifdef BACKOFF
#define OSF_CONF_EXT_BACKOFF                BACKOFF
#endif

/* Miss RXES testing */
#ifdef MISS_RXS
#define OSF_CONF_TEST_MISS_RXS              MISS_RXS
#endif /* MISS_RXS */
#ifdef TEST_NODE
#define OSF_CONF_TEST_NODE                  TEST_NODE
#endif /* TEST_NODE */

/*---------------------------------------------------------------------------*/
/* Debug */
/*---------------------------------------------------------------------------*/

/* Debug logging */
#ifdef LOGGING
#define OSF_CONF_LOGGING                    LOGGING
#else
#define OSF_CONF_LOGGING                    1
#endif

/* Debug GPIO */
#ifdef GPIO
#define OSF_DEBUG_GPIO                      GPIO
#else
#define OSF_DEBUG_GPIO                      1
#endif

/* Debug LEDS */
#ifdef LEDS
#define OSF_DEBUG_LEDS                      LEDS
#else
#define OSF_DEBUG_LEDS                      1
#endif

/*---------------------------------------------------------------------------*/
/* Debug level of modules */
/*---------------------------------------------------------------------------*/

#define LOG_CONF_LEVEL_TCPIP    LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_IPV6     LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_6LOWPAN  LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_NULLNET  LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC      LOG_LEVEL_ERR
#define LOG_CONF_LEVEL_FRAMER   LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAIN     LOG_LEVEL_NONE

#endif /* PROJECT_CONF_H_ */
