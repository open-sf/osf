/**
 * \file
 *         NULL testbed configuration (NULLTB).
 * \author
 *         Michael Baddeley <michael.g.baddeley@gmail.com> *
 */


#include "contiki.h"
#include "node-id.h"

#include "lib/memb.h"
#include "net/packetbuf.h"

#include "services/testbed/testbed.h"
#include "services/testbed/testbed-rand.h"

#include "sys/log.h"
#define LOG_MODULE "NULLTB-E2"
#define LOG_LEVEL LOG_LEVEL_INFO

#define RAND_UP_TO(n) (int)(((n) * (uint32_t)random_rand()) / RANDOM_RAND_MAX)

volatile tb_config_t dc_cfg;

static volatile uint8_t read_event = 0;

static struct etimer timer;

PROCESS(tb_nulltb_process, "Null Testbed Process");
/*---------------------------------------------------------------------------*/
static inline void
print_division(unsigned long x, unsigned long y)
{
  uint32_t a = (uint32_t) (((uint64_t) x * 100uLL) / y);
  uint32_t b = (uint32_t) (((uint64_t) x * 100uLL / y % 100));
  if(b < 0) {
    b = (~b + 1);
  }
  printf("%02lu.%02lu", a/100, b % 100);
}

/*---------------------------------------------------------------------------*/
static void
spoof_isr()
{
  read_event = 1; // spoof the ISR
}

/*---------------------------------------------------------------------------*/
static void
config(void)
{
#if CONTIKI_TARGET_NRF52840
  dc_cfg.node_id = node_id;
#endif
  dc_cfg.patterns[0].msg_length = TB_DATA_LEN;
  memcpy((uint8_t*) dc_cfg.patterns[0].source_id, tb_get_sources(), tb_get_n_src());
  memcpy((uint8_t*) dc_cfg.patterns[0].destination_id, tb_get_destinations(), tb_get_n_dest());
#if TESTBED_WITH_BORDER_ROUTER && CONTIKI_TARGET_NRF52840
  memcpy((uint8_t*) dc_cfg.patterns[0].br_id, tb_get_brs(), tb_get_n_br());
#endif
}

/*---------------------------------------------------------------------------*/
static void
init(void)
{
  /* Use static num so all nodes have same buf */
  tb_rand_init(1234);
#if TB_CONF_PERIOD
  /* Setup a periodic send timer. */
  LOG_INFO("NULLTB PERIOD - %us\n", TB_PERIOD/CLOCK_SECOND);
#elif TB_CONF_PERIOD_MIN && TB_CONF_PERIOD_MAX
  /* Setup an aperiodic send timer. */
  LOG_INFO("NULLTB MIN PERIOD - %us\n", TB_PERIOD_MIN/CLOCK_SECOND);
  LOG_INFO("NULLTB MAX PERIOD - %us\n", TB_PERIOD_MAX/CLOCK_SECOND);
#endif
  process_start(&tb_nulltb_process, NULL);
}

/*---------------------------------------------------------------------------*/
static uint8_t
event() {
  return read_event;
}

/*---------------------------------------------------------------------------*/
static void
eeprom_read(uint8_t* dst_buf)
{
  // maybe uncomment this
  // uint8_t i;
  // for(i = 0; i < tb_msg_len; i++)
  // {
  //   TB_RAND_QUICK(dst_buf[i]);
  // }
  read_event = 0;
  rtimer_clock_t t_end_read = RTIMER_NOW() + TB_EEPROM_READ_TIME;
  do {} while(RTIMER_CLOCK_LT(RTIMER_NOW(), t_end_read));
}

/*---------------------------------------------------------------------------*/
static void
eeprom_write(uint8_t* src_buf)
{
  // rtimer_clock_t t_end_write = NRF_RTIMER_NOW() + TB_EEPROM_WRITE_TIME;
  // do {} while(RTIMER_CLOCK_LT(NRF_RTIMER_NOW(), t_end_write));
  rtimer_clock_t t_end_write = RTIMER_NOW() + TB_EEPROM_WRITE_TIME;
  do {} while(RTIMER_CLOCK_LT(RTIMER_NOW(), t_end_write));
}

/*---------------------------------------------------------------------------*/
static void
schedule_read() {
  #if TB_PERIOD
      /* Periodic send timer. */
      clock_time_t next_send = TB_PERIOD;
  #else
      /* Aperiodic send timer. */
      clock_time_t next_send = TB_PERIOD_MIN + RAND_UP_TO(TB_PERIOD_MAX - TB_PERIOD_MIN);
  #endif

  #if LOG_LEVEL >= LOG_LEVEL_DBG
      LOG_DBG("Send in... ");
      print_division(next_send, CLOCK_SECOND);
      LOG_DBG_("s  %lu\n", next_send);
  #endif
    etimer_set(&timer, next_send);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tb_nulltb_process, ev, data)
{
  PROCESS_BEGIN();

  // try to remove pkt_flag here
  if(tb_node_type != NODE_TYPE_SOURCE && pkt_flag == 1) {
    LOG_WARN("Node type is (%s). Is not source, exiting nulltb reading process!\n", NODE_TYPE_TO_STR(tb_node_type));
    PROCESS_EXIT();
  }

  schedule_read();

  while(1) {
    /* Wait for the periodic timer to expire and then restart the timer. */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    spoof_isr(); // spoof the ISR
    schedule_read();
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
const struct eeprom_driver eeprom = {
  config,
  init,
  event,
  eeprom_read,
  eeprom_write
};
