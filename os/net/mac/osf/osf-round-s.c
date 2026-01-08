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
#include "net/mac/osf/osf-net.h"

#include <string.h>
#if OSF_MPHY
#include "net/mac/osf/osf-proto.h"
#endif

#if BUILD_WITH_TESTBED
#include "services/testbed/testbed.h"
#endif

#include "sys/log.h"
#define LOG_MODULE "OSF-RND-S"
#define LOG_LEVEL LOG_LEVEL_DBG

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
    osf_log_s("DBG","SS\n");
    /* Header */
    osf_buf_hdr->src = node_id;
    osf_buf_hdr->dst = 0xFF;
    rnd_pkt->epoch = osf.epoch;
    packet_len += sizeof(rnd_pkt->epoch);
    /* Joining */
    osf_net_node_t *n = osf_net_get_next_join_request();
    if(n != NULL) {
      LOG_DBG("JR found for node %x\n", n->id);
      rnd_pkt->net_id = OSF_DEFAULT_NETWORK_ID;
      packet_len += sizeof(rnd_pkt->net_id);
      rnd_pkt->net_join_id = n->id;
      packet_len += sizeof(rnd_pkt->net_join_id);
      // linkaddr_copy(&rnd_pkt->net_join_lladdr, &n->lladdr);
      memcpy(&rnd_pkt->net_join_lladdr.u8[0], &n->lladdr.u8[0], LINKADDR_SIZE);
      packet_len += sizeof(rnd_pkt->net_join_lladdr);
      rnd_pkt->net_next_join_epoch = osf_net_get_next_join_epoch();
      packet_len += sizeof(rnd_pkt->net_next_join_epoch);
      // FIXME: This is a terrible way to do this. We need an ACK from the node.
      n->joined = 1;
    }
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
    }
#endif
#if OSF_MPHY
    rnd_pkt->pattern = osf_mphy_pattern;
    packet_len += sizeof(rnd_pkt->pattern);
#endif
    return packet_len;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static uint8_t
receive()
{
  osf_pkt_s_round_t *rnd_pkt = (osf_pkt_s_round_t *)osf_buf_rnd_pkt;
  // FIXME: We don't see anything in the join round as this happens almost immedately. We should only
  //        try and join if we have already sent a join request.
  if (!node_is_joined) {
    osf_log_s("DBG","RS\n");
    /* Temporarily join so we can send our details to the tiemsync. We then check the joining lladdr to see if we have 
       been assigned to the network. */
    // printf("%x\n", linkaddr_node_addr.u8[LINKADDR_SIZE - 1]);
    printf("%x\n", rnd_pkt->net_join_lladdr.u8[LINKADDR_SIZE - 1]);
    // if (linkaddr_cmp(&linkaddr_node_addr, &rnd_pkt->net_join_lladdr)) {
    if (memcmp(&linkaddr_node_addr.u8[0], &rnd_pkt->net_join_lladdr.u8[0], LINKADDR_SIZE) == 0) {
      // FIXME: We need to return the join id and the joiner lladdr.
      printf("J\n");
      printf("id - %x\n", rnd_pkt->net_join_id);
      printf("lladdr - %02X:%02X:%02X:%02X:%02X:%02X\n",
             rnd_pkt->net_join_lladdr.u8[0], rnd_pkt->net_join_lladdr.u8[1],
             rnd_pkt->net_join_lladdr.u8[2], rnd_pkt->net_join_lladdr.u8[3],
             rnd_pkt->net_join_lladdr.u8[4], rnd_pkt->net_join_lladdr.u8[5]);
      osf_net_join(osf_buf_hdr->src, rnd_pkt->net_id);
      return 1;
    }
  }

#if OSF_ROUND_S_PAYLOAD
  if(osf.proto->role == OSF_ROLE_DST || osf_buf_hdr->dst == node_id || osf_buf_hdr->dst == 0xFF) {
    osf_buf_receive(rnd_pkt->id, osf_buf_hdr->src, osf_buf_hdr->dst, rnd_pkt->payload, OSF_DATA_LEN_MAX, osf_buf_hdr->slot);
    osf.proto->received[osf.proto->index] = osf_buf_hdr->src;
    return 1;
  }
#endif
#if OSF_MPHY
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
      DEBUG_LEDS_OFF(JOINED_LED);
      node_is_synced = 0;
      node_is_joined = 0;
      osf.proto->index = osf.proto->len;
      LOG_WARN("{ep-%u} Resync! Failed epochs %u\n", osf.epoch, osf.failed_epochs);
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
