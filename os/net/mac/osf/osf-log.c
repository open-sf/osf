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
 *         OSF logging.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */
 
#include "contiki.h"
#include "contiki-net.h"
#include "node-id.h"
#include "lib/queue.h"
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-ch.h"
#include "net/mac/osf/osf-packet.h"
#include "net/mac/osf/osf-debug.h"
#include "net/mac/osf/osf-log.h"

/* TODO: These should not be here. Move to radio driver and use get() functions */
#include "net/mac/osf/nrf52840-osf.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "OSF-LOG"
#define LOG_LEVEL LOG_LEVEL_DBG

#define HEXC(c) (((c) & 0xf) <= 9 ? ((c) & 0xf) + '0' : ((c) & 0xf) + 'a' - 10)

/*---------------------------------------------------------------------------*/
osf_log_stats_t osf_log_stats;

/*---------------------------------------------------------------------------*/
/* Message logs */
#if OSF_LOG_MSG
QUEUE(osf_log_msg_queue);
static uint8_t         log_index_msg;
static osf_log_msg_t   log_array_msg[OSF_LOG_MSG_SIZE_MAX];
#endif

/* Slot logs */
#if OSF_LOG_SLOTS_STATE | OSF_LOG_SLOTS_NODE | OSF_LOG_SLOTS_RSSI | OSF_LOG_SLOTS_TD
static osf_log_slots_t slots[OSF_SCHEDULE_LEN_MAX];
#endif

/* Radio buffer logs */
#if OSF_LOG_LAST_PACKET
static radio_buffer_t  radio_buf[OST_PBUF_N_MAX * OSF_SCHEDULE_LEN_MAX] = {0};
static uint8_t         radio_buf_index = 0;
#endif

/*---------------------------------------------------------------------------*/
void
osf_log_init()
{
#if OSF_LOG_MSG
  log_index_msg = 0;
  queue_init(osf_log_msg_queue);
#endif

#if OSF_LOG_RX_STATS
  memset(&osf_log_stats, 0, sizeof(osf_log_stats));
#endif

#if OSF_LOG_LAST_PACKET
  memset(&radio_buf, 0, sizeof(radio_buf));
  radio_buf_index = 0;
#endif

  uint8_t i;
  for(i = 0; i < OSF_SCHEDULE_LEN_MAX; i++) {
#if OSF_LOG_SLOTS_STATE
    memset(&slots[i].state, '.', OSF_MAX_MAX_SLOTS);
#endif
#if OSF_LOG_SLOTS_NODE
    memset(&slots[i].node, 0, OSF_MAX_MAX_SLOTS);
#endif
#if OSF_LOG_SLOTS_RSSI
    memset(&slots[i].rssi, 0, OSF_MAX_MAX_SLOTS);
#endif
#if OSF_LOG_SLOTS_TD
    memset(&slots[i].td, 0, OSF_MAX_MAX_SLOTS);
#endif
#if OSF_LOG_SLOTS_CH
    memset(&slots[i].ch, 0, OSF_MAX_MAX_SLOTS);
#endif
  }
}

/*---------------------------------------------------------------------------*/
#if OSF_LOG_MSG
osf_log_msg_t *
new_msg_log()
{
  osf_log_msg_t *new;
  if(log_index_msg < OSF_LOG_MSG_SIZE_MAX) {
    /* Get the next available log */
    new = &log_array_msg[log_index_msg];
    /* Push on to the correct stack */
    queue_enqueue(osf_log_msg_queue, new);
    /* Point to the next available buffer space! */
    log_index_msg++;
    /* Return the empty log */
    return new;
  } else {
    LOG_ERR("MSG Log full (%u/%u)!\n", log_index_msg, OSF_LOG_MSG_SIZE_MAX);
    return NULL;
  }
}
#endif /* OSF_LOG_MSG */

/*---------------------------------------------------------------------------*/
void
osf_log_msg(const char *prefix, const char *module, const uint8_t *msg, uint8_t len, uint8_t type)
{
#if OSF_LOG_MSG
  osf_log_msg_t *l = new_msg_log();
  if(l != NULL) {
    l->prefix = prefix;
    l->module = module;
    l->slot = osf.slot;
    l->index = osf.proto->index;
    l->epoch = osf.epoch;
    l->type = type;
    l->len = len;
    memcpy(l->msg, msg, len);
  }
#endif
}

