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
 *         OSF protocols common functions.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */


#include "contiki.h"
#include "contiki-net.h"
#include "net/mac/osf/nrf52840-osf.h"

#include "sys/node-id.h"
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-proto.h"
#include "net/mac/osf/osf-packet.h"
#include "net/mac/osf/osf-buffer.h"
#include "net/mac/osf/extensions/osf-ext.h"

#if BUILD_WITH_TESTBED
#include "services/testbed/testbed.h"
#endif
#if BUILD_WITH_DEPLOYMENT
#include "services/deployment/deployment.h"
#endif

#include "sys/log.h"
#define LOG_MODULE "OSF-PROTO"
#define LOG_LEVEL LOG_LEVEL_INFO

/*---------------------------------------------------------------------------*/
/* Common round configuration */
void
osf_round_configure(osf_round_conf_t *rconf, osf_round_t *rnd, osf_phy_conf_t *phy, uint8_t ntx, uint8_t max_slots)
{
  if(!osf_is_on) {
    rnd->init();
  }

  rconf->phy = phy;
  rconf->ntx = ntx;
  rconf->max_slots = max_slots;

  rtimer_clock_t slot_duration = my_radio_set_phy_airtime(phy, OSF_PKT_HDR_LEN + OSF_PKT_RND_LEN(rnd->type), rnd->statlen);
  rconf->duration = (slot_duration * rconf->max_slots) - (OSF_TIFS_TICKS); // take off last TIFS + RRU

  /* Protocol extensions */
  DO_OSF_P_EXTENSION(configure, osf.proto, rconf);
}

/*---------------------------------------------------------------------------*/
void
osf_round_conf_print(osf_round_conf_t *rconf, osf_round_t *rnd)
{
  LOG_INFO("--- %s --- \n", OSF_ROUND_TO_STR(rnd->type));
  LOG_INFO("- PHY              - %s\n", OSF_PHY_TO_STR(rconf->phy->mode));
  LOG_INFO("- STATLEN          - %s\n", (rnd->statlen) ? "TRUE" : "FALSE");
  LOG_INFO("- HEADER_AIR_TIME  - %8lu ticks | %6lu us\n", rconf->phy->header_air_ticks, RTIMERTICKS_TO_USX(rconf->phy->header_air_ticks));
  LOG_INFO("- POST_ADDR_TIME   - %8lu ticks | %6lu us\n", rconf->phy->post_addr_air_ticks, RTIMERTICKS_TO_USX(rconf->phy->post_addr_air_ticks));
  LOG_INFO("- PAYLOAD_AIR_TIME - %8lu ticks | %6lu us | %u B %s\n", rconf->phy->payload_air_ticks, \
                                                               RTIMERTICKS_TO_USX(rconf->phy->payload_air_ticks), \
                                                               (!rnd->statlen) ? OSF_MAXLEN(rconf->phy->mode) : OSF_PKT_HDR_LEN + OSF_PKT_RND_LEN(rnd->type), \
                                                               (!rnd->statlen) ? "(MTU for var len packets)" : "" );
  LOG_INFO("- FOOTER_AIR_TIME  - %8lu ticks | %6lu us\n", rconf->phy->footer_air_ticks, RTIMERTICKS_TO_USX(rconf->phy->footer_air_ticks));
  LOG_INFO("- PACKET_AIR_TIME  - %8lu ticks | %6lu us \n", rconf->phy->packet_air_ticks, RTIMERTICKS_TO_USX(rconf->phy->packet_air_ticks));
  LOG_INFO("- TXRX_ADDR_OFFSET - %8lu ticks | %6lu us\n", rconf->phy->tx_rx_addr_offset_ticks, RTIMERTICKS_TO_USX(rconf->phy->tx_rx_addr_offset_ticks));
  LOG_INFO("- TXRX_END_OFFSET  - %8lu ticks | %6lu us\n", rconf->phy->tx_rx_end_offset_ticks, RTIMERTICKS_TO_USX(rconf->phy->tx_rx_end_offset_ticks));
  LOG_INFO("- SLOT_DURATION    - %8lu ticks | %6lu us\n", rconf->phy->slot_duration, RTIMERTICKS_TO_USX(rconf->phy->slot_duration));
  LOG_INFO("- ROUND_DURATION   - %8lu ticks | %6lu us\n", rconf->duration, RTIMERTICKS_TO_USX(rconf->duration));
}

/*---------------------------------------------------------------------------*/
void
osf_proto_print(osf_proto_t *proto)
{
  uint8_t i;
  LOG_INFO("=== %s ===\n", OSF_PROTO_TO_STR(proto->type));
  LOG_INFO("- PROTO LEN        - %u rounds\n", proto->len);
  LOG_INFO("- PROTO DURATION   - %5lu ticks | %4lu us\n", proto->duration, RTIMERTICKS_TO_USX(proto->duration));
  LOG_INFO("- STA EMPTY        - %u\n", OSF_PROTO_STA_EMPTY);
#if OSF_MPHY
  LOG_INFO("- STA MPHY         - %u (%s%%)\n", OSF_MPHY, OSF_MPHY_PATTERN_TO_STR(osf_mphy_pattern));
#endif
  LOG_INFO("=== Proto Ext ===\n");
  LOG_INFO("- NOISE DETECTION  - %u\n", OSF_EXT_ND);
  LOG_INFO("- RANDOM BACKOFF   - %u\n", OSF_EXT_BACKOFF);

  LOG_INFO("=== Driver Ext ===\n");
  LOG_INFO("- RANDOM NTX       - %u\n", OSF_EXT_RNTX);
  LOG_INFO("- SCHEDULE: |");
  for (i = 0; i < proto->len; i++) {
    osf_round_conf_t *rconf = &proto->sched[i];
    uint8_t is_source = OSF_CHK_BIT_BYTE(rconf->sources, deployment_index_from_id(node_id));
    LOG_INFO_("%u:%s-%s-%u", i, OSF_ROUND_TO_STR_SHORT(rconf->round->type), OSF_PHY_TO_STR(rconf->phy->mode), is_source);
    LOG_INFO_("|");
  }
  LOG_INFO_("\n");
}
