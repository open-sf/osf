/*
 * Copyright (c) 2022, Technology Innovation Institute
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /**
 * \file
 *         OSF main file.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

#include "contiki.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include "watchdog.h"

#include "project-conf.h"

#include "net/packetbuf.h"
#include "net/netstack.h"

#if BUILD_WITH_TESTBED
#include "services/testbed/testbed.h"
#endif

#include "nrf_timer.h"
#include "nrf_clock.h"
#include "nrf_radio.h"
#include "nrf_ppi.h"
#include "sys/critical.h"
#include "sys/int-master.h"
#include "net/mac/osf/nrf52840-osf.h"

#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-timer.h"
#include "net/mac/osf/osf-ch.h"
#include "net/mac/osf/osf-packet.h"
#include "net/mac/osf/osf-proto.h"
#include "net/mac/osf/osf-buffer.h"
#include "net/mac/osf/extensions/osf-ext.h"
#include "net/mac/osf/osf-debug.h"
#include "net/mac/osf/osf-log.h"
#include "net/mac/osf/osf-stat.h"

#include "nrf52840-ieee.h"
static void RADIO_IRQHandler_callback();

/* MUST INCLUDE THESE FOR NODE IDS AND TESTBED PATTERNS */
#include "services/deployment/deployment.h"
#if CONF_TESTBED
#include "services/testbed/testbed.h"
#endif

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "OSF"
#define LOG_LEVEL LOG_LEVEL_DBG

/*---------------------------------------------------------------------------*/
/* OSF main struct */
osf_t             osf = {0};

/* OSF state machine */
uint8_t           osf_state;

/* Aligned buf for TX/RX. Macro for osf_buf is defined in header file. */
osf_buf_t         osf_aligned_buf;
/* Pointers to parts of the osf_packet */
osf_pkt_phy_t    *osf_buf_phy;
osf_pkt_hdr_t    *osf_buf_hdr;
void             *osf_buf_rnd_pkt;
uint16_t          osf_buf_len;

/* OSF extensions */
static osf_ext_d_t *osf_d_extension = OSF_DRIVER_EXTENSION;
osf_ext_p_t *osf_p_extension = OSF_PROTO_EXTENSION;

/* Global OSF variables */
uint8_t osf_is_on = 0;

uint8_t node_is_timesync = 0;
uint8_t node_is_synced = 0;
uint8_t node_is_joined = 0;
uint8_t osf_timesync = 0;

uint8_t node_is_source = 0;
uint8_t node_is_destination = 0;
uint8_t node_is_br = 0;

/* Timings */
rtimer_clock_t t_ref = 0;          // ref time for start of each slot
static rtimer_clock_t t_ev_ready_ts = 0;  // end event timestamp
static rtimer_clock_t t_ev_addr_ts = 0;   // address event timestamp
static rtimer_clock_t t_ev_end_ts = 0;    // end event timestamp
static rtimer_clock_t t_round_start = 0;  // ref time for start of OSF round
static rtimer_clock_t t_round_end = 0;    // ref time to stop OSF round
static rtimer_clock_t t_epoch_end = 0;         // ref time to stop OSF epoch
static uint8_t ch_timeout_set = 0;
static uint8_t rx_timeout_set = 0;

/* Timeouts */
static rtimer_clock_t t_ch_first_timeout; // first channel hopping timeout after t_ref
static rtimer_clock_t t_ch_next_timeout;  // subsequent channel hopping timeout after first
static uint16_t       n_ch_timeouts;      // counter for number of channel timeouts this round
static void hop_rx(struct rtimer *t, void *ptr);
static void stop_rx(struct rtimer *t, void *ptr);

/* Application data */
static osf_input_callback_t receive_callback;

/*----------------------------------------------------------------------------*/
/* TESTING PURPOSES ONLY */
#if OSF_TEST_MISS_RXS
static uint8_t test_n_missed_rxes = OSF_TEST_MISS_RXS;
#endif

/*----------------------------------------------------------------------------*/
/* Function declarations */
static void    schedule_epoch();
static void    start_round();
static void    end_round();
static inline void do_slot();
static void    print_osf_config();
static void    print_osf_timings();