/*---------------------------------------------------------------------------*/
/* Slots */
/*---------------------------------------------------------------------------*/
#if OSF_LOG_SLOTS_STATE
void
osf_log_slot_state(char state)
{
  if(node_is_synced) {
    slots[osf.proto->index].state[osf.slot] = state;
  }
}
#endif

/*---------------------------------------------------------------------------*/
#if OSF_LOG_SLOTS_NODE
void
osf_log_slot_node(uint8_t node)
{
  slots[osf.proto->index].node[osf.slot] = node;
}
#endif

/*---------------------------------------------------------------------------*/
#if OSF_LOG_SLOTS_RSSI
void
osf_log_slot_rssi()
{
  slots[osf.proto->index].rssi[osf.slot] = my_radio_rssi();
}
#endif

/*---------------------------------------------------------------------------*/
#if OSF_LOG_SLOTS_TD
void
osf_log_slot_td()
{
  slots[osf.proto->index].td[osf.slot] = osf.t_slot_drift;
}
#endif

/*---------------------------------------------------------------------------*/
#if OSF_LOG_SLOTS_CH
void
osf_log_slot_ch()
{
  if(node_is_synced) {
    slots[osf.proto->index].ch[osf.slot] = osf_channels[osf_ch_index];
  }
}
#endif

/*---------------------------------------------------------------------------*/
#if OSF_LOG_LAST_PACKET
void
osf_log_radio_buffer(uint8_t *buf, uint8_t len, uint8_t is_tx, uint8_t rnd_pkt_len, uint8_t statlen, uint8_t round)
{
  if(radio_buf_index == osf.proto->index) {
    memcpy(&radio_buf[radio_buf_index].buf, buf, len);
    radio_buf[radio_buf_index].len = len;
    radio_buf[radio_buf_index].is_tx = is_tx;
    radio_buf[radio_buf_index].rnd_pkt_len = rnd_pkt_len;
    radio_buf[radio_buf_index].print_phy = !statlen;
    radio_buf[radio_buf_index].round = round;
    radio_buf[radio_buf_index].crc = NRF_RADIO->RXCRC;
    radio_buf[radio_buf_index].round_idx = osf.proto->index;
    radio_buf[radio_buf_index].slot = osf.slot;
    radio_buf_index++;
  }
}
#endif

/*---------------------------------------------------------------------------*/
#if OSF_LOG_RX_STATS
void
osf_log_rx_end(uint8_t last_crc_is_ok)
{
  osf_log_stats.rx_crc_failed += got_address_event && !last_crc_is_ok;
  osf_log_stats.rx_none += (!got_address_event || !got_end_event) && !last_rx_ok;
}
#endif

/*---------------------------------------------------------------------------*/
/* Printing */
/*---------------------------------------------------------------------------*/
static inline void
print_msg_log()
{
#if OSF_LOG_MSG
  uint8_t i=0, j=0;
  uint8_t maxpow;
  osf_log_msg_t *l;
  while ((l = queue_dequeue(osf_log_msg_queue)) != NULL) {
      printf("[%-4s: %-10s] ", l->prefix, l->module);
      // printf("{%u|ep-%u|r-%u|s-%u} ", node_id, l->epoch, l->index, l->slot);
      switch(l->type) {
        case OSF_LOG_MSG_S:
          /* With %s no need for # of bytes (len) */
          printf("%s", l->msg);
          break;
        case OSF_LOG_MSG_U:
          for(i = 0; i < l->len; i++) {
            printf("%u ", l->msg[i]);
          };
          printf("\n");
          break;
        case OSF_LOG_MSG_D:
          for(i = 0; i < l->len; i++) {
            printf("%u:%d ", i, (int8_t)l->msg[i]);
          };
          printf("\n");
          break;
        case OSF_LOG_MSG_X:
          for(i = 0; i < l->len; i++) {
            // printf("%02x ", l->msg[i]);
            printf("%u ", l->msg[i]);
          };
          printf("\n");
          break;
        case OSF_LOG_MSG_B:
          maxpow = 1<<(l->len*8-1);
          for (i=0; i < l->len; i++) {
            for(j = 0; j < 8; j++) {
              printf("%i", !!(l->msg[i] & maxpow));
              l->msg[i] = l->msg[i] << 1;
            };
            printf(" ");
          }
          printf("\n");
          break;
        default:
          LOG_ERR(" ERROR: Unknown OSF msg type\n");
          return;
      }
  }
#endif /* OSF_LOG_MSG */
}

