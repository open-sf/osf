/*---------------------------------------------------------------------------*/
#ifndef NRF52832_CONF_H_
#define NRF52832_CONF_H_
/*---------------------------------------------------------------------------*/
#if NRF52840_WITH_ESB_RADIO
#define NETSTACK_CONF_RADIO        nrf52840_esb_driver
#elif NRF52840_WITH_NULL_RADIO
#define NETSTACK_CONF_RADIO        nullradio_driver
#endif
/*---------------------------------------------------------------------------*/
#ifndef UART0_CONF_BAUD_RATE
#define UART0_CONF_BAUD_RATE       NRF_UART_BAUDRATE_115200
#endif
/*---------------------------------------------------------------------------*/
#endif /* NRF52832_CONF_H_ */
/*---------------------------------------------------------------------------*/
