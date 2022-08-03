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
 *         OSF STA protocol.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
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
#define LOG_MODULE "OSF-STA"
#define LOG_LEVEL LOG_LEVEL_DBG

#if OSF_MPHY
uint8_t osf_mphy_pattern_100[] = {PHY_BLE_2M,   PHY_BLE_2M,   PHY_BLE_2M,   PHY_BLE_2M};
// uint8_t osf_mphy_pattern_75[]  = {PHY_BLE_2M,   PHY_BLE_125K, PHY_BLE_2M,   PHY_BLE_2M};
// uint8_t osf_mphy_pattern_50[]  = {PHY_BLE_2M,   PHY_BLE_125K, PHY_BLE_2M,   PHY_BLE_125K};
// uint8_t osf_mphy_pattern_25[]  = {PHY_BLE_2M,   PHY_BLE_125K, PHY_BLE_125K, PHY_BLE_125K};
// uint8_t osf_mphy_pattern_0[]   = {PHY_BLE_125K, PHY_BLE_125K, PHY_BLE_125K, PHY_BLE_125K};
uint8_t osf_mphy_pattern_75[]  = {PHY_BLE_2M,   PHY_BLE_500K, PHY_BLE_2M,   PHY_BLE_2M};
uint8_t osf_mphy_pattern_50[]  = {PHY_BLE_2M,   PHY_BLE_500K, PHY_BLE_2M,   PHY_BLE_500K};
uint8_t osf_mphy_pattern_25[]  = {PHY_BLE_2M,   PHY_BLE_500K, PHY_BLE_500K, PHY_BLE_500K};
uint8_t osf_mphy_pattern_0[]   = {PHY_BLE_500K, PHY_BLE_500K, PHY_BLE_500K, PHY_BLE_500K};
uint8_t osf_mphy_pattern = OSF_MPHY_PATTERN_75;
uint8_t *osf_mphy;
clock_time_t mphy_last_received[255] = {0};

// static uint16_t rcvd_2m = 0;
// static uint16_t total_2m = 0;
// static uint16_t rcvd_125K = 0;
// static uint16_t total_125k = 0;
#endif

static uint8_t       my_bit_index;
static osf_proto_t  *this = &osf_proto_sta;

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

#if OSF_MPHY
  if(node_is_timesync) {
    uint8_t node, i, new_mphy_pattern;
    clock_time_t now = clock_time();
    uint8_t late = 0;
    int percentage_late;
    // HACK for D-Cube! D-Cube doesn't start sending messages until 1min in!
    if(osf.epoch < 55) {
      for(i = 0; i < 255; i++) {
        mphy_last_received[i] = now;
      }
    }
    /* Calculate percentage of nodes received within the last 30s (i.e. D-Cube Aperiodic) */
    for (i = 0; i <  osf.src_len; i++) {
      node = osf.sources[i];
      if(CLOCK_LT(mphy_last_received[node], now) && (now - mphy_last_received[node] >= (CLOCK_SECOND * MPHY_THRESHOLD_SECONDS))) {
        late++;
        // LOG_INFO("%u late! (%u/%u)\n", node, late, osf.src_len);
      }
    }
    percentage_late = (late*100)/(osf.src_len);
    // if(percentage_late <= 5) {
    //   new_mphy_pattern = OSF_MPHY_PATTERN_100;
    // } else
    if (percentage_late <= 25) {
      new_mphy_pattern = OSF_MPHY_PATTERN_75;
    } else if (percentage_late <= 50) {
      new_mphy_pattern = OSF_MPHY_PATTERN_50;
    } else if (percentage_late <= 75) {
      new_mphy_pattern = OSF_MPHY_PATTERN_25;
    } else {
      new_mphy_pattern = OSF_MPHY_PATTERN_0;
    }
    osf_mphy_pattern = new_mphy_pattern;
    LOG_WARN("pat:%s%% (%u=%u/%u)!\n", OSF_MPHY_PATTERN_TO_STR(osf_mphy_pattern), percentage_late, late, osf.src_len);
  }
  uint8_t *osf_mphy = OSF_GET_MPHY_PATTERN(osf_mphy_pattern);
