/**
 * \file
 *         Testbed main file.
 * \author
 *         Michael Baddeley <michael.g.baddeley@gmail.com>
 */

#include "stdio.h"

#include "contiki.h"
#include "node-id.h"
#include "sys/rtimer.h"
#include "net/mac/osf/osf-debug.h"
#include "net/packetbuf.h"
#include "lib/memb.h"

#if BUILD_WITH_RPL_BORDER_ROUTER
#include "services/rpl-border-router/rpl-border-router.h"
#elif BUILD_WITH_NULL_BORDER_ROUTER
#include "services/null-border-router/null-border-router.h"
#endif

#include "services/testbed/testbed.h"

#include "sys/log.h"
#define LOG_MODULE "TESTBED"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UNUSED(x) (void)(x)

/* Data structures to hold node roles. It is possible to set these
   manually, or let the testbed set them from the config. */
#ifdef TB_CONF_SOURCES
TB_SOURCES(TB_CONF_SOURCES)
#else
static uint8_t tb_sources[TB_MAX_SRC_DEST]      = {0};
#endif

#ifdef TB_CONF_DESTINATIONS
TB_DESTINATIONS(TB_CONF_DESTINATIONS)
#else
static uint8_t tb_destinations[TB_MAX_SRC_DEST] = {0};
#endif

#ifdef TB_CONF_FORWARDERS
TB_FORWARDERS(TB_CONF_FORWARDERS)
#else
static uint8_t tb_forwarders[TB_MAX_SRC_DEST]   = {0};
#endif

#ifdef TB_CONF_BRS
TB_BRS(TB_CONF_BRS)
#else
static uint8_t tb_brs[TB_MAX_BR]               = {0};
#endif

/* Externs */
uint8_t tb_pattern_id, tb_node_type, tb_node_is_br, tb_msg_len, tb_traffic_pattern;
uint8_t tb_num_src, tb_num_dst, tb_num_fwd, tb_num_br;

/* Buffers */
uint8_t tb_rx_fifo[TB_RX_FIFO_LEN][MAX_TB_PACKET_LEN] = {{0}};
uint8_t tb_rx_fifo_pos = 0;
static uint8_t tb_tx_fifo[TB_TX_FIFO_LEN][MAX_TB_PACKET_LEN] = {{0}};
static uint8_t tb_tx_fifo_head = 0;
static uint8_t tb_tx_fifo_tail = 0;

/* Error Isolation */
uint16_t tb_exp_id = 0;
uint8_t pkt_flag = 0;

// Each source node is connected via software I2C to an EEPROM that contains the raw
// sensor values to be transmitted to one or multiple (at most eight) destination nodes.
// [...] signals the availability of new data to be sent in the EEPROM by toggling a pre-defined GPIO pin.
volatile uint8_t gpio_event = 0;

/*---------------------------------------------------------------------------*/
/* EEPROM */
/*---------------------------------------------------------------------------*/
PROCESS(tb_eeprom_reader_process, "DCUBE EEPROM reader");
PROCESS(tb_eeprom_writer_process, "DCUBE EEPROM writer");
PROCESS(tb_update_pkt_flag, "Update Pkt Flag");
PROCESS(tb_br_process, "DCUBE border router");

static void print_traffic_pattern(volatile tb_pattern_t* p);
static void print_config(volatile tb_config_t* cfg);
static void print_custom_config(void);

/*---------------------------------------------------------------------------*/
/* Scenario Configuration */
/*---------------------------------------------------------------------------*/
// Get the full depcomp configuration struct
volatile tb_config_t *
tb_get_config(void)
{
  return &dc_cfg;
}

/*---------------------------------------------------------------------------*/
// Get the current traffic pattern for this node
volatile tb_pattern_t *
tb_get_pattern(void)
{
  return &dc_cfg.patterns[tb_pattern_id];
}

