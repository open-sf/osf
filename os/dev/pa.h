#ifndef PA_H_
#define PA_H_

#include <stddef.h>

#if PA_CONF_MAX_TX_POWER
#define PA_MAX_TX_POWER PA_CONF_MAX_TX_POWER
#else
#define PA_MAX_TX_POWER 0xFF
#endif

typedef enum
{
    PA_LNA_ANT1,
    PA_LNA_ANT2
} pa_lna_ant_t;

struct pa_driver {
  void (* init)(void);
  void (* set_antenna)(pa_lna_ant_t default_ant);
  void (* tx_on)(void);
  void (* rx_on)(void);
  void (* off)(void);
};

#endif /* PA_H_ */
