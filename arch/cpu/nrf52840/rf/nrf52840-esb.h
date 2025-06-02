/* arch/cpu/nrf52840/rf/nrf52840-esb.h */

#ifndef NRF52840_ESB_H_
#define NRF52840_ESB_H_

#include <stdint.h>
#include <stdbool.h>

/* Callback type for RADIO IRQ */
typedef void (*nrf52840_radioirq_callback_t)(void);

/**
 * \brief Register a callback for RADIO interrupts.
 */
void
nrf52840_radioirq_register_handler(nrf52840_radioirq_callback_t handler);

#endif /* NRF52840_ESB_H_ */