/*---------------------------------------------------------------------------*/
uint8_t
tb_get_n_src(void)
{
// FIXME: Hack until I can think of a better way to get number of sources to nulltb
#if TB_CONF_SOURCES
  return TB_N_SOURCES;
#else
  return tb_num_src;
#endif
}

/*---------------------------------------------------------------------------*/
uint8_t *
tb_get_sources(void)
{
  return tb_sources;
}

/*---------------------------------------------------------------------------*/
uint8_t
tb_get_n_dest(void)
{
  // FIXME: Hack until I can think of a better way to get number of destinations to nulltb
  #if TB_CONF_DESTINATIONS
    return TB_N_DESTINATIONS;
  #else
    return tb_num_dst;
  #endif
}

/*---------------------------------------------------------------------------*/
uint8_t *
tb_get_destinations(void)
{
  return tb_destinations;
}


/*---------------------------------------------------------------------------*/
uint8_t
tb_get_n_br(void)
{
  // FIXME: Hack until I can think of a better way to get number of brs to nulltb
  #if TB_CONF_BRS
    return TB_N_BRS;
  #else
    return tb_num_br;
  #endif
}

/*---------------------------------------------------------------------------*/
uint8_t *
tb_get_brs(void)
{
  return tb_brs;
}

/*---------------------------------------------------------------------------*/
uint8_t
tb_get_node_type(void)
{
  return tb_node_type;
}

