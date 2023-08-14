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
    PA_LNA_ANT1 = 0,
    PA_LNA_ANT2
} pa_lna_ant_t;

typedef enum
{
    PA_TX_Plus20dBm = 0,
    PA_TX_Plus10dBm
} pa_tx_gain_t;

#define PA_ANT_TO_STR(A) \
  ((A == PA_LNA_ANT1) ? ("LNA_ANT1") : \
   (A == PA_LNA_ANT2) ? ("LNA_ANT2") : ("???"))

#define PA_TXPOWER_TO_STR(P) \
  ((P == PA_TX_Plus20dBm) ? ("+20dBm") : \
   (P == PA_TX_Plus10dBm) ? ("+10dBm") : ("???"))

struct pa_driver {
  void (* init)(void);
  void (* set_antenna)(pa_lna_ant_t default_ant);
  void (* set_tx_gain)(pa_tx_gain_t default_gain);
  void (* tx_on)(void);
  void (* rx_on)(void);
  void (* off)(void);
};

#endif /* PA_H_ */
