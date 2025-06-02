#include "contiki.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "sys/log.h"
#include "sys/critical.h"
#include "nrf52840-esb.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "nrf.h"
#include "hal/nrf_radio.h"
#include "hal/nrf_clock.h"
#include "hal/nrf_power.h"

#define LOG_MODULE "ESB"
#define LOG_LEVEL  LOG_LEVEL_NONE

#define MAX_PAYLOAD_LEN 127

#ifndef CONF_RADIO_TXPOWER
#define CONF_RADIO_TXPOWER RADIO_TXPOWER_TXPOWER_Pos4dBm
#endif

#ifndef CONF_RADIO_CCA_THRESHOLD
#define CONF_RADIO_CCA_THRESHOLD -75
#endif

#ifndef IEEE802154_DEFAULT_CHANNEL
#define IEEE802154_DEFAULT_CHANNEL 26
#endif

PROCESS(nrf_esb_process, "nRF ESB Radio Process");

static nrf52840_radioirq_callback_t irq_handler = NULL;
static uint8_t rxbuf[MAX_PAYLOAD_LEN + 1];
static uint8_t txbuf[MAX_PAYLOAD_LEN + 1];
static int8_t last_rssi = 127;
static volatile bool rx_pending = false;
static bool is_on = false;
static bool poll_mode = false;
static radio_value_t current_channel = IEEE802154_DEFAULT_CHANNEL;
static radio_value_t current_txpower = CONF_RADIO_TXPOWER;

static int init(void);
static int prepare(const void *payload, unsigned short len);
static int transmit(unsigned short len);
static int send(const void *payload, unsigned short len);
static int radio_read(void *buf, unsigned short buflen);
static int channel_clear(void);
static int receiving_packet(void);
static int pending_packet(void);
static int on(void);
static int off(void);
static radio_result_t get_value(radio_param_t param, radio_value_t *value);
static radio_result_t set_value(radio_param_t param, radio_value_t value);
static radio_result_t get_object(radio_param_t param, void *dest, size_t size);
static radio_result_t set_object(radio_param_t param, const void *src, size_t size);

const struct radio_driver nrf52840_esb_driver = {
  init, prepare, transmit, send,
  radio_read, channel_clear, receiving_packet, pending_packet,
  on, off,
  get_value, set_value, get_object, set_object
};

void nrf52840_radioirq_register_handler(nrf52840_radioirq_callback_t handler) {
  irq_handler = handler;
  LOG_DBG("IRQ handler registered %p\n", (void *)handler);
}

static void enter_rx(void) {
  nrf_radio_state_t curr_state = (nrf_radio_state_t)NRF_RADIO->STATE;
  LOG_DBG("Enter RX, state=%u\n", curr_state);

  if(curr_state == RADIO_STATE_STATE_Rx) {
    LOG_DBG("Already in RX\n");
    return;
  }

  NRF_RADIO->PACKETPTR = (uint32_t)rxbuf;
  NRF_RADIO->EVENTS_READY = 0;
  NRF_RADIO->EVENTS_END = 0;
  NRF_RADIO->EVENTS_DISABLED = 0;
  NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk;
  NRF_RADIO->TASKS_RXEN = 1;

  LOG_DBG("RX started, state=%u\n", (int)NRF_RADIO->STATE);
}

static void configure(void) {
  NRF_RADIO->FREQUENCY = current_channel;
  NRF_RADIO->TXPOWER = current_txpower << RADIO_TXPOWER_TXPOWER_Pos;
  NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_1Mbit << RADIO_MODE_MODE_Pos;
  NRF_RADIO->PREFIX0 = 0xC1;
  NRF_RADIO->BASE0 = 0x89ABCDEF;
  NRF_RADIO->TXADDRESS = 0x00;
  NRF_RADIO->RXADDRESSES = 1 << 0;
  NRF_RADIO->CRCCNF = RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos;
  NRF_RADIO->CRCPOLY = 0x11021;
  NRF_RADIO->CRCINIT = 0xFFFF;
  NRF_RADIO->PCNF0 = (0 << RADIO_PCNF0_S0LEN_Pos) |
                     (8 << RADIO_PCNF0_LFLEN_Pos) |
                     (0 << RADIO_PCNF0_S1LEN_Pos);
  NRF_RADIO->PCNF1 = (MAX_PAYLOAD_LEN << RADIO_PCNF1_MAXLEN_Pos) |
                     (0 << RADIO_PCNF1_STATLEN_Pos) |
                     (3 << RADIO_PCNF1_BALEN_Pos) |
                     (RADIO_PCNF1_ENDIAN_Little << RADIO_PCNF1_ENDIAN_Pos);
  NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk;
  NRF_RADIO->INTENCLR = 0xFFFFFFFF;
  NRF_RADIO->INTENSET = RADIO_INTENSET_CRCOK_Msk | RADIO_INTENSET_CRCERROR_Msk;
  NVIC_ClearPendingIRQ(RADIO_IRQn);
  NVIC_EnableIRQ(RADIO_IRQn);
  enter_rx();
}