/*---------------------------------------------------------------------------*/
// Sets all local dc variables (tb_pattern_id, node_type, sources, etc.). Returns
// the id of the pattern the node is active in.
// TODO: Patterns are currently UNIQUE, i.e. a node will only have ONE role in a
//       SINGLE pattern.
static uint8_t
get_pattern_info()
{
  uint8_t i, j, ret = 0, found = 0;

  for(i = 0; i < TB_NUMPATTERN; i++) {
#ifdef TB_CONF_SOURCES
    LOG_WARN(" > Using preset SRCs (x%u)\n", TB_N_SOURCES);
    for(j=0; j < TB_N_SOURCES; j++) {
      tb_num_src++;
      // check to see we are participating
      if(tb_sources[j] == node_id) {
        found = 1;
        tb_node_type = NODE_TYPE_SOURCE;
      }
    }
#else
    for(j = 0; j < TB_MAX_SRC_DEST; j++) {
      if(dc_cfg.patterns[i].source_id[j]) {
        tb_sources[tb_num_src] = dc_cfg.patterns[i].source_id[j];
        tb_num_src++;
      }
      if(dc_cfg.patterns[i].source_id[j] == node_id) {
        // first check if we are a source
        tb_node_type = NODE_TYPE_SOURCE;
        ret = i;
        found = 1;
      }
    }
#endif

#ifdef TB_CONF_DESTINATIONS
    if(!tb_destinations[0]) {
      LOG_WARN(" > ALL non-SRC nodes are DSTs!\n");
      if(tb_node_type != NODE_TYPE_SOURCE) {
        tb_node_type = NODE_TYPE_DESTINATION;
      }
    } else {
      LOG_WARN(" > Using preset DSTs (x%u)\n", TB_N_DESTINATIONS);
      for(j=0; j < TB_N_DESTINATIONS; j++) {
        tb_num_dst++;
        // check to see we are participating
        if((tb_destinations[j] == node_id) || (tb_destinations[j] == TB_DESTINATION_BCAST && !tb_node_type)) {
          found = 1;
          tb_node_type = NODE_TYPE_DESTINATION;
        }
      }
    }
#else
    for(j = 0; j < TB_MAX_SRC_DEST; j++) {
      // destinations
      if(dc_cfg.patterns[i].destination_id[j]) {
        tb_destinations[tb_num_dst] = dc_cfg.patterns[i].destination_id[j];
        tb_num_dst++;
      }
      if(dc_cfg.patterns[i].destination_id[j] == node_id) {
        // then check if we are a destination
        tb_node_type = NODE_TYPE_DESTINATION;
        ret = i;
        found = 1;
      }
    }
#endif

#ifdef TB_CONF_BRS
    LOG_WARN(" > Using preset BRs (x%u)\n", TB_N_BRS);
    for(j=0; j < TB_N_BRS; j++) {
      tb_num_br++;
      // check to see we are participating
      if(tb_brs[j] == node_id) {
        tb_node_is_br = 1;
      }
    }
#else
#if TESTBED_WITH_BORDER_ROUTER
    for(j = 0; j < TB_MAX_BR; j++) {
      // border routers
      if(dc_cfg.patterns[i].br_id[j]) {
        tb_brs[tb_num_dst] = dc_cfg.patterns[i].br_id[j];
        tb_num_br++;
      }
      if(dc_cfg.patterns[i].br_id[j] == node_id) {
        // then check if we are a border router
        tb_node_is_br = 1;
        ret = i;
      }
    }
#endif
#endif
    /* If no traffic pattern has been set (e.g. when using nulltb) then figure
       it out using the # of sources and destinations */
    if(!dc_cfg.patterns[i].traffic_pattern) {
      LOG_WARN(" > No traffic_pattern! Setting using # of SRC/DST\n");
      /* NB: Testbed supports > 1 pattern but nulltb doesn't!!! */
      if (tb_num_src == 1) {
        dc_cfg.patterns[0].traffic_pattern = P2P;
      } else if(tb_num_src >= tb_num_dst) {
        dc_cfg.patterns[0].traffic_pattern = MP2P;
      } else if(tb_num_src < tb_num_dst) {
        dc_cfg.patterns[0].traffic_pattern = P2MP;
      } else {
        LOG_WARN("Unknown traffic_pattern! (s:%u d:%u)", tb_num_src, tb_num_dst);
      }
    }
    LOG_INFO(" > traffic pattern is %s (s:%u d:%u br:%u)\n",
      PATTERN_TO_STR(dc_cfg.patterns[0].traffic_pattern),
      tb_num_src, tb_num_dst, tb_num_br);
  }
#ifdef TB_CONF_FORWARDERS
  LOG_WARN(" > Using preset FWDs (x%u)\n", TB_N_FORWARDERS);
  for(i =0; i < TB_N_FORWARDERS; i++) {
    tb_num_fwd++;
    // check to see we are participating
    if(tb_forwarders[i] == node_id) {
      found = 1;
      tb_node_type = NODE_TYPE_FORWARDER;
    }
  }
  if(!found) {
    // neither a source or a destination a border router or a forwarder - so we are nothing
    tb_node_type = NODE_TYPE_NONE;
  }
#else
  // if(!found) {
  //   // neither a source or a destination or border router - so we must be a forwarder
  //   tb_node_type = NODE_TYPE_FORWARDER;
  //   tb_num_fwd++;
  // }
#endif

  return ret;
}