/*---------------------------------------------------------------------------*/
void
print_slots_state()
{
#if OSF_LOG_SLOTS_STATE
  uint8_t i, j;
  // LOG_INFO("{%u|ep-%-4u} ", node_id, osf.epoch);
  for (i = 0; i < osf.proto->len; i++) {
    for(j = 0; j < osf.proto->sched[i].max_slots; j++) {
#if OSF_LOG_SLOTS_NODE | OSF_LOG_SLOTS_CH
      LOG_INFO_("%2c,", slots[i].state[j]);
#elif OSF_LOG_SLOTS_RSSI | OSF_LOG_SLOTS_TD
      LOG_INFO_("%3c,", slots[i].state[j]);
#else
      LOG_INFO_("%c", slots[i].state[j]);
#endif
    }
    // LOG_INFO_("|");
  }
  LOG_INFO_("\n");
#endif
}

/*---------------------------------------------------------------------------*/
void
print_slots_node()
{
#if OSF_LOG_SLOTS_NODE
  uint8_t i, j;
  LOG_INFO("{%u|ep-%-4u} ", node_id, osf.epoch);
  for (i = 0; i < osf.proto->len; i++) {
    for(j = 0; j < OSF_MAX_MAX_SLOTS; j++) {
      if(slots[i].node[j]) {
        LOG_INFO_("%2x,", slots[i].node[j]);
      } else {
        LOG_INFO_("  ,");
      }
    }
    LOG_INFO_("|");
  }
  LOG_INFO_("\n");
#endif
}

/*---------------------------------------------------------------------------*/
void
print_slots_rssi()
{
#if OSF_LOG_SLOTS_RSSI
  uint8_t i, j;
  LOG_INFO("{%u|ep-%-4u} ", node_id, osf.epoch);
  for (i = 0; i < osf.proto->len; i++) {
    for(j = 0; j < OSF_MAX_MAX_SLOTS; j++) {
      if(slots[i].rssi[j]) {
        LOG_INFO_("%2d,", slots[i].rssi[j]);
      } else {
        LOG_INFO_("   ,");
      }
    }
    LOG_INFO_("|");
  }
  LOG_INFO_("\n");
#endif
}

/*---------------------------------------------------------------------------*/
void
print_slots_td()
{
#if OSF_LOG_SLOTS_TD
  uint8_t i, j;
  LOG_INFO("{%u|ep-%-4u} ", node_id, osf.epoch);
  for (i = 0; i < osf.proto->len; i++) {
    for(j = 0; j < OSF_MAX_MAX_SLOTS; j++) {
      if(slots[i].td[j]) {
        LOG_INFO_("%3ld,", slots[i].td[j]);
      } else {
        LOG_INFO_("   ,");
      }
    }
    LOG_INFO_("|");
  }
  LOG_INFO_("\n");
#endif
}

/*---------------------------------------------------------------------------*/
void
print_slots_ch()
{
#if OSF_LOG_SLOTS_CH
  uint8_t i, j;
  LOG_INFO("{%u|ep-%-4u} ", node_id, osf.epoch);
  for (i = 0; i < osf.proto->len; i++) {
    for(j = 0; j < OSF_MAX_MAX_SLOTS; j++) {
      if(slots[i].ch[j]) {
        LOG_INFO_("%2u,", slots[i].ch[j]);
      } else {
        LOG_INFO_("  ,");
      }
    }
    LOG_INFO_("|");
  }
  LOG_INFO_("\n");
#endif
}

