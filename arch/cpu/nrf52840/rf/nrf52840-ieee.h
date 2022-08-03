/*---------------------------------------------------------------------------*/
#ifndef NRF52840_IEEE_H_
#define NRF52840_IEEE_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
/*---------------------------------------------------------------------------*/

typedef void (*nrf52840_radioirq_callback_t)(void);
void nrf52840_radioirq_register_handler(nrf52840_radioirq_callback_t handler);

#endif /* NRF52840_IEEE_H_ */
