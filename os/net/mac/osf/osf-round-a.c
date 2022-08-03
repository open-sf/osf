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
 *         OSF ack round.
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
#include "net/mac/osf/osf-proto.h"
#include "net/mac/osf/osf-log.h"
#include "net/mac/osf/osf-stat.h"

#if BUILD_WITH_TESTBED
#include "services/testbed/testbed.h"
#endif

#include "sys/log.h"
#define LOG_MODULE "OSF-RND-A"
#define LOG_LEVEL LOG_LEVEL_DBG

static osf_round_t       *this = &osf_round_a;
#if OSF_PROTO_STA_ACK_TOGGLING
static uint8_t            my_ack_offset;
static uint8_t            received[OSF_BITMASK_LEN]  = {0};
static uint8_t            expecting[OSF_BITMASK_LEN] = {1}; // we toggle received from 0->1
#endif
/*---------------------------------------------------------------------------*/
static void
init()
{
#if OSF_PROTO_STA_ACK_TOGGLING
  /* Set our ACK id from a list of destinations. TODO: This should be our own
     LUT similar to deployment.h */
  uint8_t i;
  if(node_is_source) {
    for(i = 0; i < osf.src_len; i++) {
      if(osf.sources[i] == node_id) {
        my_ack_offset = i;
        // osf_log_x("IDX", &i, 1);
      }
    }
  }
#endif
}

/*---------------------------------------------------------------------------*/
static void
configure()
{
  uint8_t last_rx_src = osf.proto->received[osf.proto->index-1];
  if(osf.proto->role == OSF_ROLE_SRC && (last_rx_src || OSF_ROUND_A_ALWAYS_ACK)) {
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
  uint8_t last_rx_src = osf.proto->received[osf.proto->index-1];
  if(osf.proto->role == OSF_ROLE_SRC) {
    osf_buf_hdr->src = node_id;
    osf.proto->sent[osf.proto->index] = last_rx_src;
#if OSF_PROTO_STA_ACK_TOGGLING
    osf_pkt_a_round_t *rnd_pkt = (osf_pkt_a_round_t *)osf_buf_rnd_pkt;
    osf_buf_hdr->dst = 0xFF;
    // TODO: Use something akin to deployment.h mapping so it's just a LUT
    uint8_t i;
    for(i = 0; i < osf.src_len; i++) {
      if((osf.sources[i] == last_rx_src) && osf_buf_ack(last_rx_src)) {
        /* Flip the bit that corresponds to the last rx'd source */
        OSF_TOGGLE_BIT_BYTE(received, i);
        osf_buf_hdr->dst = last_rx_src;
        // osf_log_u("IDX", &i, 1);
        // osf_log_b("ACK", &received, OSF_BITMASK_LEN);
        break;
      }
    }
    memcpy(&rnd_pkt->ack, &received, OSF_BITMASK_LEN);
    return OSF_BITMASK_LEN;
#else
    if(last_rx_src) {
      osf_buf_hdr->dst = last_rx_src;
      return sizeof(osf_pkt_a_round_t);
    }
#if OSF_PROTO_STA_EMPTY
    if(osf.rconf->is_last == 1)
    {
      osf_buf_hdr->dst = OSF_PROTO_CODE_STA_EXIT;
      return sizeof(osf_pkt_a_round_t);
    }
#endif
#endif

  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static uint8_t
receive()
{
  uint8_t last_tx_dst = osf.proto->sent[osf.proto->index-1];
  if(osf.proto->role == OSF_ROLE_DST) {
#if OSF_PROTO_STA_ACK_TOGGLING
    osf_pkt_a_round_t *rnd_pkt = (osf_pkt_a_round_t *)osf_buf_rnd_pkt;
    memcpy(received, &rnd_pkt->ack, OSF_BITMASK_LEN);
    // osf_log_b("RCV", &received, OSF_BITMASK_LEN);
    // osf_log_u("MY", &my_ack_offset, 1);
    // osf_log_b("EXP", &expecting, OSF_BITMASK_LEN);
    uint8_t i;
    for(i = 0; i < osf.dst_len; i++) {
      if((osf.destinations[i] == last_tx_dst)) {
        // osf_log_u("I", &i, 1);
        if(OSF_CHK_BIT_BYTE(received, my_ack_offset) == OSF_CHK_BIT_BYTE(expecting, i)) {
          // osf_log_s("RES", "POP\n");
          osf_buf_tx_remove_head();
          OSF_TOGGLE_BIT_BYTE(expecting, i);
          // osf_log_b("EXP", &expecting, OSF_BITMASK_LEN);
        } else {
          // osf_log_s("RES", "???\n");
        }
      }
    }
#else /* OSF_PROTO_STA_ACK_TOGGLING */
    if(osf_buf_hdr->dst == node_id) {
      osf_buf_tx_remove_head();
      osf.proto->received[osf.proto->index] = last_tx_dst;
      osf_stat.osf_mac_ack_total++; // Statistics
    }
#endif /* OSF_PROTO_STA_ACK_TOGGLING */
  }
#if OSF_PROTO_STA_EMPTY
  else {
    if (osf_buf_hdr->dst == OSF_PROTO_CODE_STA_EXIT) {
      osf.rconf->is_last = 1;
    }
  }
#endif
  return 0;
}

/*---------------------------------------------------------------------------*/
static void
no_rx()
{

}

/*---------------------------------------------------------------------------*/
/* OSF round data struct */
osf_round_t osf_round_a = {
  /* Round details */
  "osf_round_a",         /* name */
  /* Round constants */
  OSF_ROUND_A,           /* type */
  0,                     /* is sync round */
  OSF_ROUND_A_PRIMITIVE, /* primitive */
  /* Configurable options */
  OSF_ROUND_A_STATLEN,   /* use static length (i.e., no length field) */
  0,                     /* is an initiator */
  /* API */
  &init,                 /* initialization (one-off) */
  &configure,            /* configure before start of round */
  &send,                 /* called when a node sends data */
  &receive,              /* called when a node receives receives data */
  &no_rx
};
