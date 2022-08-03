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
 *         OSF random backoff protocol extension.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

#include "contiki.h"
#include "node-id.h"
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-packet.h"
#include "net/mac/osf/osf-buffer.h"
#include "net/mac/osf/osf-log.h"

#include "net/mac/osf/extensions/osf-ext.h"

#if OSF_CONF_EXT_BACKOFF
#include "sys/log.h"
#define LOG_MODULE "BACKOFF"
#define LOG_LEVEL LOG_LEVEL_INFO

#define OSF_RAND(var, mod)         do {rand_seed = (rand_seed * 1103515245 + 12345) & UINT32_MAX; var = (rand_seed % mod);} while(0)
static uint32_t rand_seed;

#define OSF_EXT_BACKOFF_THRESHOLD  80

/*---------------------------------------------------------------------------*/
static void
init()
{
  rand_seed = node_id;
}

/*---------------------------------------------------------------------------*/
static void
next(osf_proto_t *proto, osf_round_conf_t *rconf)
{
  uint8_t random;
  uint8_t threshold;
  if (rconf->round->type == OSF_ROUND_T && osf_buf_tx_length()) {
    OSF_RAND(random, 100);
    if((rconf->phy->mode == PHY_BLE_1M) || (rconf->phy->mode == PHY_BLE_2M)) {
      threshold = OSF_EXT_BACKOFF_THRESHOLD;
    } else {
      threshold = 100; // i.e. no backoff
    }
    proto->role = osf_buf_tx_length() ? (random < threshold ? OSF_ROLE_SRC : OSF_ROLE_FWD) : (node_is_destination ? OSF_ROLE_DST : OSF_ROLE_FWD);
  }
}

/*---------------------------------------------------------------------------*/
/* BV extension driver */
/*---------------------------------------------------------------------------*/
osf_ext_p_t osf_ext_p_backoff = {
    "osf_backoff",
    &init,
    NULL,
    NULL,
    &next,
};
#endif