/*---------------------------------------------------------------------------*/
/* API */
/*---------------------------------------------------------------------------*/
static uint8_t
init()
{
  UNUSED(tb_forwarders);
  LOG_INFO("Starting %s Testbed...\n", TB_TO_STR(TESTBED));
  /* Configure the testbed pattern */
  LOG_INFO("- Configuring e2...\n");
  eeprom.config();
  /* Parse the pattern config  */
  LOG_INFO("- Get traffic pattern...\n");
  tb_pattern_id = get_pattern_info();
  /* If we are not participating as a source, destination, or forwarder, then
     we should quit contiki. */
  if(tb_get_node_type() == NODE_TYPE_NONE) {
    LOG_ERR("We have no node type (SRC/DEST/FWD)! Quit Contiki!\n");
    return 0;
  }
  /* Init I2C for sources and destinations to read/write EEPROM */
  LOG_INFO("- Init I2C...\n");
  eeprom.init();

  /* Data pattern and length */
  tb_traffic_pattern = dc_cfg.patterns[tb_pattern_id].traffic_pattern;
  tb_msg_len = dc_cfg.patterns[tb_pattern_id].msg_length;

  /* Node type */
  if(tb_node_type == NODE_TYPE_SOURCE) {
    // only source nodes need to handle the event GPIO
    process_start(&tb_eeprom_reader_process, NULL);
  } else if(tb_node_type == NODE_TYPE_DESTINATION) {
    // destinations need to write received packets
    process_start(&tb_eeprom_writer_process, NULL);
    process_start(&tb_update_pkt_flag, NULL);
  } else {
    // else we are a forwarder
  }
  LOG_INFO("- Node type will be... %s\n", NODE_TYPE_TO_STR(tb_node_type));

#if TESTBED_WITH_BORDER_ROUTER
  /* Border routers */
  if(tb_node_is_br) {
    // border routers must kick off the BR process (whatever that might be)
    process_start(&tb_br_process, NULL);
  }
  LOG_INFO("- Node is BR... %u\n", tb_node_is_br);
#endif

  // LOG_INFO("- E2 Settle Time... %u\n", GLOSSY_RTIMER_TO_MS(EEPROM_SETTLE_TIME));
  print_config(&dc_cfg);
  print_custom_config();
  LOG_INFO("%s initialized - pattern id %u: " \
    "pattern: %s msg_len:%u node_type:%s s:%u d:%u br:%u f:%u\n",
    TB_TO_STR(TESTBED), tb_pattern_id, PATTERN_TO_STR(tb_traffic_pattern), tb_msg_len,
    NODE_TYPE_TO_STR(tb_node_type), tb_num_src, tb_num_dst, tb_num_br, tb_num_fwd);

  return 1;
}

/*---------------------------------------------------------------------------*/
// Push a packet to be written to the testbed EEPROM
static uint8_t
push(uint8_t *rx_pkt, uint8_t len)
{
  if(((tb_tx_fifo_head + 1) % TB_TX_FIFO_LEN) == tb_tx_fifo_tail) {
    LOG_ERR("E2-W FIFO FULL!\n");
    return 0;
  }
  memcpy(tb_tx_fifo[tb_tx_fifo_head], rx_pkt, len);
  tb_tx_fifo_head = (tb_tx_fifo_head + 1) % TB_TX_FIFO_LEN;
  LOG_DBG("E2-W++ [%u/%u]\n", tb_tx_fifo_head, tb_tx_fifo_tail);
  return 1;
}

/*---------------------------------------------------------------------------*/
// Pop a packet for transmission from the testbed EEPROM
static uint8_t
pop(uint8_t **dest, uint8_t *len)
{
  if(tb_rx_fifo_pos == 0) {
    LOG_ERR("E2-R FIFO FULL!\n");
    return 0;
  }
  *len = tb_msg_len;
  tb_rx_fifo_pos--;
  LOG_DBG("E2-R-- %u\n", tb_rx_fifo_pos);
  // avoid doing memcpy (faster)
  *dest = (uint8_t*) &tb_rx_fifo[tb_rx_fifo_pos];
  return tb_rx_fifo_pos + 1;
}

/*---------------------------------------------------------------------------*/
static void
poll_read()
{
  process_poll(&tb_eeprom_reader_process);
}

/*---------------------------------------------------------------------------*/
static void
poll_write()
{
  process_poll(&tb_eeprom_writer_process);
}

/*---------------------------------------------------------------------------*/
static void
poll_pkt_flag()
{
  process_poll(&tb_update_pkt_flag);
}

/*---------------------------------------------------------------------------*/
void
tb_register_read_callback(tb_read_callback cb)
{
  testbed.read_callback = cb;
}

/*---------------------------------------------------------------------------*/
struct testbed_driver testbed = {
  init,
  push,
  pop,
  poll_read,
  poll_write,
  poll_pkt_flag,
  NULL
};

