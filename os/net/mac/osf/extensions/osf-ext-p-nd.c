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
 *         OSF noise detection protocol extension.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

#include "contiki.h"
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-log.h"

#include "net/mac/osf/extensions/osf-ext.h"

#if OSF_CONF_EXT_ND
#include "sys/log.h"
#define LOG_MODULE "ND"
#define LOG_LEVEL LOG_LEVEL_INFO

#define CCA_TH  (-60)
#define BUSY_CCA_THRESHOLD  2
#define NOISY_SLOT_NUM      2

typedef struct noise_detection_para {
  uint16_t noisySampleThres;
  uint16_t noisyRoundThres;
  uint16_t noisySampleCnt;
  uint16_t noisyRoundCnt;
} noise_detection_stats_t;

static noise_detection_stats_t noiseStats = {
  .noisyRoundCnt = 0,
  .noisySampleCnt = 0,
  .noisySampleThres = BUSY_CCA_THRESHOLD,
  .noisyRoundThres = NOISY_SLOT_NUM
};

/*---------------------------------------------------------------------------*/
static bool
noise_detection_action(osf_round_type_t type, uint8_t isDst)
{
  if(isDst)
  {
    if(type == OSF_ROUND_T)
    {
      if(noiseStats.noisyRoundCnt < noiseStats.noisyRoundThres)
      {
        if(noiseStats.noisySampleCnt >= noiseStats.noisySampleThres)
        {
          noiseStats.noisyRoundCnt++;
        }else{
          noiseStats.noisyRoundCnt = 0;
        }
      }
    }
  }else{
    if(type == OSF_ROUND_A)
    {
      if(noiseStats.noisyRoundCnt < noiseStats.noisyRoundThres)
      {
        if(noiseStats.noisySampleCnt >= noiseStats.noisySampleThres)
        {
          noiseStats.noisyRoundCnt++;
        }else{
          noiseStats.noisyRoundCnt = 0;
        }
      }
    }
  }

  noiseStats.noisySampleCnt = 0;

  if(noiseStats.noisyRoundCnt >= noiseStats.noisyRoundThres)
    return false;
  else
    return true;
}

/*---------------------------------------------------------------------------*/
static void
configure(osf_proto_t *proto, osf_round_conf_t *rconf)
{
  rconf->phy->noise_threshold = CCA_TH;
  noiseStats.noisySampleCnt = 0;
  noiseStats.noisyRoundCnt = 0;
}

/*---------------------------------------------------------------------------*/
static void
hop()
{
  int8_t rss;
  NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, (radio_value_t *)&rss);
  if(rss > osf.rconf->phy->noise_threshold)
  //if(true)
  {
    noiseStats.noisySampleCnt++;
  }

  // Just for logging
  if(node_is_destination)
  {
    if((osf.proto->sched[osf.proto->index]).round->type == OSF_ROUND_T)
    {
      if(noiseStats.noisyRoundCnt < noiseStats.noisyRoundThres)
      {
        if(noiseStats.noisySampleCnt >= noiseStats.noisySampleThres)
        {
          osf_log_slot_state('N');
        }
      }
    }
  }else{
    if((osf.proto->sched[osf.proto->index]).round->type == OSF_ROUND_A)
    {
      if(noiseStats.noisyRoundCnt < noiseStats.noisyRoundThres)
      {
        if(noiseStats.noisySampleCnt >= noiseStats.noisySampleThres)
        {
          osf_log_slot_state('N');
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
static void
next(osf_proto_t *proto, osf_round_conf_t *rconf)
{
  if(node_is_synced)
  {
    if(proto->index != 0)
    {
      if(!node_is_destination)
      {
        if(noise_detection_action((proto->sched[proto->index - 1]).round->type, node_is_destination) == false)
        {
          slot = NULL;
          memset(proto->received, 0, sizeof(proto->received));
          memset(proto->sent, 0, sizeof(proto->sent));
          proto->index = 0;
        }
      }
    }
  }
}

/*---------------------------------------------------------------------------*/
/* BV extension driver */
/*---------------------------------------------------------------------------*/
osf_ext_p_t osf_ext_p_nd = {
    "osf_nd",
    NULL,
    &configure,
    &hop,
    &next,
};
#endif
