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
 *         OSF transmit round.
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
#include "net/mac/osf/osf-proto.h"
#include "net/mac/osf/osf-buffer.h"
#include "net/mac/osf/osf-log.h"
#include "net/mac/osf/osf-debug.h"
#include "net/mac/osf/osf-stat.h"

#include "sys/log.h"
#define LOG_MODULE "OSF-RND-T"
#define LOG_LEVEL LOG_LEVEL_INFO

static osf_round_t *this = &osf_round_tx;

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
  osf_stat.osf_mac_t_total++; /* Statistics */
}

/*---------------------------------------------------------------------------*/
static uint8_t
send()
{
  uint8_t packet_len = 0;
  osf_buf_element_t *el = osf_buf_tx_get();
  /* Send ONLY if we have TX data and are NOT a TS (TS can send in S round) */
  if(osf.proto->role == OSF_ROLE_SRC && el != NULL) {
    osf_buf_hdr->src = el->src;
    osf_buf_hdr->dst = el->dst;
    osf_pkt_t_round_t *rnd_pkt = (osf_pkt_t_round_t *)osf_buf_rnd_pkt;
    rnd_pkt->id = el->id;
    packet_len += sizeof(rnd_pkt->id);
    memcpy(rnd_pkt->payload, el->data, el->len);
    packet_len += el->len;
    osf.proto->sent[osf.proto->index] = el->dst;
    return packet_len;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static uint8_t
receive()
{
  osf_pkt_t_round_t *rnd_pkt = (osf_pkt_t_round_t *)osf_buf_rnd_pkt;
  if (osf.proto->role == OSF_ROLE_DST || osf_buf_hdr->dst == node_id || osf_buf_hdr->dst == 0xFF) {
    if (this->statlen){
      osf_buf_receive(rnd_pkt->id, osf_buf_hdr->src, osf_buf_hdr->dst, rnd_pkt->payload, OSF_DATA_LEN_MAX, osf_buf_hdr->slot);
    } else {
      /* Store actual payload size */
      osf_buf_receive(rnd_pkt->id, osf_buf_hdr->src, osf_buf_hdr->dst, rnd_pkt->payload, osf_buf_len - sizeof(rnd_pkt->id) - OSF_PKT_HDR_LEN, osf_buf_hdr->slot);
    }
    osf.proto->received[osf.proto->index] = osf_buf_hdr->src;
#if OSF_MPHY
    mphy_last_received[osf_buf_hdr->src] = clock_time();
#endif
    osf_stat.osf_mac_rx_total++; // Statistics
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static void
no_rx()
{

}

/*---------------------------------------------------------------------------*/
/* OSF round data struct */
osf_round_t osf_round_tx = {
  /* Round details */
  "osf_round_tx",        /* name */
  /* Round constants */
  OSF_ROUND_T,           /* type */
  0,                     /* is sync round */
  OSF_ROUND_T_PRIMITIVE, /* primitive */
  /* Configurable options */
  OSF_ROUND_T_STATLEN,   /* use static length (i.e., no length field) */
  0,                     /* is an initiator */
  /* API */
  &init,                 /* initialization (one-off) */
  &configure,            /* configure before start of round */
  &send,                 /* called when a node sends data */
  &receive,              /* called when a node receives receives data */
  &no_rx
};