#endif

  /* Init S round */
  rconf = &this->sched[this->index];
  rconf->t_offset = 0;
  osf_round_configure(rconf, rconf->round, my_radio_get_phy_conf(OSF_ROUND_S_PHY), rconf->ntx + OSF_ROUND_S_NTX, OSF_ROUND_S_MAX_SLOTS);
  this->duration += rconf->duration + OSF_ROUND_GUARD;
  if(!osf_is_on) {
    osf_round_conf_print(rconf, rconf->round);
  }
#if OSF_PROTO_STA_EMPTY
  rconf->is_last = 0;
#endif

  for(i = 0; i < OSF_PROTO_STA_N_TA; i++) {
#if OSF_MPHY
    uint8_t mode = osf_mphy[(i % OSF_MPHY_PATTERN_LEN)];
#else
    uint8_t mode = OSF_ROUND_T_PHY;
#endif

    /* Init T round */
    rconf = &this->sched[++this->index];
    rconf->t_offset = this->duration;
    osf_round_configure(rconf, rconf->round, my_radio_get_phy_conf(mode), rconf->ntx + OSF_ROUND_T_NTX, OSF_ROUND_S_MAX_SLOTS);
    this->duration += rconf->duration + OSF_ROUND_GUARD;
    if(!osf_is_on && !i) {
      osf_round_conf_print(rconf, rconf->round);
    }
#if OSF_PROTO_STA_EMPTY
    rconf->is_last = 0;
#endif


#if !OSF_MPHY
    mode = OSF_ROUND_A_PHY;
#endif
    /* Init A round */
    rconf = &this->sched[++this->index];
    rconf->t_offset = this->duration;
    osf_round_configure(rconf, rconf->round, my_radio_get_phy_conf(mode), rconf->ntx + OSF_ROUND_A_NTX, OSF_ROUND_S_MAX_SLOTS);
    this->duration += rconf->duration + OSF_ROUND_GUARD;
    if(!osf_is_on && !i) {
      osf_round_conf_print(rconf, rconf->round);
    }
#if OSF_PROTO_STA_EMPTY
    rconf->is_last = 0;
#endif
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
  osf_round_conf_t *rconf = &this->sched[this->index];

  /* Set a bit index for this node */
  my_bit_index = deployment_index_from_id(node_id);

  /* Set up the protocol schedule */
  rconf->round = &osf_round_s;
  for(i = 0; i < OSF_PROTO_STA_N_TA; i++) {
      rconf = &this->sched[++this->index];
      rconf->round = &osf_round_tx;
      rconf = &this->sched[++this->index];
      rconf->round = &osf_round_a;
  }
  this->len = this->index + 1; // note our protocol length
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

#if OSF_PROTO_STA_EMPTY
  osf_round_conf_t *prev_round = &this->sched[this->index-1];
  if(prev_round->is_last == 1)
  {
    memset(this->received, 0, sizeof(this->received));
    memset(this->sent, 0, sizeof(this->sent));
    this->index = this->len;
  }
#endif

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
        // if you have data, you are a SRC
        // if you are a known static dst, you are a DST
        // if the dst field is your id, you are a DST <-- done in round
        // all other nodes are FWD
        this->role = osf_buf_tx_length() ? OSF_ROLE_SRC : (node_is_destination ? OSF_ROLE_DST : OSF_ROLE_FWD);
        break;
      /* Configure A round */
      case OSF_ROUND_A:
        // if you received data in the T, you are a SRC
        // if you sent data in the T, you are a DST
        // all other nodes are FWD
#if OSF_PROTO_STA_EMPTY
        if(node_is_destination) {
          if((osf.n_rnd_since_rx > OSF_PROTO_STA_EMPTY)
#if OSF_EXT_ND
            || ((noise_detection_action((&this->sched[this->index - 1])->round->type, node_is_destination) == false) && (node_is_destination))
#endif
          )
          {
            rconf->is_last = 1;
            this->received[this->index-1] = OSF_PROTO_CODE_STA_EXIT;
          }
        }
#endif
        this->role = this->received[this->index-1] ? OSF_ROLE_SRC : (this->sent[this->index-1] ? OSF_ROLE_DST : OSF_ROLE_FWD);
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
osf_proto_t osf_proto_sta = {
  /* Protocol details */
  OSF_PROTO_STA,      /* type */
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
