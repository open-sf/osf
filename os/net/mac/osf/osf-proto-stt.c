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
 *         OSF STT protocol.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 */

#include "contiki.h"
#include "contiki-net.h"
#include "node-id.h"
#include "net/mac/osf/nrf52840-osf.h"

#include "net/mac/osf/osf-debug.h"
#include "net/mac/osf/osf-log.h"
#include "net/mac/osf/osf-packet.h"
#include "net/mac/osf/osf-proto.h"
#include "net/mac/osf/osf-buffer.h"
#include "net/mac/osf/osf.h"

#include "net/mac/osf/extensions/osf-ext.h"

#if BUILD_WITH_TESTBED
#include "services/testbed/testbed.h"
#endif
#if BUILD_WITH_DEPLOYMENT
#include "services/deployment/deployment.h"
#endif

#include "sys/log.h"
#define LOG_MODULE "OSF-STT"
#define LOG_LEVEL LOG_LEVEL_DBG

static uint8_t       my_bit_index;
static osf_proto_t  *this = &osf_proto_stt;

#define UNUSED(x) (void)(x)

/*---------------------------------------------------------------------------*/
static void
configure()
{
  uint8_t i;
  osf_round_conf_t *rconf;

  /* Init vars for start of epoch */
  this->index = 0;
  this->duration = 0;

  /* Init S round */
  rconf = &this->sched[this->index];
  rconf->t_offset = 0;
  osf_round_configure(rconf, rconf->round, my_radio_get_phy_conf(OSF_ROUND_S_PHY), rconf->ntx + OSF_ROUND_S_NTX, OSF_ROUND_S_MAX_SLOTS);
  this->duration += rconf->duration + OSF_ROUND_GUARD;
  if(!osf_is_on) {
    osf_round_conf_print(rconf, rconf->round);
  }

  /* 
   * Init T rounds. We will use deployment mapping for now. 
   * FIXME: Some better way of setting the number of T rounds
   *        based on the number of nodes in the network.
   */
  for(i = 0; i < deployment_node_count(); i++) {
    rconf = &this->sched[++this->index];
    rconf->t_offset = this->duration;
    osf_round_configure(rconf, rconf->round, my_radio_get_phy_conf(OSF_ROUND_T_PHY), rconf->ntx + OSF_ROUND_T_NTX, OSF_ROUND_S_MAX_SLOTS);
    this->duration += rconf->duration + OSF_ROUND_GUARD;
    if(!osf_is_on && !i) {
      osf_round_conf_print(rconf, rconf->round);
    }
  }

  this->duration -= OSF_ROUND_GUARD;
  this->index = 0; // reset index
  memset(this->received, 0, sizeof(this->received));
  memset(this->sent, 0, sizeof(this->sent));

}

/*---------------------------------------------------------------------------*/
static void
init()
{
  uint8_t i;

  /* Set up sync round */
  osf_round_conf_t *rconf = &this->sched[0];
  rconf->round = &osf_round_s;

  /* Set up the TX rounds */
  for(i = 1; i <= deployment_node_count(); i++) {
      rconf = &this->sched[i];
      rconf->round = &osf_round_tx;
  }

  /* Set a bit index for this node */
  my_bit_index = deployment_index_from_id(node_id) + 1; // +1: index starts with 0, but ID starts with 1

  this->len = i; // note our protocol length
  LOG_ERR("Our length is %u. idx is %u\n", i, my_bit_index);
  this->index = 0; // reset index

  /* Configure the protocol (phys, ntx, statlen, etc.) - gives us the (initial) duration */
  configure();

  /* Print the round PHY timings */
  if(!osf_is_on) {
    osf_proto_print(this);
  }
}

/*---------------------------------------------------------------------------*/
static osf_round_conf_t*
next_round()
{
  osf_round_conf_t *rconf = NULL;

  if(this->index < this->len) {
    rconf = &this->sched[this->index];

    switch (rconf->round->type) {
      /* Configure S round */
      case OSF_ROUND_S:
        // if you are a timesync, you are a SRC
        // if you are a known static dst, you are a DST
        // if the dst field is your id, you are a DST <-- done in round
        // all nodes MUST SYNC.
        this->role = node_is_timesync ? OSF_ROLE_SRC : (node_is_destination ? OSF_ROLE_DST : OSF_ROLE_FWD);
        break;
      /* Configure T round */
      case OSF_ROUND_T:
        // if you have data AND IT IS YOUR DESIGNATED SLOT, you are a SRC
        // if you are a known static dst, you are a DST
        // if the dst field is your id, you are a DST <-- done in round
        // all other nodes are FWD
        // FIXME: Need a better way to determine who is in what slot
        this->role = (osf_buf_tx_length() && (my_bit_index == this->index)) ? OSF_ROLE_SRC : (node_is_destination ? OSF_ROLE_DST : OSF_ROLE_FWD);
        break;
      default:
        LOG_ERR("Unknown round type! (%u) %u/%u\n", rconf->round->type, this->index, this->len);
        break;
    }

    /* Protocol Extension */
    DO_OSF_P_EXTENSION(next, this, rconf);

    /* Configure the round */
    rconf->round->configure();

  } else {
    // TODO: Move this to osf_proto_end()
    this->role = OSF_ROLE_NONE;
    memset(this->received, 0, sizeof(this->received));
    memset(this->sent, 0, sizeof(this->sent));
    this->index = 0;
  }

  return rconf;
}

/*---------------------------------------------------------------------------*/
/* OSF round data struct */
osf_proto_t osf_proto_stt = {
  /* Protocol details */
  OSF_PROTO_STT,      /* type */
  0,                  /* protocol duration (depends on phy, ntx, and EXPECTED data length) */
  /* API */
  &init,              /* protocol init */
  &configure,         /* protocol configure */
  &next_round,        /* called to setup next protocol round */
  /* Protocol schedule */
  0,                  /* current index in schedule  */
  0,                  /* protocol schedule length */
  {{0}},              /* protocol schedule */
  {0},
  {0},
  OSF_ROLE_NONE
};
