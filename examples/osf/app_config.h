#ifndef APP_CONFIG_H
#define APP_CONFIG_H
//
 /* RTC peripheral driver */
 #define RTC0_ENABLED 1
 #define RTC1_ENABLED 0
//
 /* TIMER periperal driver */
 #define TIMER0_ENABLED 1
 #define TIMER1_ENABLED 1
 #define TIMER2_ENABLED 1
 #define TIMER3_ENABLED 1
//
 /* WDT peripheral driver */
 #define NRFX_WDT_ENABLED 1
//
#if NRF52840_NATIVE_USB

/* TODO : check default values and other valued
 * in file arch/platform/nrf52840/config/sdk_config.h
 */
#define NRFX_USBD_ENABLED 1
#define NRFX_USBD_CONFIG_IRQ_PRIORITY 6

#define NRFX_USBD_CONFIG_DMASCHEDULER_MODE 0
#define NRFX_USBD_CONFIG_DMASCHEDULER_ISO_BOOST 1
#define NRFX_USBD_CONFIG_ISO_IN_ZLP 0

#define POWER_ENABLED 1
#define POWER_CONFIG_IRQ_PRIORITY 6
#define NRFX_POWER_CONFIG_DEFAULT_DCDCEN 0
#define NRFX_POWER_CONFIG_DEFAULT_DCDCENHV 0

#define NRF_POWER_HAS_USBREG 1

#define USBD_ENABLED 1
#define USBD_CONFIG_IRQ_PRIORITY 6
#define USBD_CONFIG_DMASCHEDULER_MODE 0
#define USBD_CONFIG_DMASCHEDULER_ISO_BOOST 1
#define USBD_CONFIG_ISO_IN_ZLP 0

#define APP_USBD_ENABLED 1
#define APP_USBD_VID 0x1915
#define APP_USBD_PID 0x520F
#define APP_USBD_DEVICE_VER_MAJOR 1
#define APP_USBD_DEVICE_VER_MINOR 0

#define APP_USBD_CONFIG_SELF_POWERED 1
#define APP_USBD_CONFIG_MAX_POWER 100
#define APP_USBD_CONFIG_POWER_EVENTS_PROCESS 1
#define APP_USBD_CONFIG_EVENT_QUEUE_SIZE 32
#define APP_USBD_CONFIG_SOF_HANDLING_MODE 2
#define APP_USBD_CONFIG_SOF_TIMESTAMP_PROVIDE 0
#define APP_USBD_CONFIG_DESC_STRING_SIZE 31

#define APP_USBD_CONFIG_EVENT_QUEUE_ENABLE 0 /* nrf_atfifo.h */

#define APP_USBD_CDC_ACM_ENABLED 1
#define APP_USBD_CDC_ACM_ZLP_ON_EPSIZE_WRITE 1

#define NRFX_USBD_CONFIG_LOG_ENABLED 1
#define NRFX_USBD_CONFIG_LOG_LEVEL 3

#endif /* NRF52840_NATIVE_USB */

#endif /* APP_CONFIG_H */
