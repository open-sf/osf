/*---------------------------------------------------------------------------*/
#ifndef NRF_IEEE_DRIVER_H_
#define NRF_IEEE_DRIVER_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
/*---------------------------------------------------------------------------*/

typedef void (*nrf_radioirq_callback_t)(void);
void nrf_radioirq_register_handler(nrf_radioirq_callback_t handler);

#endif /* NRF_IEEE_DRIVER_H_ */