/*---------------------------------------------------------------------------*/
void
print_rx_stats()
{
#if OSF_LOG_RX_STATS
  osf_log_stats.rx_ok_total += osf.n_rx_ok;
  osf_log_stats.berr_total += osf_log_stats.berr;
  osf_log_stats.rx_failed_total += osf_log_stats.rx_crc_failed + osf_log_stats.rx_none;
  osf_log_stats.rx_prr = (osf_log_stats.rx_ok_total*100) / (MAX(1, osf_log_stats.rx_ok_total + osf_log_stats.rx_failed_total));
  LOG_INFO("{rx-%u} ok:%u, ber:%u\n", osf.epoch, osf.n_rx_ok, osf_log_stats.berr);
  LOG_INFO("{stats-%u} total:%lu ok:%lu, crc:%lu, none:%lu | ber_byte:%lu, ber_pkt:%lu, berr_total:%lu | prr:%lu\n",
         osf.epoch, osf_log_stats.rx_ok_total + osf_log_stats.rx_failed_total, osf_log_stats.rx_ok_total, osf_log_stats.rx_crc_failed, osf_log_stats.rx_none,
         osf_log_stats.berr_per_byte_max, osf_log_stats.berr_per_pkt_max, osf_log_stats.berr_total,
         osf_log_stats.rx_prr);
#endif
}

/*---------------------------------------------------------------------------*/
void
print_last_packet()
{
#if OSF_LOG_LAST_PACKET
  uint8_t i, j, offset;
  for (j = 0; j < radio_buf_index; j++) {
    offset = 0;
    radio_buffer_t *rb = &radio_buf[j];
    LOG_INFO_("{%d, %u, %u} %s (%c): %3u ", osf.epoch, rb->round_idx, rb->slot,
      OSF_ROUND_TO_STR(rb->round), (rb->is_tx ? 'T' : 'R'), rb->len);
    /* Print PHY if not using statlen */
    uint8_t phy_len = OSF_PKT_PHY_LEN(osf.rconf->phy->mode, !rb->print_phy);
    if(phy_len) {
      LOG_INFO_("| (%u) ", phy_len);
      for(i = 0; i < phy_len; i++) {
        LOG_INFO_("%02x ", rb->buf[i]);
      }
      offset += phy_len;
    } else {
      LOG_INFO_("| (0)       ");
    }
    /* Print osf header */
    LOG_INFO_("| (%u) ", OSF_PKT_HDR_LEN);
    for(i = offset; i < (offset + OSF_PKT_HDR_LEN); i++) {
      LOG_INFO_("%02x ", rb->buf[i]);
    }
    offset += OSF_PKT_HDR_LEN;
    /* Print round header */
    LOG_INFO_("| (%3u) ", rb->rnd_pkt_len);
    for(i = offset; i < (offset + MIN(rb->rnd_pkt_len, OSF_LOG_PRINT_LEN_MAX)); i++) {
      LOG_INFO_("%02x ", rb->buf[i]);
    }
    offset += rb->rnd_pkt_len;
    // /* Print payload */
    // LOG_INFO_("| (%u) ", rb->len - offset);
    // uint8_t printlen = ((rb->len - offset) > OSF_LOG_PRINT_LEN_MAX ? OSF_LOG_PRINT_LEN_MAX : rb->len);
    // for(i = offset; i < printlen; i++) {
    //   LOG_INFO_("%02x ", rb->buf[i]);
    // }
    if ((rb->len - offset) > OSF_LOG_PRINT_LEN_MAX) {
      LOG_INFO_("... %02x ",  rb->buf[rb->len - 1]);
    }
    LOG_INFO_("[ %lx ]\n", rb->crc);
  }
#endif
}

/*---------------------------------------------------------------------------*/
void
print_heartbeat()
{
#if OSF_LOG_HEARTBEAT
  LOG_INFO("{%u|ep-%-4u} <3\n", node_id, osf.epoch);
#endif
}

/*---------------------------------------------------------------------------*/
void
osf_log_print()
{
  print_rx_stats();
  print_slots_state();
  print_slots_node();
  print_slots_rssi();
  print_slots_td();
  print_slots_ch();
  print_last_packet();
  print_msg_log();
  print_heartbeat();
  /* Clear logs */
  osf_log_init();
}
