/**
 * \file
 *         Testbed main file.
 * \author
 *         Michael Baddeley <michael.g.baddeley@gmail.com>
 */

#ifndef TESTBED_H_
#define TESTBED_H_

#include "contiki.h"
#include "contiki-net.h"
#include "services/testbed/testbed-conf.h"

/* Turn on TB Patching for DCUBE */
#if TB_CONF_PATCHING
#define TB_PATCHING TB_CONF_PATCHING
#else
#define TB_PATCHING 0
#endif

/* Traffic patterns : Need to be #def for preprocessor reasons! */
#define NONE        0
#define P2P         1
#define P2MP        2
#define MP2P        3
#define MP2MP       4

#define PATTERN_TO_STR(P) \
 ((P == NONE)  ? ("NONE")  : \
  (P == P2P)   ? ("P2P")   : \
  (P == P2MP)  ? ("P2MP")  : \
  (P == MP2P)  ? ("MP2P")  : \
  (P == MP2MP) ? ("MP2MP") : ("UNKWN"))

#if TB_CONF_NUM_PATTERN
#define TB_NUMPATTERN TB_CONF_NUM_PATTERN
#else
#define TB_NUMPATTERN 1
#endif

#ifdef TB_CONF_SOURCES
#define TB_SOURCES(...)               static uint8_t tb_sources[] = {__VA_ARGS__};
#define TB_N_SOURCES                  sizeof(tb_sources)/sizeof(uint8_t)
#endif

#ifdef TB_CONF_DESTINATIONS
#define TB_DESTINATIONS(...)          static uint8_t tb_destinations[] = {__VA_ARGS__};
#define TB_N_DESTINATIONS             sizeof(tb_destinations)/sizeof(uint8_t)
#endif

#define TB_DESTINATION_BCAST          0xFF

#ifdef TB_CONF_FORWARDERS
#define TB_FORWARDERS(...)            static uint8_t tb_forwarders[] = {__VA_ARGS__};
#define TB_N_FORWARDERS               sizeof(tb_forwarders)/sizeof(uint8_t)
#endif

#ifdef TB_CONF_BRS
#define TB_BRS(...)                   static uint8_t tb_brs[] = {__VA_ARGS__};
#define TB_N_BRS                      sizeof(tb_brs)/sizeof(uint8_t)
#endif

#ifndef TB_PATCHING
#if !defined(TB_CONF_SOURCES) && !defined(TB_CONF_DESTINATIONS)
#warning "WARN: TESTBED NO Patching and NO SRC=... or DST=... defined. Are you sure?..."
#elif defined(TB_CONF_SOURCES) && !defined(TB_CONF_DESTINATIONS)
#error "ERROR: TESTBED Must define DST=...!"
#elif !defined(TB_CONF_SOURCES) && defined(TB_CONF_DESTINATIONS)
#error "ERROR: TESTBED Must define SRC=...!"
#endif /* TB_CONF_SOURCES && TB_CONF_DESTINATIONS */
#endif /* TB_PATCHING */

/* eeprom rx and tx arrays (used in platform specific implementations) */
#if TB_CONF_RX_FIFO_LEN
#define TB_RX_FIFO_LEN    TB_CONF_RX_FIFO_LEN
#else
#define TB_RX_FIFO_LEN    10
#endif

#if TB_CONF_TX_FIFO_LEN
#define TB_TX_FIFO_LEN    TB_CONF_TX_FIFO_LEN
#else
#define TB_TX_FIFO_LEN    10
#endif

#define MAX_TB_PACKET_LEN 255               // BLE payload in MTU

typedef struct
{
	uint8_t traffic_pattern;                  // 0:unused, 1:p2p, 2:p2mp, 3:mp2p, 4: mp2mp
	uint8_t source_id[TB_MAX_SRC_DEST];       // Only source_id[0] is used for p2p/p2mp
	uint8_t destination_id[TB_MAX_SRC_DEST];  // Only destination_id[0] is used for p2p/mp2p
#if TESTBED_WITH_BORDER_ROUTER
	uint8_t br_id[TB_MAX_BR];                 // Default max is 16
#endif
	uint8_t msg_length;                       // Message length in bytes in/to EEPROM
	uint8_t msg_offsetH;                      // Message offset in bytes in EEPROM (high byte)
	uint8_t msg_offsetL;                      // Message offset in bytes in EEPROM (low byte)

	uint32_t periodicity;                     // Period in ms (0 indicates aperiodic traffic)
	uint32_t aperiodic_upper_bound;           // Upper bound for aperiodic traffic in ms
	uint32_t aperiodic_lower_bound;           // Lower bound for aperiodic traffic in ms
#if CONTIKI_TARGET_NRF52840
  uint32_t delta;                           // The delay bound delta in ms
#endif
} tb_pattern_t;