/* Check if in interrupt mode */
static inline bool isInterrupt()
{
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

/*---------------------------------------------------------------------------*/
/* Processes */
/*---------------------------------------------------------------------------*/
// FIXME: Isn't this going against the resons for a fully ISR-based driver?
//        Although I think we do something similar for the TESTBED calls?
PROCESS(osf_post_round_process, "OSF Post Round Process");
PROCESS(osf_post_epoch_process, "OSF Post Epoch Process");

/*---------------------------------------------------------------------------*/
// TODO: Replace with osf_off()
static void
osf_stop(void)
{
  if(osf_state == OSF_STATE_OFF) {
    LOG_WARN("OSF is already stopped!\n");
  }
  /* Turn off the radio */
  my_radio_off_completely();
  osf_state = OSF_STATE_OFF;
  NETSTACK_PA.off();
  /* Do extensions */
  DO_OSF_D_EXTENSION(stop);

  DEBUG_LEDS_OFF(CRCERR_LED);
}

/*---------------------------------------------------------------------------*/
/* Timers for serve Idle time of the ROUND */
// TODO: Don't need these until we have dual radio configurations.
// #define RTIMERX_RTC0_RATIO (RTIMERX_SECOND / CLOCK_SECOND)
// #define RTIMERX_TO_RTC0(X) ((rtimer_clock_t)(((int64_t)(X) / RTIMERX_RTC0_RATIO)))

/*---------------------------------------------------------------------------*/
void
osf_sync(void)
{
  /* Calculate reference time */
  rtimer_clock_t t_epoch_ref_tmp = t_ev_addr_ts - osf.slot * osf_phy_conf->slot_duration;
  /* Calculate drift if already synced */
  if(node_is_synced) {
    osf.t_epoch_drift = t_epoch_ref_tmp - osf.t_epoch_ref;
  }
  /* Sync */
  osf_pkt_s_round_t *rnd_pkt = (osf_pkt_s_round_t *)osf_buf_rnd_pkt;
  if(node_is_synced && osf.epoch != rnd_pkt->epoch){
    // FIXME: Need to have an array of received packets so that we don't start
    //        sending corrupted packets in GLOSSY
    osf_log_s("WARN", "osf.epoch != received epoch!\n");
    osf_stat.osf_sync_epoch_err_total++; /* Statistics */
  } else {
    osf.epoch = rnd_pkt->epoch;
  }
  osf.t_epoch_ref = t_epoch_ref_tmp;
  // FIXME: This only works if we assume we can only sync from the S round
  t_round_start = osf.t_epoch_ref;
  t_epoch_end = osf.t_epoch_ref + osf.proto->duration;
  t_round_end = osf.t_epoch_ref + osf.rconf->duration;
  osf.last_sync_slot = osf.slot;
  osf.failed_epochs = 0;
  node_is_synced = 1;
  osf.n_syncs++;
  /* Catch up to the correct channel */
  osf_ch_init_index(osf.epoch + osf.proto->index);
  osf_ch_index += (osf.slot+1);
  osf_ch_index = osf_ch_index % osf_ch_len;
  /* Join - TODO: This could be a whole handshake process */
  if(!node_is_joined) {
    osf.join_epoch = osf.epoch;
    node_is_joined = 1;
    osf_stat.osf_join_total++; /* Statistics */
  }
  DEBUG_LEDS_ON(SYNCED_LED);
}

/*---------------------------------------------------------------------------*/
static void
set_timesync()
{
#if BUILD_WITH_TESTBED
  volatile tb_pattern_t *pattern = tb_get_pattern();
  switch(pattern->traffic_pattern) {
  case P2P:
  case P2MP:
    osf_timesync = tb_get_sources()[0];
    break;
  case MP2P:
    osf_timesync = tb_get_destinations()[0];
    break;
  case MP2MP:
  default:
    LOG_ERR("Unhandled PATTERN type! %s (%u)\n", PATTERN_TO_STR(pattern->traffic_pattern), pattern->traffic_pattern);
    return;
  }
  LOG_INFO("- OSF Timesync AUTO (%s) set to node %u ", PATTERN_TO_STR(pattern->traffic_pattern), osf_timesync);
#else
  osf_timesync = OSF_TS;
  LOG_INFO("- OSF Timesync MANUAL set to node %u ", osf_timesync);
#endif
  /* Check if we are the timesync */
  if(node_id == osf_timesync) {
    node_is_timesync = 1;
    node_is_synced = 1;
    node_is_joined = 1;
    DEBUG_LEDS_ON(TS_LED);
    LOG_INFO_("... I am TS! (TS is %u)\n", osf_timesync);
  } else {
    LOG_INFO_("... I am NOT TS! (TS is %u)\n", osf_timesync);
  }
}

/*---------------------------------------------------------------------------*/
/* Round scheduling */
/*---------------------------------------------------------------------------*/
static void
schedule_epoch()
{
  /* Increment the epoch counter */
  osf.epoch++;
  // DEBUG_LEDS_TOGGLE(ROUND_LED);
  /* Set next available epoch from now */
  osf.t_epoch_ref += osf.period;
  /* Check that we haven't overrun doing other stuff. If so, keep calling this
     function until we can schedule an epoch. */
  rtimer_clock_t now = RTIMERX_NOW();
  if(RTIMER_CLOCK_LT(osf.t_epoch_ref, now)) {
    schedule_epoch();
  } else {
#if BUILD_WITH_TESTBED
    if (tb_node_type == NODE_TYPE_SOURCE && node_is_synced) {
      testbed.poll_read();
    }
#endif
    /* Clear protocol schedule */
    uint8_t i;
    for (i = 0; i < osf.proto->len; i++) {
      osf.proto->sched[i].ntx = 0;
    }
    /* Do extension */
    DO_OSF_D_EXTENSION(configure, osf.proto);
    /* Configure epoch protocol */
    osf.proto->configure();
    t_epoch_end = osf.t_epoch_ref + osf.proto->duration;
    osf.rconf = osf.proto->next_round();
    start_round();
  }
}

/*---------------------------------------------------------------------------*/
/* Channel timeout scheduling  */
/*---------------------------------------------------------------------------*/
static uint8_t
schedule_ch_timeout(rtimer_clock_t now)
{
  rtimer_callback_t cb;
  uint8_t r = 0;
  if(!node_is_synced || osf.round->primitive == OSF_PRIMITIVE_ROF) {
    cb = hop_rx;
  } else {
    cb = stop_rx;
  }
  if(ch_timeout_set) {
    LOG_WARN("CH timeout already set!\n");
  }
  /* If the node is not joined/synced, then do a long scanning timeout */
  if(!node_is_synced) {
    /* Do a long timeout for scanning */
    rtimer_clock_t scan_timeout = now + OSF_SCAN_TIME;
    r = rtimerx_set_fixed(scan_timeout, cb, "scan");
    n_ch_timeouts++;
    /* Print so we know we are still alive */
    if(n_ch_timeouts && !(n_ch_timeouts % 20)) {
      LOG_INFO("{%u|syn-%-4u} <3\n", node_id, n_ch_timeouts);
    }
  /* If we are synced, we want to try and hop with the initiator(s) */
  } else {
    /* First timeout we need to consider the RX guard, and then hop in the middle of ADDR -> END */
    if (!n_ch_timeouts || osf.round->primitive == OSF_PRIMITIVE_GLOSSY) {
      t_ch_first_timeout = now
                          + OSF_RX_GUARD
                          + osf_phy_conf->header_air_ticks
                          + ((osf_phy_conf->packet_air_ticks - osf_phy_conf->header_air_ticks)  >> 1);
      r = rtimerx_set_fixed(t_ch_first_timeout, cb, "h1");
    /* Subsequent timeouts we just need to hop at OSF_SLOT_LEN after the first timeout */
    } else if (n_ch_timeouts < osf.rconf->max_slots) {
      t_ch_next_timeout = t_ch_first_timeout + (n_ch_timeouts * osf_phy_conf->slot_duration);
      /* If we are at the last slot, we actually don't want to hop, we just
         want to stop and go back to the round process */
      if(n_ch_timeouts == osf.rconf->max_slots - 1) {

        r = rtimerx_set_fixed(t_ch_next_timeout, stop_rx, "he");
      } else {
        r = rtimerx_set_fixed(t_ch_next_timeout, cb, "hn");
      }
      if (r != RTIMER_OK) {
#if OSF_LOGGING
        osf_log_slot_state('O');
#endif
        n_ch_timeouts++;
        return schedule_ch_timeout(now);
      }
    } else {
      /* We never received, so return to the round process */
      osf_stop();
      end_round();
      return 0;
    }
  }
  return 1;
}

/*---------------------------------------------------------------------------*/
static uint8_t
schedule_rx_timeout(rtimer_clock_t now)
{
  uint8_t r;
  if(rx_timeout_set) {
    LOG_WARN("RX timeout already set!\n");
  }
  /* We are currently at EVENTS_ADDR, so set a timeout for just after when we
     would expect EVENTS_END to be. */
  rtimer_clock_t t_rx_timeout = now
                               + osf_phy_conf->payload_air_ticks
                               + osf_phy_conf->footer_air_ticks
                               + osf_phy_conf->tx_rx_end_offset_ticks;
  r = rtimerx_set_fixed(t_rx_timeout, stop_rx, "sn");
  if (r != RTIMER_OK) {
    osf_stop();
    LOG_WARN("RX timeout MISS\n");
    end_round();
    osf_stat.osf_rt_miss_timeout_total++;/* Statistics */
    return 0;
  } else {
    return 1;
  }
}

/*---------------------------------------------------------------------------*/
static uint8_t
schedule_slot_timeout(rtimer_clock_t now)
{
  uint8_t r;
  if(rx_timeout_set) {
    LOG_WARN("Glossy timeout already set!\n");
  }
  /* We want to timeout at the end of the slot */
  rtimer_clock_t t_slot_timeout = now
                               + OSF_RX_GUARD
                               + osf_phy_conf->packet_air_ticks
                               + osf_phy_conf->tx_rx_end_offset_ticks
                               + US_TO_RTIMERTICKSX(10); // Small guard for time spent in EVENTS_END interrupt
  r = rtimerx_set_fixed(t_slot_timeout, stop_rx, "st");

  if (r != RTIMER_OK) {
    osf_stop();
    LOG_WARN("GLOSSY timeout MISS\n");
    end_round();
    osf_stat.osf_rt_miss_glossy_total++;/* Statistics */
    return 0;
  } else {
    return 1;
  }
}

/*---------------------------------------------------------------------------*/
static uint8_t
get_next_channel()
{
  uint8_t channel = CH_DEFAULT;
#if OSF_HOPPING
  if(node_is_synced) {
    channel = osf_channels[osf_ch_index];
#if OSF_LOGGING
    osf_log_slot_ch();
#endif
#if OSF_TEST_MISS_RXS
    // Hardcoded IDs to test channel hopping timeouts on the scope
    if(test_n_missed_rxes && (node_id == OSF_TEST_NODE) && (osf_state == OSF_STATE_WAITING)) {
      channel = 85; // Set to a channel outside of a standard
      test_n_missed_rxes--;
    }
#endif
    // DEBUG_FLASH_GPIO(osf_ch_index, DBG_PIN1);
    osf_ch_index = (osf_ch_index + 1) % osf_ch_len;
  } else {
    channel = scan_channels[osf_scan_index];
    // DEBUG_FLASH_GPIO(osf_scan_index, DBG_PIN1);
    // Increment channel for next time we hop
    osf_scan_index++;
    if(osf_scan_index == osf_scan_len) {
      osf_scan_index = 0;
    }
  }
#endif
  return channel;
}

/*---------------------------------------------------------------------------*/
static void
hop_rx(struct rtimer *t, void *ptr)
{
  /* Kick the dog */
  watchdog_periodic();
#if OSF_LOGGING
  osf_log_slot_state('H');
#endif
  ch_timeout_set = 0;
  /* If we are synced we need to estimate where we are */
  if(node_is_synced) {
    n_ch_timeouts++;
    osf.slot++; // increment to next slot
    DO_OSF_D_EXTENSION(hop);
    DO_OSF_P_EXTENSION(hop);
  }
  /* Set channel and RX */
  uint8_t channel = get_next_channel();
  my_radio_rx_hop_channel(channel);
}

/*---------------------------------------------------------------------------*/
static void
stop_rx(struct rtimer *t, void *ptr)
{
  rx_timeout_set = 0;
  ch_timeout_set = 0;
  /* Just stop RX without doing anything else */
#if OSF_LOGGING
  if(osf.round->primitive == OSF_PRIMITIVE_ROF) {
    osf_log_slot_state('E');
  } else {
    osf_log_slot_state('.');
  }
#endif
  /* If we have timed out the RX, and are going in to a new RX, we need to
     pretend we did a hop_rx() */
  n_ch_timeouts++;
  /* Increment to (at least) the next slot. We actually have no idea how
     many slots we have missed by it must be at least one. */
  osf.slot++;
  /* Do next slot */
  do_slot();
}


/*---------------------------------------------------------------------------*/
/* RX/TX */
/*---------------------------------------------------------------------------*/
static uint8_t
start_rx(rtimer_clock_t target)
{
  /* Set state */
  osf_state = OSF_STATE_WAITING;
  /* Set target to wake up and RX (small time before we expect the first TX) */
  target -= OSF_RX_GUARD;
  /* Set channel */
  uint8_t channel = get_next_channel();

#if OSF_RX_MAX_LIMIT
  if (osf.rconf->phy->mode == PHY_IEEE) {
    my_radio_set_maxlen(OSF_MAXLEN(osf.rconf->phy->mode));
  } else {
    /* Limit maxlen for avoid RX overflow */
    // my_radio_set_maxlen(OSF_ROUND_S_PAYLOAD_LENGTH); // FIXME: WHY?
    if(node_is_joined) {
      if (osf.round->type == OSF_ROUND_A) {
        // my_radio_set_maxlen(OSF_PKT_HDR_LEN + OSF_PKT_A_RND_LEN + MIC_SIZE); // FIXME: WHY?
      } else if (osf.round->type == OSF_ROUND_T) {
        my_radio_set_maxlen(OSF_MAXLEN(osf.rconf->phy->mode));
        }
    }
  }
#else
  my_radio_set_maxlen(OSF_MAXLEN(osf.rconf->phy->mode));
#endif

  /* Schedule RX (will be triggered on TIMERX) */
  return schedule_rx_abs(&osf_buf[0], channel, target);
}

/*---------------------------------------------------------------------------*/
static void
end_rx()
{
  osf.last_rx_ok = (NRF_RADIO->CRCSTATUS & RADIO_CRCSTATUS_CRCSTATUS_CRCOk); // TODO: move this radio check to the radio

  /* Statistics */
  /* FIXME: What is this trying to log? */
  if (node_is_joined && (osf.round->type == OSF_ROUND_T)) {
    osf_stat.osf_mac_rx_slots_total++;
  }

  /* Check CRC */
  if(osf.last_rx_ok) {
    /* We received a valid packet */
    osf_state = OSF_STATE_RECEIVED;
    osf.n_rx_ok += 1;
    /* Copy the data over to our buf. If we expect a PHY header, then take
       the length from the LF, otherwise, take the expected statlen */
    // FIXME: osf_buf_len should really include the PHY header len
    if(!osf.round->statlen) {
      osf_buf_len = ((osf.rconf->phy->mode != PHY_IEEE) ? osf_buf_phy->ble.len : osf_buf_phy->ieee.len);
    } else {
      osf_buf_len = osf.rconf->phy->conf->statlen;
    }
    /* Check osf header */
    osf.slot = osf_buf_hdr->slot;
    /* Work out slot drift */
    osf.t_slot_drift = (node_is_synced ? t_ev_addr_ts - t_ref - OSF_REF_SHIFT : 0);
    /* Check for sync round */
    if(!node_is_timesync && osf.round->sync) {
      osf_sync();
    }
    /* Do extensions */
    DO_OSF_D_EXTENSION(rx_ok, osf.round->type, osf_buf, osf_buf_len);
#if OSF_LOGGING
    osf_log_slot_state('R');
    osf_log_slot_node(osf_buf_hdr->src);
    osf_log_slot_rssi();
    osf_log_slot_td();
    osf_log_radio_buffer(osf_buf, OSF_PKT_PHY_LEN(osf.rconf->phy->mode, osf.round->statlen) + osf_buf_len, 0, OSF_PKT_RND_LEN(osf.round->type), osf.round->statlen, osf.round->type);
#endif
  } else {
    /* Error indicator */
    DEBUG_LEDS_ON(CRCERR_LED);
    /* We received an invalid packet. Log this error then head back to RX */
    osf.n_rx_crc += 1;
    // FIXME: This should not be done here.
    if (osf.round->type == OSF_ROUND_S) {
        osf_stat.osf_rx_sync_crc_error_total++;
    } else if (osf.round->type == OSF_ROUND_T) {
        osf_stat.osf_rx_tx_crc_error_total++;
    } else if (osf.round->type == OSF_ROUND_A) {
        osf_stat.osf_rx_ack_crc_error_total++;
    }
    /* Do extensions */
    DO_OSF_D_EXTENSION(rx_error);
#if OSF_LOGGING
    osf_log_slot_state('C');
    osf_log_slot_rssi();
    osf_log_slot_td();
#endif
    if(!node_is_synced) {
      start_rx(RTIMERX_NOW() + US_TO_RTIMERTICKSX(500));
      return;
    }
  }
  /* Do next slot */
  n_ch_timeouts++;
  osf.slot++;
  do_slot();
}

/*---------------------------------------------------------------------------*/
static uint8_t
start_tx(rtimer_clock_t target)
{
  /* Update OSF state */
  osf_state = OSF_STATE_RECEIVED;

  // FIXME: What on earth is going on here?
  if(!osf.round->statlen) {
    if (osf.round->type == OSF_ROUND_T){
      my_radio_set_maxlen(OSF_MAXLEN(osf.rconf->phy->mode));
    }
    // FIXME: WHY?
    // if (osf.round->type == OSF_ROUND_S) {
    //   *osf_buf_phy_len = OSF_ROUND_S_PAYLOAD_LENGTH;
    // } else if (osf.round->type == OSF_ROUND_A) {
    //   *osf_buf_phy_len = OSF_PKT_HDR_LEN + OSF_PKT_A_RND_LEN;
    // }
  }

  /* Set osf header */
  osf_buf_hdr->slot = osf.slot;
  /* Set channel */
  uint8_t channel = get_next_channel();
  /* Schedule TX (will be triggered on TIMERX) */
  return schedule_tx_abs(osf_buf, channel, target);
}

/*---------------------------------------------------------------------------*/
void
end_tx()
{
#if OSF_LOGGING
  osf_log_slot_state('T');
  osf_log_slot_node(osf_buf_hdr->dst);
#endif
  osf.n_tx++;
  DO_OSF_D_EXTENSION(tx_ok);
  // /* If we are doing glossy then each TX needs to increment the ch timeouts,
  //    pretending we just did a ROF hop */
  // if(osf.round->primitive == OSF_PRIMITIVE_GLOSSY) {
  //   n_ch_timeouts = 0;
  // } else {
    n_ch_timeouts++;
  // }
  /* Do next slot */
  osf_log_radio_buffer(osf_buf, OSF_PKT_PHY_LEN(osf.rconf->phy->mode, osf.round->statlen) + osf_buf_len, 1, OSF_PKT_RND_LEN(osf.round->type), osf.round->statlen, osf.round->type);
  osf.slot++; // increment to next slot (also do this in RX)
  do_slot();
}

/*---------------------------------------------------------------------------*/
static inline void
do_slot()
{
  uint8_t r;
  /* Set next timeslot */
  t_ref = t_round_start + osf.slot * osf_phy_conf->slot_duration;
  /* Pull back by the ref shift if we aren't a timesync */
  if (!node_is_timesync) {
    t_ref -= OSF_REF_SHIFT;
  }
  /* Loop for NTX timeslots in round (TX and RX) */
  if(ROUND_LEN_RULE) {
    DEBUG_LEDS_ON(ROUND_LED);
    /* Do we TX or RX this timeslot? */
    if(osf.round->primitive == OSF_PRIMITIVE_ROF ? OSF_DOTX_ROF() : OSF_DOTX_GLOSSY()) {
      r = start_tx(t_ref);
      osf.last_slot_type = OSF_SLOT_T;
    } else {
      r = start_rx(t_ref);
      osf.last_slot_type = OSF_SLOT_R;
    }
    if(r != RTIMER_OK) {
      rtimer_clock_t now = RTIMERX_NOW();
      /* Check we aren't beond the end of the epoch */
      if(RTIMER_CLOCK_LT(t_epoch_end, now)) {
  #if OSF_LOGGING
        osf_log_slot_state('X');
  #endif
        LOG_DBG("{%u|ep-%-4u} rt miss EPOCH %s | t_ref:+%lu us eoe:+%lu us\n",
          node_id, osf.epoch, OSF_ROUND_TO_STR(osf.round->type),
          RTIMERTICKS_TO_USX(now - t_ref), RTIMERTICKS_TO_USX(now - t_epoch_end));
        osf.proto->index = osf.proto->len; // we exit the round process after we reach len
        osf_stop();
        end_round();
        osf_stat.osf_rt_miss_epoch_total++; /* Statictics */
      /* Check we aren't beond the end of the round */
      } else if (RTIMER_CLOCK_LT(t_round_end, now)) {
  #if OSF_LOGGING
        osf_log_slot_state('S');
  #endif
        LOG_DBG("{%u|ep-%-4u} rt miss ROUND %s | t_ref:+%lu us eor:+%lu us\n",
          node_id, osf.epoch, OSF_ROUND_TO_STR(osf.round->type),
          RTIMERTICKS_TO_USX(now - t_ref), RTIMERTICKS_TO_USX(now - t_round_end));
        osf_stop();
        end_round();
        osf_stat.osf_rt_miss_round_total++;/* Statictics */
      /* Else we must have missed a slot */
      } else {
#if OSF_LOGGING
        osf_log_slot_state('M');
#endif
        n_ch_timeouts++;
        osf.slot++;
        do_slot();
        osf_stat.osf_rt_miss_slot_total++;/* Statictics */
      }
    }
  } else {
    osf_stop();
    end_round();
    DEBUG_LEDS_OFF(ROUND_LED);
  }
}


/*---------------------------------------------------------------------------*/
static void
start_round() {
  /* Register our own radio handler (we can release later) */
  // FIXME: Radio stuff needs to be moved to the radio driver.
  NVIC_DisableIRQ(RADIO_IRQn);
  nrf_radio_int_disable(0xFFFFFFFF);
  NVIC_ClearPendingIRQ(RADIO_IRQn);
  nrf52840_radioirq_register_handler(RADIO_IRQHandler_callback);

  /* After a whole epoch of sleeping, we will have drifted. Correct this. */
  // osf.t_epoch_ref += osf.t_epoch_drift; // FIXME: Makes things worse. Need a moving avg.

  /* Initialise the round */
  osf.round = osf.rconf->round;

  /* Init radio event times */
  t_ev_ready_ts = 0;
  t_ev_addr_ts = 0;
  t_ev_end_ts = 0;
  /* Init round vars */
  osf.n_tx = 0;
  osf.n_rx_ok = 0;
  osf.n_rx_crc = 0;
  osf.last_rx_ok = 0;
  osf.slot = 0;
  n_ch_timeouts = 0;
  osf.last_sync_slot = 0;
#if OSF_TEST_MISS_RXS
  test_n_missed_rxes = OSF_TEST_MISS_RXS;
#endif
  /* Setup packet structure (inclusion of PHY hdr depends on statlen)*/
  osf_buf_phy = (osf_pkt_phy_t *)&osf_buf[0];
  osf_buf_hdr = (osf_pkt_hdr_t *)&osf_buf[OSF_PKT_PHY_LEN(osf.rconf->phy->mode, osf.round->statlen)];
  osf_buf_rnd_pkt = (void *)&osf_buf[OSF_PKT_PHY_LEN(osf.rconf->phy->mode, osf.round->statlen) + OSF_PKT_HDR_LEN];

  /* Send data according to round rules. Might be called in process context at
     join phase  */
  int_master_status_t stat = 0;
  if (!isInterrupt()) {
    stat = critical_enter();
  }
  uint8_t len = osf.round->send();
  // if(len && (osf.round->type == OSF_ROUND_T) && !osf.round->statlen && node_is_joined) {
  if(len) {
    osf_stat.osf_mac_tx_total++; /* Statistics */
  }
  if (!isInterrupt()) {
    critical_exit(stat);
  }

  osf_buf_len = OSF_PKT_HDR_LEN + (osf.round->statlen ? OSF_PKT_RND_LEN(osf.round->type) : len);

  /* Do extension */
  // DEBUG_GPIO_ON(DBG_PIN2);
  DO_OSF_D_EXTENSION(start, osf.round->type, osf.round->is_initiator, OSF_PKT_RND_LEN(osf.round->type));
  // DEBUG_GPIO_OFF(DBG_PIN2);

  /* Check we aren't trying to send more than the MTU can handle */
  if(osf_buf_len <= OSF_MAXLEN(osf.rconf->phy->mode)) {
    /* Re-initialise the radio for this round's PHY conf */
    my_radio_init(osf.rconf->phy, &osf_buf[0], osf_buf_len, osf.round->statlen, osf.round->type);
    /* Schedule all rounds, slots, etc from our epoch ref */
    t_round_start = osf.t_epoch_ref + osf.rconf->t_offset;
    t_round_end = t_round_start + osf.rconf->duration;
    /* Init 'random' channel hopping seed */
    if(node_is_synced) {
      osf_ch_init_index(osf.epoch + osf.proto->index);
    } else {
      osf_ch_init_scan_index();
    }

    /* Start the slots */
    do_slot();
  } else {
    LOG_ERR("osf_buf_len (%u) > OSF_MAXLEN (%u)!\n", osf_buf_len, OSF_MAXLEN(osf.rconf->phy->mode));
    end_round();
  }
}

/*---------------------------------------------------------------------------*/
static void
end_round() {
  /* Receive data according to round rules */
  if(!osf.round->is_initiator && (osf.n_rx_ok || osf.n_rx_crc)) {
    osf.n_rnd_since_rx = 0;
    if(osf.n_rx_ok) {
      osf.round->receive();
    } else {
      osf.round->no_rx();
    }
  } else {
    osf.n_rnd_since_rx++;
    osf.round->no_rx();
  }

  /* Clear the buffer */
  memset(osf_buf, 0, sizeof(osf_buf_t));

  /* Free the handler for other MACs */
  // FIXME: Radio stuff should really not go here
  NVIC_DisableIRQ(RADIO_IRQn);
  nrf_radio_int_disable(0xFFFFFFFF);
  NVIC_ClearPendingIRQ(RADIO_IRQn);
  nrf52840_radioirq_register_handler(NULL);

  /* Next round */
  osf.proto->index++;
  if ((osf.rconf = osf.proto->next_round()) != NULL) {
    start_round();
  /* Rounds have finished, schedule next epoch */
  } else {
    osf.n_rnd_since_rx = 0;
    /* Schedule first round of next epoch */
    schedule_epoch();
    /* Print logs */
    process_poll(&osf_post_epoch_process);
  }
}

/*---------------------------------------------------------------------------*/
/* Post epoch process to do stuff outside of the ISR context */
PROCESS_THREAD(osf_post_epoch_process, ev, ev_data)
{
  PROCESS_BEGIN();

  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    DEBUG_GPIO_ON(DBG_PIN3); // 80us - empty loop

    /* Print logs */
#if OSF_LOGGING
    osf_buf_log_print();
    osf_log_print();
    osf_buf_log_init();
#endif

    /* One more check if data in buffer */
    if (osf_buf_rx_length()) {
      process_poll(&osf_post_round_process);
    }

    DEBUG_GPIO_OFF(DBG_PIN3);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/* Post round process to do stuff outside of the ISR context */
PROCESS_THREAD(osf_post_round_process, ev, ev_data)
{
  PROCESS_BEGIN();

  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    DEBUG_GPIO_ON(DBG_PIN3); // 100 us; 1,2ms ; 5ms (ping 1232); 7.5ms (UDP 1232)
    /* If data in FIFO buffer -> push it to App */
    if (osf_buf_rx_length()) {
        osf_buf_element_t *el = osf_buf_rx_peek();
        if (el != NULL) {
          osf_receive(el->src, el->dst, el->data, el->len);
          int_master_status_t stat = critical_enter();
          osf_buf_rx_remove_head();
          critical_exit(stat);
        }
    }

    /* If data in buffer ? */
    if (osf_buf_rx_length()) {
      process_poll(&osf_post_round_process);
    }

    if (osf_buf_rx_length() > 2*OSF_NTX) {
      LOG_WARN("RX queue %d elements !\r\n", osf_buf_rx_length());
    }

    if (osf_buf_tx_length() > 2*OSF_NTX) {
      LOG_WARN("TX queue %d elements !\r\n", osf_buf_tx_length());
    }

    DEBUG_GPIO_OFF(DBG_PIN3);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/* Radio IRQ Handler for RoF state machine */
/*---------------------------------------------------------------------------*/
// static rtimer_clock_t t_exp_ev_addr_ts = 0;
static void
RADIO_IRQHandler_callback()
{
  DEBUG_GPIO_ON(RADIO_IRQ_EVENT_PIN); // Spikes READY | ADDR | END fo r debug
  /* READY Event */
  if (NRF_RADIO->EVENTS_READY) {
    NRF_RADIO->EVENTS_READY = 0;
    t_ev_ready_ts = NRF_TIMERX->CC[TIMESTAMP_REG];
    // t_exp_ev_addr_ts = t_ev_ready_ts + osf_phy_conf->header_air_ticks + US_TO_RTIMERTICKS(20);
    if (osf_state == OSF_STATE_RECEIVED) {
      /* If we are an initiator about to transmit, or a receiver who has received,
         then kick us into a transmitting state */
      osf_state = OSF_STATE_TRANSMITTING;
      NETSTACK_PA.tx_on();
    } else if (osf_state == OSF_STATE_WAITING){
      /* If we are receiving, set the channel timeout */
      if(osf.round->primitive == OSF_PRIMITIVE_ROF || !osf.n_rx_ok) {
        /* RoF || Glossy before 1st RX - we want to stay in RX and hop halfway between ADDR and END */
        ch_timeout_set = schedule_ch_timeout(t_ev_ready_ts);
      } else {
        /* Glossy - we want to stop RX just after where we expect address */
        schedule_slot_timeout(t_ev_ready_ts);
      }
      NETSTACK_PA.rx_on();
    }
  }
  /* ADDRESS Event */
  else if (NRF_RADIO->EVENTS_ADDRESS) {
  // FIXME: See nrf52840-osf.c
  // else if (((osf_phy_conf->mode != PHY_IEEE) && NRF_RADIO->EVENTS_ADDRESS)
  //           || ((osf_phy_conf->mode == PHY_IEEE) && NRF_RADIO->EVENTS_FRAMESTART)) {
    NRF_RADIO->EVENTS_ADDRESS = 0;
    NRF_RADIO->EVENTS_FRAMESTART = 0;
    t_ev_addr_ts = NRF_TIMERX->CC[TIMESTAMP_REG];
    /* If we are a receiver who is waiting, we now have an base addr + prefix
       (or SFD if we are IEEE 802.15.4). Start to receive. */
    if (osf_state == OSF_STATE_WAITING) {
      // TODO: If we don't get an address when we expect, we should ignore it.
      // rtimer_clock_t ready_to_addr = (t_ev_addr_ts - t_ev_ready_ts);
      // rtimer_clock_t expected = osf_phy_conf->header_air_ticks + osf_phy_conf->post_addr_air_ticks + OSF_RX_GUARD + US_TO_RTIMERTICKSX(20);
      // if (!node_is_synced || (node_is_synced && RTIMER_CLOCK_LT(ready_to_addr, expected))) {
        rtimerx_clear(); // stop channel timeouts
        ch_timeout_set = 0;
        // start RSSI measurement
        osf_state = OSF_STATE_RECEIVING;
        /* We need to set ANOTHER timer to kill our RX if we don't get EVENTS_END
           this is an edge case where we end up spanning two (or more) slots
           before we get END */
        rx_timeout_set = schedule_rx_timeout(t_ev_addr_ts);
      // }
    }
  }
  /* END Event */
  else if (NRF_RADIO->EVENTS_END) {
    NRF_RADIO->EVENTS_END = 0;

    NETSTACK_PA.off();

    t_ev_end_ts = NRF_TIMERX->CC[TIMESTAMP_REG];
    /* We have been receiving, end RX. The next TX/RX will be scheduled by the
       next round which is called by end_rx() */
    if (osf_state == OSF_STATE_RECEIVING) {
      rtimerx_clear(); // stop rx timeout
      rx_timeout_set = 0;
      end_rx();
    /* WE have been transmitting. end TX. The next TX/RX will be scheduled by the
       next round which is called by end_tx() */
    } else if (osf_state == OSF_STATE_TRANSMITTING) {
      osf_state = OSF_STATE_WAITING;
      end_tx();
    } else {
      //LOG_ERR("Should NEVER be here. We must be RECEIVING or TRANSMITTING!\n");
    }
  }
  DEBUG_GPIO_OFF(RADIO_IRQ_EVENT_PIN);
}

/*---------------------------------------------------------------------------*/
/* Public functions for netstack dtiver */
/*---------------------------------------------------------------------------*/
void
osf_configure(uint8_t *sources, uint8_t src_len,
              uint8_t *destinations, uint8_t dst_len,
              uint8_t *border_routers, uint8_t br_len)
{
  /* Copy over static sources, destinations and border routers */
  memcpy(osf.sources, sources, src_len);
  osf.src_len = src_len;
  memcpy(osf.destinations, destinations, dst_len);
  osf.dst_len = dst_len;
  memcpy(osf.border_routers, border_routers, br_len);
  osf.br_len = br_len;

  /* let ourselves know what node type we are */
  uint8_t i;
  for(i = 0; i < osf.src_len; i++) {
    if(osf.sources[i] == node_id) {
      node_is_source = 1;
      break;
    }
  }
  for(i = 0; i < osf.dst_len; i++) {
    if(osf.destinations[i] == node_id) {
      node_is_destination = 1;
      break;
    }
  }
  for(i = 0; i < osf.br_len; i++) {
    if(osf.border_routers[i] == node_id) {
      node_is_br = 1;
      break;
    }
  }
}

/*---------------------------------------------------------------------------*/
void
osf_init()
{
  LOG_INFO("Init OSF...\n");
  /* Initialise channel hopping */
  osf_ch_init();
  /* Initialise MAC buffer */
  osf_buf_init();
  /* Print current config */
  print_osf_config();
  print_osf_timings();
  /* Configure GPIO pins for Radio profiling */
  osf_debug_configure_pins();
  osf_debug_clear_pins();
  osf_debug_gpio_init();
  /* Initialize runtime statistics */
  osf_stat_init();
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_send(uint8_t *data, uint8_t len, uint8_t dst)
{
  return osf_buf_tx_put(data, len, dst);
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_receive(uint8_t src, uint8_t dst, uint8_t *data, uint8_t len)
{
  receive_callback(data, len);
  return 1;
}

/*---------------------------------------------------------------------------*/
void
osf_register_input_callback(osf_input_callback_t cb)
{
  LOG_INFO("Register a receive callback...\n");
  receive_callback = cb;
}

/*---------------------------------------------------------------------------*/
static int
osf_on()
{
  /* Avoid reinitialization without shutdown */
  if(osf_is_on) {
    return 1;
  }
  /* Set the timesync */
  set_timesync();
  /* Initialize osf radio timer */
  rtimerx_init();
  /* Initialize protocol */
  osf.proto = OSF_GET_PROTO(OSF_PROTOCOL);
  osf.proto->init();
  /* Initialize Protocol extensions */
  DO_OSF_P_EXTENSION(init);
  /* Initialise period */
  rtimer_clock_t min_period = OSF_PRE_EPOCH_GUARD + osf.proto->duration + OSF_POST_EPOCH_GUARD;
  if(OSF_PERIOD < min_period) {
    osf.period = min_period;
    LOG_WARN("OSF_PERIOD < min_period! Period limited to %luus - pre:%lu d:%lu post:%lu\n",
    RTIMERTICKS_TO_USX(osf.period),
    RTIMERTICKS_TO_USX(OSF_PRE_EPOCH_GUARD),
    RTIMERTICKS_TO_USX(osf.proto->duration),
    RTIMERTICKS_TO_USX(OSF_POST_EPOCH_GUARD));
  } else {
    osf.period = OSF_PERIOD;
  }
  /* Initialise driver extensions */
  DO_OSF_D_EXTENSION(init);
  /* Initialize logging */
  osf_log_init();
  /* OSF is now ON */
  osf_is_on = 1;

  /* Debug info */
  LOG_INFO("- NVIC_GetPriority(RADIO_IRQn)  - %lu \n", NVIC_GetPriority(RADIO_IRQn));  // 0
  LOG_INFO("- NVIC_GetPriority(TIMERX_IRQn) - %lu \n", NVIC_GetPriority(TIMERX_IRQn)); // 1
  LOG_INFO("- NVIC_GetPriority(USBD_IRQn)   - %lu \n", NVIC_GetPriority(USBD_IRQn));   // NRFX_USBD_CONFIG_IRQ_PRIORITY 6
  NVIC_SetPriority(TIMER0_IRQn, 2);
  LOG_INFO("- NVIC_GetPriority(TIMER0_IRQn) - %lu \n", NVIC_GetPriority(TIMER0_IRQn)); // 2
  NVIC_SetPriority(RTC0_IRQn, 5);
  LOG_INFO("- NVIC_GetPriority(RTC0_IRQn)   - %lu \n", NVIC_GetPriority(RTC0_IRQn));   // 5

  /* Start the post round process (e.g., for put data to App) */
  process_start(&osf_post_round_process, NULL);
  /* Start the post epoch process (e.g., for logging)*/
  process_start(&osf_post_epoch_process, NULL);
  /* Set time for first round to start */
  osf.t_epoch_ref = RTIMERX_NOW();
  /* Schedule the first epoch (will continute running in an ISR context
     until osf_off is called) */
  schedule_epoch();
  return 1;
}

/*---------------------------------------------------------------------------*/
static int
osf_off(void)
{
  osf_is_on = 0;
  return 1;
}


/*---------------------------------------------------------------------------*/
const struct mac_driver osf_driver = {
  "OSF",
  osf_init,
  NULL,
  NULL,
  osf_on,
  osf_off,
  NULL
};

/*---------------------------------------------------------------------------*/
/* OSF API functions */
/*---------------------------------------------------------------------------*/
rtimer_clock_t
osf_get_reference_time()
{
  return t_ref;
}

/*---------------------------------------------------------------------------*/
/* Printing */
/*---------------------------------------------------------------------------*/
static void
print_osf_config()
{
  /* Info */
  LOG_INFO("OSF Config... (INFO)\n");
#ifdef OSF_PHY
  LOG_INFO("- OSF_CONF_PHY                 - %s\n", OSF_PHY_TO_STR(OSF_CONF_PHY));
#endif
  LOG_INFO("- OSF_TS                       - %u\n", OSF_TS);
  LOG_INFO("- OSF_NTX                      - %u\n", OSF_NTX);
  LOG_INFO("- OSF_MAX_MAX_SLOTS            - %u\n", OSF_MAX_MAX_SLOTS);
  LOG_INFO("- OSF_MAX_NODES                - %u\n", OSF_MAX_NODES);
  LOG_INFO("- OSF_TXPOWER                  - %s\n", OSF_TXPOWER_TO_STR(OSF_TXPOWER));
  LOG_INFO("- OSF_PROTOCOL:                - %s\n", OSF_PROTO_TO_STR(OSF_PROTOCOL));
  LOG_INFO("- OSF_PROTO_EXTENSION:         - %s\n", (osf_p_extension != NULL ? osf_p_extension->name : "NONE"));
  LOG_INFO("- OSF_DRIVER_EXTENSION:        - %s\n", (osf_d_extension != NULL ? osf_d_extension->name : "NONE"));
  /* Debug */
  LOG_INFO("OSF Config... (DEBUG)\n");
  LOG_INFO("- OSF_PKT_PHY_LEN              - %u bytes\n", OSF_PKT_PHY_LEN(osf.rconf->phy->mode, osf.round->statlen));
  LOG_INFO("- OSF_PKT_HDR_LEN              - %u bytes\n", OSF_PKT_HDR_LEN);
  LOG_INFO("- OSF_DATA_LEN_MAX             - %u bytes\n", OSF_DATA_LEN_MAX);
  LOG_INFO("- OSF_BITMASK_LEN              - %u bytes\n", OSF_BITMASK_LEN);
}

/*---------------------------------------------------------------------------*/
static void
print_osf_timings()
{
  LOG_INFO("OSF PHY Timings... (DEBUG)\n");
  LOG_INFO("- RADIO_RAMPUP_TIME            - %u (%luus)\n", RADIO_RAMPUP_TIME, RTIMERTICKS_TO_USX(RADIO_RAMPUP_TIME));
  LOG_INFO("- OSF_TIFS_TICKS               - %llu ticks | %lu us\n", OSF_TIFS_TICKS, RTIMERTICKS_TO_USX(OSF_TIFS_TICKS));
  LOG_INFO("- OSF_PRE_EPOCH_GUARD          - %llu (%luus)\n", OSF_PRE_EPOCH_GUARD, RTIMERTICKS_TO_USX(OSF_PRE_EPOCH_GUARD));
  LOG_INFO("- OSF_ROUND_GUARD              - %llu (%luus)\n", OSF_ROUND_GUARD, RTIMERTICKS_TO_USX(OSF_ROUND_GUARD));
  LOG_INFO("- OSF_RX_GUARD                 - %llu (%luus)\n", OSF_RX_GUARD, RTIMERTICKS_TO_USX(OSF_RX_GUARD));
  LOG_INFO("- OSF_REF_SHIFT                - %lu ticks | %lu us\n", OSF_REF_SHIFT, RTIMERTICKS_TO_USX(OSF_REF_SHIFT));
}
