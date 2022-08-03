/**
 * \file
 *         D-Cube testbed configuration (sky).
 * \author
 *         Michael Baddeley <michael.g.baddeley@gmail.com> *
 */

#include "stdio.h"

#include "contiki.h"
#include "node-id.h"
#include "watchdog.h"
#include "isr_compat.h"

#include "net/packetbuf.h"
#include "lib/memb.h"

#include "services/testbed/testbed.h"
#include "services/testbed/dcube/sky/dcube-sky.h"
#include "services/testbed/dcube/sky/my2c.h"

#define LOG_MODULE "DCUBE-E2"
#define LOG_LEVEL LOG_LEVEL_INFO

/* In flash configuration struct, will be replaced by binary patching */
#if TB_PATCHING
volatile tb_config_t __attribute((section (".testbedConfigSection"))) dc_cfg;
/* Custom Testbed Configuration Struct Placeholder */
// volatile custom_config_t __attribute((section (".customConfigSection"))) custom_cfg={0};
#else
volatile tb_config_t dc_cfg;
volatile custom_config_t custom_cfg;
#endif /* TB_PATCHING */

static volatile uint8_t read_event = 0;

/* NB: While this driver will probably work, there's currently no way it'll
       fit in the sky's limited space. Needs work. */

/*---------------------------------------------------------------------------*/
// FIXME: Compatability issues with button-sensor line 48
ISR(PORT2, __eeprom_isr) {
  P2IFG &= ~BV(EVENT_PIN);
  read_event = 1;
}

/*---------------------------------------------------------------------------*/
static void
config(void)
{
}

/*---------------------------------------------------------------------------*/
static void
init()
{
  //enable and stop the i2c (otherwise it would block the bus)
  watchdog_stop();

  my2c_enable();
  my2c_stop();

  // only enable the interrupt if we have started the process for it
  if(tb_node_type == NODE_TYPE_SOURCE) {
    //configure pin to input with interrupt
    P2DIR &= ~BV(EVENT_PIN);
    P2SEL &= ~BV(EVENT_PIN);
    P2IES |= BV(EVENT_PIN);
    P2IFG &= ~BV(EVENT_PIN);
    P2IE |= BV(EVENT_PIN);

    //wait until the pin is initially settled
    clock_delay(10000);
    while( (P2IN & BIT6) != 0);
  } else if(tb_node_type == NODE_TYPE_DESTINATION) {
    // configure event pin as an output
    P2SEL &= ~BV(EVENT_PIN);
    P2DIR |=  BV(EVENT_PIN);
    P2OUT &= ~BV(EVENT_PIN);
  }

  watchdog_start();
}

/*---------------------------------------------------------------------------*/
static uint8_t
event()
{
  return read_event;
}

/*---------------------------------------------------------------------------*/
void
eeprom_read(uint8_t* dst_buf)
{
  watchdog_stop();

  read_event = 0;

  //i2c start
  my2c_start();
  //write address on the bus
  my2c_write(0x50 << 1);
  //write memory address (2 bytes)
  my2c_write(dc_cfg.patterns[tb_pattern_id].msg_offsetH);
  my2c_write(dc_cfg.patterns[tb_pattern_id].msg_offsetL);
  //disable the bus and wait a bit
  my2c_stop();
  clock_delay(100);
  my2c_start();
  //write address on the bus and set read bit
  my2c_write((0x50 << 1) | 1);

  //LOG_DBG("EEPROM RX: ");
  //read back all the data, on the last byte indicate end
  uint8_t i;
  for(i = 0; i < (dc_cfg.patterns[tb_pattern_id].msg_length); i++) {
    if(i == ((dc_cfg.patterns[tb_pattern_id].msg_length) - 1)) {
      dst_buf[i] = my2c_read(0);
    } else {
      dst_buf[i] = my2c_read(1);
    }
  }
  //i2c stop
  my2c_stop();
  watchdog_start();
}

/*---------------------------------------------------------------------------*/
// using the msg len from the configuration struct
void
eeprom_write(uint8_t* src_buf)
{
  watchdog_stop();

  // raise gpio pin to indicate write operation
  P2OUT |= BV(EVENT_PIN);

  //start i2c communication
  my2c_start();
  //write address on the bus
  my2c_write(0x50 << 1);
  //write memory address (2 bytes)
  my2c_write(dc_cfg.patterns[tb_pattern_id].msg_offsetH);
  my2c_write(dc_cfg.patterns[tb_pattern_id].msg_offsetL);

  uint8_t i;
  //write messages up to the length in the struct
  for(i = 0; i < dc_cfg.patterns[tb_pattern_id].msg_length; i++) {
    my2c_write(src_buf[i]);
  }
  //stop i2c communication
  my2c_stop();

  //lower gpio pin to indicate finished write
  P2OUT &= ~BV(EVENT_PIN);

  watchdog_start();
}

/*---------------------------------------------------------------------------*/
const struct eeprom_driver eeprom = {
  config,
  init,
  event,
  eeprom_read,
  eeprom_write
};
