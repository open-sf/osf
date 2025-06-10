#include "dev/nullpa.h"

static void
init(void)
{

}

static void
set_antenna(pa_lna_ant_t default_ant)
{

}

static void
set_tx_gain(pa_tx_gain_t default_gain)
{

}

static void
set_attenuator(uint8_t mode)
{
  
}

static void
set_rx_gain(pa_rx_gain_t default_gain)
{

}

// EVENTS_TXREADY
static void
tx_begin(void)
{

}

// EVENTS_RXREADY
static void
rx_begin(void)
{

}

// EVENTS_END
static void
off(void)
{

}

/*---------------------------------------------------------------------------*/
const struct pa_driver nullpa_driver = {
  init,
  set_antenna,
  set_tx_gain,
  set_attenuator,
  set_rx_gain,
  tx_begin,
  rx_begin,
  off,
};
