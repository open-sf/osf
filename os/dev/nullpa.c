#include "dev/nullpa.h"

static void
init(void)
{

}

static void
set_antenna(pa_lna_ant_t default_ant)
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
  tx_begin,
  rx_begin,
  off,
};