static void power_on_and_configure(void) {
  NRF_RADIO->POWER = 1;
  configure();
}

static int init(void) {
  LOG_INFO("ESB init: starting HFCLK\n");
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;
  while(!NRF_CLOCK->EVENTS_HFCLKSTARTED);
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  power_on_and_configure();
  process_start(&nrf_esb_process, NULL);
  on();
  return 0;
}

static int prepare(const void *payload, unsigned short len) {
  LOG_DBG("prepare(): len=%u\n", len);
  if(len > MAX_PAYLOAD_LEN) return RADIO_TX_ERR;
  txbuf[0] = len;
  memcpy(&txbuf[1], payload, len);
  NRF_RADIO->PACKETPTR = (uint32_t)txbuf;
  return RADIO_TX_OK;
}

static int transmit(unsigned short len) {
  LOG_DBG("transmit(): len=%u\n", len);
  NRF_RADIO->EVENTS_READY = 0;
  NRF_RADIO->EVENTS_END = 0;
  NRF_RADIO->TASKS_DISABLE = 1;
  while(NRF_RADIO->EVENTS_DISABLED == 0);

  NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk;
  NRF_RADIO->TASKS_TXEN = 1;

  while(NRF_RADIO->EVENTS_READY == 0);
  while(NRF_RADIO->EVENTS_END == 0);

  NRF_RADIO->TASKS_DISABLE = 1;
  while(NRF_RADIO->EVENTS_DISABLED == 0);

  enter_rx();
  return RADIO_TX_OK;
}

static int send(const void *payload, unsigned short len) {
  return prepare(payload, len) == RADIO_TX_OK ? transmit(len) : RADIO_TX_ERR;
}

static int radio_read(void *buf, unsigned short buflen) {
  if(!rx_pending) return 0;
  rx_pending = false;
  uint16_t l = rxbuf[0];
  if(l > buflen) return 0;
  memcpy(buf, &rxbuf[1], l);
  return l;
}

static int channel_clear(void) {
  NRF_RADIO->EVENTS_RSSIEND = 0;
  NRF_RADIO->TASKS_RSSISTART = 1;
  while(!NRF_RADIO->EVENTS_RSSIEND);
  int8_t rssi_sample = (int8_t)NRF_RADIO->RSSISAMPLE;
  last_rssi = rssi_sample;
  LOG_DBG("channel_clear(): RSSI = %d dBm => %s\n", rssi_sample,
          (rssi_sample < CONF_RADIO_CCA_THRESHOLD) ? "clear" : "busy");
  return (rssi_sample < CONF_RADIO_CCA_THRESHOLD) ? 1 : 0;
}

static int receiving_packet(void) {
  int receiving = 0;
  if(!poll_mode) {
    if(NRF_RADIO->EVENTS_ADDRESS &&
       NRF_RADIO->STATE == RADIO_STATE_STATE_Rx) {
      receiving = 1;
    }
  }
  LOG_DBG("receiving_packet(): %d\n", receiving);
  return receiving;
}

static int pending_packet(void) {
  return rx_pending ? 1 : 0;
}

static int on(void) {
  is_on = true;
  enter_rx();
  return 1;
}

static int off(void) {
  is_on = false;
  NRF_RADIO->TASKS_DISABLE = 1;
  return 0;
}

