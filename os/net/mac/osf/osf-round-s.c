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
 *         OSF sync round.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */


#include "contiki.h"
#include "contiki-net.h"
#include "net/mac/osf/nrf52840-osf.h"

#include "sys/node-id.h"
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-packet.h"
#include "net/mac/osf/osf-buffer.h"
#include "net/mac/osf/osf-log.h"
#include "net/mac/osf/osf-debug.h"
#include "net/mac/osf/osf-stat.h"

#if OSF_MPHY
#include "net/mac/osf/osf-proto.h"
#endif

#if BUILD_WITH_TESTBED
#include "services/testbed/testbed.h"
#endif

#include "sys/log.h"
#define LOG_MODULE "OSF-RND-S"
#define LOG_LEVEL LOG_LEVEL_WARN

static osf_round_t *this = &osf_round_s;
/*---------------------------------------------------------------------------*/
static void
init()
{
  /* N/A */
}

/*---------------------------------------------------------------------------*/
static void
configure()
{
  if(osf.proto->role == OSF_ROLE_SRC) {
    this->is_initiator = 1;
    osf.last_slot_type = OSF_SLOT_R;
  } else {
    this->is_initiator = 0;
    osf.last_slot_type = OSF_SLOT_T;
  }
}

/*---------------------------------------------------------------------------*/
static uint8_t
send()
{
  uint8_t packet_len = 0;
  osf_pkt_s_round_t *rnd_pkt = (osf_pkt_s_round_t *)osf_buf_rnd_pkt;
  /* Send ONLY if we are a TS*/
  if(node_is_timesync) {
    osf_buf_hdr->src = node_id;
    osf_buf_hdr->dst = 0xFF;
    rnd_pkt->epoch = osf.epoch;
    packet_len += sizeof(rnd_pkt->epoch);
#if OSF_ROUND_S_PAYLOAD
    osf_buf_element_t *el = osf_buf_tx_get();
    /* Send data from the MAC buffer */
    if(el != NULL) {
      osf_buf_hdr->dst = el->dst;
      rnd_pkt->id = el->id;
      packet_len += sizeof(rnd_pkt->id);
      memcpy(rnd_pkt->payload, el->data, el->len);
      rnd_pkt += el->len;
      osf.proto->sent[osf.proto->index] = el->dst;
      osf_log_slot_node(osf_buf_hdr->dst);
      return packet_len;
    }
#endif
#if OSF_MPHY
    rnd_pkt->pattern = osf_mphy_pattern;
#else
    return 0;
#endif
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static uint8_t
receive()
{
#if OSF_ROUND_S_PAYLOAD
  osf_pkt_s_round_t *rnd_pkt = (osf_pkt_s_round_t *)osf_buf_rnd_pkt;
  if(osf.proto->role == OSF_ROLE_DST || osf_buf_hdr->dst == node_id || osf_buf_hdr->dst == 0xFF) {
    osf_buf_receive(rnd_pkt->id, osf_buf_hdr->src, osf_buf_hdr->dst, rnd_pkt->payload, OSF_DATA_LEN_MAX, osf_buf_hdr->slot);
    osf.proto->received[osf.proto->index] = osf_buf_hdr->src;
    return 1;
  }
#endif
#if OSF_MPHY
  osf_pkt_s_round_t *rnd_pkt = (osf_pkt_s_round_t *)osf_buf_rnd_pkt;
  osf_mphy_pattern = rnd_pkt->pattern;
  return 1;
#else
  return 0;
#endif
}

/*---------------------------------------------------------------------------*/
static void
no_rx()
{
  if(!node_is_timesync) {
    osf.failed_epochs++;
    osf_stat.osf_ts_lost_total++; /* Statistics */
    /* If we did not sync for N epochs, then desync */
    if(!node_is_timesync && osf.failed_epochs >= OSF_RESYNC_THRESHOLD) {
      DEBUG_LEDS_OFF(SYNCED_LED);
      node_is_synced = 0;
      node_is_joined = 0;
      osf.proto->index = osf.proto->len;
      LOG_WARN("{ep-%u} Resync! Failed epochs %u\n", osf.epoch, osf.failed_epochs);
      was_out_of_sync = 1;
    }
  }
}

/*---------------------------------------------------------------------------*/
/* OSF round data struct */
osf_round_t osf_round_s = {
  /* Round details */
  "osf_round_s",         /* name */
  /* Round constants */
  OSF_ROUND_S,           /* type */
  1,                     /* is sync round */
  OSF_ROUND_S_PRIMITIVE, /* primitive */
  /* Configurable options */
  OSF_ROUND_S_STATLEN,   /* use static length (i.e., no length field) */
  0,                     /* is an initiator */
  /* API */
  &init,                 /* initialization (one-off) */
  &configure,            /* configure before start of round */
  &send,                 /* called when a node sends data */
  &receive,              /* called when a node receives receives data */
  &no_rx
};
