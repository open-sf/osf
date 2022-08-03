/**
 * \file
 *         D-Cube testbed configuration (nRF52840).
 * \author
 *         Michael Baddeley <michael.g.baddeley@gmail.com> *
 */


#include "contiki.h"

#include "net/packetbuf.h"
#include "lib/memb.h"

#include "nrf_drv_gpiote.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"

#include "services/testbed/testbed.h"

#include "sys/log.h"
#define LOG_MODULE "DCUBE-E2"
#define LOG_LEVEL LOG_LEVEL_INFO

/* In flash configuration struct, will be replaced by binary patching */
volatile tb_config_t __attribute((section (".testbedConfigSection"))) dc_cfg = {0};
// volatile tb_config_t __attribute((section (".testbedConfigSection"))) dc_cfg = {3,{{2,{3,0,0,0},{1,2,4,0},8,0,0,5000,0,0,5000}}};
/* Custom Testbed Configuration Struct Placeholder */
#ifdef TB_CONF_CUSTOM_CONFIG
volatile custom_config_t __attribute((section (".customConfigSection"))) custom_cfg={TB_CONF_CUSTOM_CONFIG};
#else
volatile custom_config_t __attribute((section (".customConfigSection"))) custom_cfg={0};
#endif

/* Common addresses definition for eeprom. */
#define EEPROM_ADDR             (0xA0U >> 1)

/* Testbed I2C Pin definition. */
#define TWI0_CONFIG_SCL          3
#define TWI0_CONFIG_SDA          4

// EEPROM PIN
#define PIN_IN NRF_GPIO_PIN_MAP(1,2)

/* TWI instance ID. */
#define TWI_INSTANCE_ID          0

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
static volatile bool m_xfer_done = false;

static uint8_t eeprom_pkt_buf[257];

static volatile uint8_t read_event = 0;

/*---------------------------------------------------------------------------*/
// I2C initialization.
static void
twi_init(void)
{
  ret_code_t err_code;

  const nrf_drv_twi_config_t twi_eeprom_config = {
      .scl                = TWI0_CONFIG_SCL,
      .sda                = TWI0_CONFIG_SDA,
      .frequency          = NRF_DRV_TWI_FREQ_100K,
      .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
      .clear_bus_init     = false
  };

  // err_code = nrf_drv_twi_init(&m_twi, &twi_eeprom_config, twi_handler, NULL);
  err_code = nrf_drv_twi_init(&m_twi, &twi_eeprom_config, NULL, NULL);
  APP_ERROR_CHECK(err_code);

  nrf_drv_twi_enable(&m_twi);
}

/*---------------------------------------------------------------------------*/
/* GPIO */
/*---------------------------------------------------------------------------*/
void
read_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  // atm_log_str("E2 EVENT\n");
  read_event = 1;
}

/*---------------------------------------------------------------------------*/
void
write_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  // TODO: Is this necessary?
}

/*---------------------------------------------------------------------------*/
// Function for initializing the boards GPIOs
static void
gpio_init(void)
{
  ret_code_t err_code;

  //GPIO block monitored by the testbed (P1.01-P1.08)
  for(uint8_t pin = 1; pin < 9; pin++)
  {
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1,pin));
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1,pin));
  }

  if(tb_node_type == NODE_TYPE_DESTINATION) {
    LOG_INFO("Init DST GPIO...\n");
    //EEPROM data is signaled on pin P1.02
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1,2));
  } else if (tb_node_type == NODE_TYPE_SOURCE) {
    LOG_INFO("Init SRC GPIO...\n");
    // nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    // in_config.pull = NRF_GPIO_PIN_PULLUP; //drive by the testbed only if selected as output
    err_code = nrf_drv_gpiote_in_init(PIN_IN, &in_config, read_pin_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(PIN_IN, true);
  }

}

/*---------------------------------------------------------------------------*/
/* EEPROM driver functions */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static void
config(void)
{
}
/*---------------------------------------------------------------------------*/
static void
init()
{
  LOG_INFO("- Init TESTBED=dcube...\n");
  if(NRF_POWER->RESETREAS & 0x00000001) {
    LOG_WARN("- Reset from pin-reset detected !\n");
  }
  if(NRF_POWER->RESETREAS & 0x00000002) {
    LOG_ERR("- Reset from watchdog detected !\n");
  }
  if(NRF_POWER->RESETREAS & 0x00000004) {
    LOG_ERR("- Reset from soft reset detected !\n");
  }
  if(NRF_POWER->RESETREAS & 0x00000008) {
    LOG_ERR("- Reset from CPU lock-up detected !\n");
  }
  twi_init();
  gpio_init();
}

/*---------------------------------------------------------------------------*/
static uint8_t
event()
{
  LOG_DBG("read event!\n");
  return read_event;
}

/*---------------------------------------------------------------------------*/
static void
eeprom_read(uint8_t* dst_buf)
{
  unsigned char eeprom_by_address[2];
  ret_code_t err_code;
  read_event = 0;
  uint16_t eeprom_address = 0; // NB: previously argv[0], and passed as 0

  memset(dst_buf, 0, dc_cfg.patterns[tb_pattern_id].msg_length);

  eeprom_by_address[1]	= eeprom_address;
  eeprom_by_address[0] = (unsigned char)(eeprom_address << 8);

  // setting the start address
  LOG_DBG("RS:%u\n", dst_buf[0]);
  do {
    m_xfer_done = false;
    err_code = nrf_drv_twi_tx(&m_twi, EEPROM_ADDR, eeprom_by_address, 2, true);
    if(NRF_SUCCESS != err_code)
      continue;
    // while(!m_xfer_done); // TODO: We want I2C to be blocking, so we don't use this
    m_xfer_done = false;
    err_code = nrf_drv_twi_rx(&m_twi, EEPROM_ADDR, dst_buf, dc_cfg.patterns[tb_pattern_id].msg_length);
  } while(NRF_SUCCESS != err_code);
  LOG_DBG("RE:%u\n", dst_buf[0]);

  // while(!m_xfer_done) watchdog_periodic(); // TODO: We want I2C to be blocking, so we don't use this
}

/*---------------------------------------------------------------------------*/
// using the msg len from the configuration struct
static void
eeprom_write(uint8_t* src_buf)
{
  ret_code_t err_code;

  eeprom_pkt_buf[0] = dc_cfg.patterns[tb_pattern_id].msg_offsetH;
  eeprom_pkt_buf[1] = dc_cfg.patterns[tb_pattern_id].msg_offsetL;
  memcpy(&eeprom_pkt_buf[2], src_buf, dc_cfg.patterns[tb_pattern_id].msg_length);
  uint8_t len = dc_cfg.patterns[tb_pattern_id].msg_length + 2;

  nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(1,2));
  // setting the start address
  do {
    m_xfer_done = false;
    err_code = nrf_drv_twi_tx(&m_twi, EEPROM_ADDR, eeprom_pkt_buf, len, false);
  } while(NRF_SUCCESS != err_code);

  // while(!m_xfer_done) watchdog_periodic(); // TODO: We want I2C to be blocking, so we don't use this

  nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1,2));
}

/*---------------------------------------------------------------------------*/
const struct eeprom_driver eeprom = {
    config, // dcube configures the traffic pattern automatically
    init,
    event,
    eeprom_read,
    eeprom_write
};
