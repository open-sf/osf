#include "dev/nullpa.h"
#include "nrf_gpio.h"

#define FEM_MODE_PIN         NRF_GPIO_PIN_MAP(0, 17)
#define FEM_RX_EN_PIN        NRF_GPIO_PIN_MAP(0, 19)
#define FEM_TX_EN_PIN        NRF_GPIO_PIN_MAP(0, 22)
#define FEM_ANT_SEL_PIN      NRF_GPIO_PIN_MAP(0, 20)
#define FEM_NPDN_PIN         NRF_GPIO_PIN_MAP(0, 23)
#define FEM_NCSN_PIN         NRF_GPIO_PIN_MAP(0, 21)

static void off(void);

static void
init(void)
{
  nrf_gpio_cfg_output(FEM_MODE_PIN);
  nrf_gpio_cfg_output(FEM_RX_EN_PIN);
  nrf_gpio_cfg_output(FEM_TX_EN_PIN);
  nrf_gpio_cfg_output(FEM_ANT_SEL_PIN);
  nrf_gpio_cfg_output(FEM_NPDN_PIN);
  nrf_gpio_cfg_output(FEM_NCSN_PIN);

  off();
}

static void
set_antenna(pa_lna_ant_t default_ant)
{
  nrf_gpio_pin_write(FEM_ANT_SEL_PIN, default_ant & 0x1);
}

static void
set_tx_gain(pa_tx_gain_t default_gain)
{
  nrf_gpio_pin_write(FEM_MODE_PIN, default_gain & 0x1);
}

// EVENTS_TXREADY
static void
tx_begin(void)
{
  nrf_gpio_pin_set(FEM_NPDN_PIN);
  nrf_gpio_pin_clear(FEM_NCSN_PIN);

  nrf_gpio_pin_clear(FEM_RX_EN_PIN);
  nrf_gpio_pin_set(FEM_TX_EN_PIN);
}

// EVENTS_RXREADY
static void
rx_begin(void)
{
  nrf_gpio_pin_set(FEM_NPDN_PIN);
  nrf_gpio_pin_clear(FEM_NCSN_PIN);

  nrf_gpio_pin_clear(FEM_TX_EN_PIN);
  nrf_gpio_pin_set(FEM_RX_EN_PIN);
}

// EVENTS_END
static void
off(void)
{
  nrf_gpio_pin_clear(FEM_NPDN_PIN);
  nrf_gpio_pin_set(FEM_NCSN_PIN);

  nrf_gpio_pin_clear(FEM_RX_EN_PIN);
  nrf_gpio_pin_clear(FEM_TX_EN_PIN);

  nrf_gpio_pin_clear(FEM_MODE_PIN);
  nrf_gpio_pin_clear(FEM_ANT_SEL_PIN);
}

/*---------------------------------------------------------------------------*/
const struct pa_driver nrf21540pa_driver = {
  init,
  set_antenna,
  set_tx_gain,
  tx_begin,
  rx_begin,
  off,
};
