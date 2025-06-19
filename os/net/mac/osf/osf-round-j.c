/*
 * Copyright (c) 2025, Technology Innovation Institute
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
 *         OSF join round.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
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
#include "net/mac/osf/osf-net.h"

#if BUILD_WITH_TESTBED
#include "services/testbed/testbed.h"
#endif

#include "sys/log.h"
#define LOG_MODULE "OSF-RND-J"
#define LOG_LEVEL LOG_LEVEL_DBG

static osf_round_t       *this = &osf_round_j;
/*---------------------------------------------------------------------------*/
static void
init()
{

}

/*---------------------------------------------------------------------------*/
static void
configure()
{
  /* Timesync is always an initiator for join rounds */
  if(node_is_timesync) {
    this->is_initiator = 1;
    osf.last_slot_type = OSF_SLOT_R;
  } else {
    /* Non-joined nodes should try to send join requests */
    if(!node_is_joined) {
      this->is_initiator = 1;
      osf.last_slot_type = OSF_SLOT_R;
    } else {
      this->is_initiator = 0;
      osf.last_slot_type = OSF_SLOT_T;
    }
  }
}

/*---------------------------------------------------------------------------*/
static uint8_t
send()
{
  if(osf.proto->role == OSF_ROLE_SRC) {
    osf_buf_hdr->src = node_id;
    osf_buf_hdr->dst = osf_timesync;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
static uint8_t
receive()
{
  if(osf.proto->role == OSF_ROLE_DST) {
    if(osf_buf_hdr->dst == node_id) {
      // Logic for joining the network
      osf_buf_tx_remove_head();
      osf.proto->received[osf.proto->index] = osf_buf_hdr->src;;
      osf_stat.osf_mac_join_total++; // Statistics
    }
  }
  
  return 1;
}

/*---------------------------------------------------------------------------*/
static void
no_rx()
{

}

/*---------------------------------------------------------------------------*/
/* OSF round data struct */
osf_round_t osf_round_j = {
  /* Round details */
  "osf_round_j",         /* name */
  /* Round constants */
  OSF_ROUND_J,           /* type */
  0,                     /* is sync round */
  OSF_ROUND_J_PRIMITIVE, /* primitive */
  /* Configurable options */
  OSF_ROUND_J_STATLEN,   /* use static length (i.e., no length field) */
  0,                     /* is an initiator */
  /* API */
  &init,                 /* initialization (one-off) */
  &configure,            /* configure before start of round */
  &send,                 /* called when a node sends data */
  &receive,              /* called when a node receives receives data */
  &no_rx
};