static radio_result_t get_value(radio_param_t param, radio_value_t *value) {
  if(!value) return RADIO_RESULT_INVALID_VALUE;
  switch(param) {
    case RADIO_CONST_MAX_PAYLOAD_LEN: *value = MAX_PAYLOAD_LEN; return RADIO_RESULT_OK;
    case RADIO_PARAM_RSSI: *value = last_rssi; return RADIO_RESULT_OK;
    case RADIO_PARAM_RX_MODE: *value = poll_mode ? RADIO_RX_MODE_POLL_MODE : 0; return RADIO_RESULT_OK;
    case RADIO_PARAM_CHANNEL: *value = current_channel; return RADIO_RESULT_OK;
    case RADIO_PARAM_TXPOWER: *value = current_txpower; return RADIO_RESULT_OK;
    case RADIO_PARAM_POWER_MODE: *value = is_on ? RADIO_POWER_MODE_ON : RADIO_POWER_MODE_OFF; return RADIO_RESULT_OK;
    default: return RADIO_RESULT_NOT_SUPPORTED;
  }
}

static radio_result_t set_value(radio_param_t param, radio_value_t value) {
  switch(param) {
    case RADIO_PARAM_CHANNEL:
      if(value < 0 || value > 100) return RADIO_RESULT_INVALID_VALUE;
      current_channel = value;
      if(is_on) {
        off();
        on();
      }
      return RADIO_RESULT_OK;
    case RADIO_PARAM_TXPOWER:
      current_txpower = value;
      if(is_on) {
        NRF_RADIO->TXPOWER = value << RADIO_TXPOWER_TXPOWER_Pos;
      }
      return RADIO_RESULT_OK;
    case RADIO_PARAM_POWER_MODE:
      if(value == RADIO_POWER_MODE_ON) return on();
      if(value == RADIO_POWER_MODE_OFF) return off();
      return RADIO_RESULT_INVALID_VALUE;
    case RADIO_PARAM_RX_MODE:
      poll_mode = (value & RADIO_RX_MODE_POLL_MODE) != 0;
      return RADIO_RESULT_OK;
    default:
      LOG_WARN("set_value(%d, %ld) not supported\n", param, (long)value);
      return RADIO_RESULT_NOT_SUPPORTED;
  }
}

static radio_result_t get_object(radio_param_t p, void *d, size_t s) { return RADIO_RESULT_NOT_SUPPORTED; }
static radio_result_t set_object(radio_param_t p, const void *s, size_t z) { return RADIO_RESULT_NOT_SUPPORTED; }

PROCESS_THREAD(nrf_esb_process, ev, data) {
  PROCESS_BEGIN();
  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    LOG_DBG("nrf_esb_process polled");
    if(pending_packet()) {
      packetbuf_clear();
      int len = radio_read(packetbuf_dataptr(), PACKETBUF_SIZE);
      if(len > 0) {
        packetbuf_set_datalen(len);
        NETSTACK_MAC.input();
      }
    }
  }
  PROCESS_END();
}

void RADIO_IRQHandler(void) {
  if(irq_handler) {
    irq_handler();
    return;
  }

  if(NRF_RADIO->EVENTS_CRCOK) {
    NRF_RADIO->EVENTS_CRCOK = 0;
    NRF_RADIO->EVENTS_ADDRESS = 0;
    last_rssi = (int8_t)NRF_RADIO->RSSISAMPLE;
    rx_pending = true;
    if(poll_mode) {
      process_poll(&nrf_esb_process);
    } else {
      packetbuf_clear();
      int len = radio_read(packetbuf_dataptr(), PACKETBUF_SIZE);
      if(len > 0) {
        packetbuf_set_datalen(len);
        NETSTACK_MAC.input();
      }
      enter_rx();
    }
  }

  if(NRF_RADIO->EVENTS_CRCERROR) {
    NRF_RADIO->EVENTS_CRCERROR = 0;
    NRF_RADIO->PACKETPTR = (uint32_t)rxbuf;
  }

  NRF_RADIO->EVENTS_END = 0;
  NRF_RADIO->EVENTS_READY = 0;
  NRF_RADIO->EVENTS_DISABLED = 0;
}