/*---------------------------------------------------------------------------*/
/* Read Process */
/*---------------------------------------------------------------------------*/
// Will be woken up from a GPIO interrupt on the testbed
// alternatively, provide test data when running locally
PROCESS_THREAD(tb_eeprom_reader_process, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("- Started E2-R tb_eeprom_reader_process\n");

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    if(!eeprom.event()) {
      continue;
    }

    // LOG_DBG("E2-R %u/%u\n", tb_rx_fifo_pos, TB_RX_FIFO_LEN);

    if(tb_rx_fifo_pos == TB_RX_FIFO_LEN) {
      continue;
    }

    DEBUG_GPIO_ON(DBG_PIN4);
    // we have been polled by the GPIO and need to read data from the EEPROM
    // eeprom.read(tb_rx_fifo[tb_rx_fifo_pos++]);
    LOG_DBG("E2-R++ %u\n", tb_rx_fifo_pos);

    uint8_t *rx_data;
    uint8_t rx_pkt_len;

    if(testbed.pop(&rx_data, &rx_pkt_len)) {
      testbed.read_callback(rx_data, rx_pkt_len, tb_destinations, tb_num_dst);
    }
    DEBUG_GPIO_OFF(DBG_PIN4);
  }
  PROCESS_END();
}

volatile rtimer_clock_t eeprom_next_write = 0;
/*---------------------------------------------------------------------------*/
/* Write process */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tb_eeprom_writer_process, ev, data)
{
  // static uint16_t last_write_ep = 0;

  PROCESS_BEGIN();

  LOG_INFO("- Started E2-W tb_eeprom_writer_process\n");

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

    /* Check we have data to write*/
    if(tb_tx_fifo_head == tb_tx_fifo_tail) {
      continue;
    }

    rtimer_clock_t now = RTIMER_NOW();

    if(!eeprom_next_write) {
      eeprom_next_write = now;
    }

    // NB: This was needed for sky, as you may be >2s since the last write.
    // if(last_write_ep != ep.seq_no) {
    //   // this is a new epoch
    //   last_write_ep = ep.seq_no;
    //   eeprom_next_write = now;
    // }

    /* Delay if last write was <20ms ago and we have new data this can happen
       when we are receiving data in bulk from several sources*/
    while(RTIMER_CLOCK_LT(now, eeprom_next_write)) {
      /* We need to wake up again later and actually do the write, constantly
         polling ourselves will prevent the MCU from going back to sleep but we
         don't have etimers */
      process_poll(&tb_eeprom_writer_process);
      PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
      now = RTIMER_NOW();
    }

    /* We have received a packet and need to write it to the EEPROM */
    eeprom.write(tb_tx_fifo[tb_tx_fifo_tail]);
    tb_tx_fifo_tail = (tb_tx_fifo_tail + 1) % TB_TX_FIFO_LEN;
    LOG_DBG("E2-W-- [%u/%u]\n", tb_tx_fifo_head, tb_tx_fifo_tail);

    eeprom_next_write = RTIMER_NOW() + EEPROM_SETTLE_TIME;

    if(tb_tx_fifo_head != tb_tx_fifo_tail) {
      /* Setup a periodic send timer. */
      process_poll(&tb_eeprom_writer_process);
    }

  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* Updating Pkt Flag for DST */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tb_update_pkt_flag, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("- Started E2-U tb_update_pkt_flag_process\n");
  // we have been polled by the GPIO and need to update pkt_flag

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    if(!eeprom.event()) {
      continue;
    }

    // set flag
    pkt_flag = 1;
    
    // init ID
    tb_exp_id = 1;

    // only needs to happen once
    PROCESS_EXIT();
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* Border router process */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tb_br_process, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("- Started tb_br_process\n");

#if BUILD_WITH_RPL_BORDER_ROUTER
  rpl_border_router_init();
  LOG_INFO("-- With RPL Border Router\n");
#elif BUILD_WITH_NULL_BORDER_ROUTER
  null_border_router_init();
  LOG_INFO("-- With NULL Border Router\n");
#else
  LOG_ERR("-- No Border Router process!!!\n");
#endif /* BUILD_WITH_RPL_BORDER_ROUTER */

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* Printing */
/*---------------------------------------------------------------------------*/
static void
print_traffic_pattern(volatile tb_pattern_t* p)
{
  uint8_t i;
  // NB: If TB_MAX_SRC_DEST is not the same as the testbed expects, patching will FAIL!
  LOG_INFO("  * My Node ID: %u\n", node_id);
  LOG_INFO("  * Traffic pattern: %s (%u)\n", PATTERN_TO_STR(p->traffic_pattern), p->traffic_pattern);
  if( (p->traffic_pattern > 0) && (p->traffic_pattern <= 4))
  {
    LOG_INFO("  * Sources:\n");
    for(i = 0; i < TB_MAX_SRC_DEST; i++)
    {
      if(p->source_id[i]!=0)
        LOG_INFO("     %d: %d\n", i, p->source_id[i]);
    }
    LOG_INFO("  * Destinations:\n");
    for(i = 0; i < TB_MAX_SRC_DEST; i++)
    {
      if(p->destination_id[i] !=0)
        LOG_INFO("     %d: %d\n", i, p->destination_id[i]);
    }
#if TESTBED_WITH_BORDER_ROUTER
    LOG_INFO("  * Border Routers:\n");
    for(i = 0; i < TB_MAX_BR; i++)
    {
      if(p->br_id[i] !=0)
        LOG_INFO("     %d: %d\n", i, p->br_id[i]);
    }
#endif
    if(p->periodicity == 0)
    {
      LOG_INFO("  * Aperiodic Upper: %lu\n", p->aperiodic_upper_bound);
      LOG_INFO("  * Aperiodic Lower: %lu\n", p->aperiodic_lower_bound);
    }
    else
    {
      LOG_INFO("  * Period: %lu\n", p->periodicity);
    }
    LOG_INFO("  * Delta: %lu\n", p->periodicity);
    LOG_INFO("  * Message Length: %d\n", p->msg_length);
    LOG_INFO("  * Message OffsetH: %d\n", p->msg_offsetH);
    LOG_INFO("  * Message OffsetL: %d\n", p->msg_offsetL);
  }
}

/*---------------------------------------------------------------------------*/
static void
print_config(volatile tb_config_t* cfg)
{
  uint8_t i;
  LOG_INFO("- Printing testbed config...\n");
  // if(cfg->node_id != node_id) {
  //   LOG_WARN("Config node_id != my node_id (%u != %u)!\n", cfg->node_id, node_id);
  // }
  for(i = 0; i < TB_NUMPATTERN; i++) {
    LOG_INFO(" > PATTERN %d:\n", i);
    print_traffic_pattern(&(cfg->patterns[i]));
// #if TB_CONF_SOURCES
//       LOG_INFO(" > Override SRCs:\n");
//       for(i = 0; i < tb_num_src; i++)
//       {
//           LOG_INFO("   %d: %d\n", i, tb_sources[i]);
//       }
// #endif
// #if TB_CONF_DESTINATIONS
//       LOG_INFO(" > Override DSTs:\n");
//       for(i = 0; i < tb_num_dst; i++)
//       {
//           LOG_INFO("   %d: %d\n", i, tb_destinations[i]);
//       }
// #endif
// #if TB_CONF_FORWARDERS
//       LOG_INFO(" > Override FWDs:\n");
//       for(i = 0; i < tb_num_fwd; i++)
//       {
//           LOG_INFO("   %d: %d\n", i, tb_forwarders[i]);
//       }
// #endif
  }
}

/*---------------------------------------------------------------------------*/
static void
print_custom_config()
{
#ifdef TB_CONF_CUSTOM_CONFIG
  LOG_INFO("- Printing testbed custom config...\n");
  LOG_INFO(" > _RPL_DIO_INTERVAL_MIN: %u\n", custom_cfg._RPL_DIO_INTERVAL_MIN);
  LOG_INFO(" > _MAX_LINK_METRIC: %u\n", custom_cfg._MAX_LINK_METRIC);
  LOG_INFO(" > _RANK_THRESHOLD: %u\n", custom_cfg._RANK_THRESHOLD);
#endif
}