typedef struct
{
#if CONTIKI_TARGET_NRF52840
  uint8_t node_id;                          // ID of the current node
#endif
	tb_pattern_t patterns[TB_NUMPATTERN];     // Up to TB_NUMPATTERN parallel configurations
} tb_config_t;


/* Custom Testbed Configuration Struct */
typedef struct {
  /* Empty */
#ifdef TB_CONF_CUSTOM_CONFIG
  /* rpl-conf.h */
  uint8_t    _RPL_DIO_INTERVAL_MIN;          // 12
  /* rpl-mrhof.c */
  uint16_t   _MAX_LINK_METRIC;               // 512
  uint16_t   _RANK_THRESHOLD;                // 384
#else
  uint8_t    null;
#endif
} __attribute__((packed)) custom_config_t;


extern volatile custom_config_t __attribute((section (".customConfigSection"))) custom_cfg;

// the testbed needs at least 20ms to read out the EEPROM packet after it has been written
#define EEPROM_SETTLE_TIME US_TO_RTIMERTICKS_64(20000)

/* EEPROM */
#ifdef TB_CONF_EEPROM_READ_TIME
#define TB_EEPROM_READ_TIME TB_CONF_EEPROM_READ_TIME
#else
#define TB_EEPROM_READ_TIME US_TO_RTIMERTICKS_64(10000)
#endif

#ifdef TB_CONF_EEPROM_WRITE_TIME
#define TB_EEPROM_WRITE_TIME TB_CONF_EEPROM_WRITE_TIME
#else
#define TB_EEPROM_WRITE_TIME US_TO_RTIMERTICKS_64(10000)
#endif

#define NODE_TYPE_NONE        0
#define NODE_TYPE_SOURCE      1
#define NODE_TYPE_DESTINATION 2
#define NODE_TYPE_FORWARDER   3

#define NODE_TYPE_TO_STR(type) \
 ((type == NODE_TYPE_NONE)        ? ("X") : \
  (type == NODE_TYPE_SOURCE)      ? ("S") : \
  (type == NODE_TYPE_DESTINATION) ? ("D") : \
  (type == NODE_TYPE_FORWARDER)   ? ("F") : ("U"))

/* Processes */
// FIXME: Need to work out a sensible way of polling these from other processes
PROCESS_NAME(tb_eeprom_writer_process);
PROCESS_NAME(tb_eeprom_reader_process);


extern volatile tb_config_t dc_cfg; // NB: defined in dcube-<target>.c

/* eeprom pattern information */
extern uint8_t tb_pattern_id, tb_node_type, tb_node_is_br, tb_num_src, tb_num_dst, tb_msg_len;

/* GPIO flag */
extern volatile uint8_t gpio_event;

extern volatile rtimer_clock_t eeprom_next_write;

/*---------------------------------------------------------------------------*/
struct eeprom_driver {
	void (* config)(void);            /* Configure the eeprom driver */
  void (* init)(void);            	/* Initialise the eeprom driver */
  uint8_t (* event)(void);					/* Notify testbed.c of e2 event */
  void (* read)(uint8_t* dst_buf);  /* Read from the eeprom */
  void (* write)(uint8_t* src_buf); /* Write to the eeprom */
};

extern const struct eeprom_driver eeprom;

/*---------------------------------------------------------------------------*/
typedef void (* tb_read_callback)(uint8_t *data, uint16_t datalen, uint8_t *dest, uint8_t n_dest);

struct testbed_driver {
  uint8_t (* init)(void);														/* Initialise the testbed driver */
  uint8_t (* push)(uint8_t *rx_pkt, uint8_t len);		/* Read from the testbed */
  uint8_t (* pop)(uint8_t **dest, uint8_t *len);		/* Write to the testbed */
  void (* poll_read)(void);													/* Poll the eeprom reader process */
  void (* poll_write)(void);												/* Poll the eeprom writer process */
  void (*poll_pkt_flag)(void);                      /* Poll to update pkt flag for DST */
	tb_read_callback read_callback;										/* Called on successful e2 read */
};
extern struct testbed_driver testbed;

/*---------------------------------------------------------------------------*/
extern uint8_t pkt_flag;
extern uint8_t tb_rx_fifo[TB_RX_FIFO_LEN][MAX_TB_PACKET_LEN];
extern uint8_t tb_rx_fifo_pos;
extern uint16_t  tb_exp_id;

/*---------------------------------------------------------------------------*/
volatile tb_pattern_t  *tb_get_pattern();
volatile tb_config_t   *tb_get_config();
uint8_t                 tb_get_n_src();
uint8_t                *tb_get_sources();
uint8_t                 tb_get_n_dest();
uint8_t                *tb_get_destinations();
uint8_t                 tb_get_n_br();
uint8_t                *tb_get_brs();
uint8_t                 tb_get_node_type();
void 										tb_register_read_callback(tb_read_callback cb);


#endif /* TESTBED_H_ */
