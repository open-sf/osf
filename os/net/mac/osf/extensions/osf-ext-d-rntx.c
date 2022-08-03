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
 *         OSF random NTX driver extension.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

#include "contiki.h"
#include "node-id.h"
#include "net/mac/osf/osf.h"
#include "net/mac/osf/extensions/osf-ext.h"

#if OSF_CONF_EXT_RNTX
#include "sys/log.h"
#define LOG_MODULE "RNTX"
#define LOG_LEVEL LOG_LEVEL_INFO

#define OSF_RAND(var, mod) do { rand_seed = (rand_seed * 1103515245 + 12345) & UINT32_MAX; var = (rand_seed % mod);} while(0)
static uint32_t rand_seed;

/*---------------------------------------------------------------------------*/
static void
init()
{
  rand_seed = node_id;
}

/*---------------------------------------------------------------------------*/
static void
configure(osf_proto_t *proto)
{
  uint8_t i, rand_ntx;
  OSF_RAND(rand_ntx, OSF_NTX);
  osf_round_conf_t *rconf;
  for (i = 0; i < proto->len; i++) {
    slot = &proto->sched[i];
    rconf->ntx = rand_ntx;
  }
}

/*---------------------------------------------------------------------------*/
static void
start(uint8_t rnd_type, uint8_t initiator, uint8_t data_len)
{

}

/*---------------------------------------------------------------------------*/
static void
tx_ok()
{

}

/*---------------------------------------------------------------------------*/
static void
rx_ok(uint8_t rnd_type, uint8_t *data, uint8_t data_len)
{

}

/*---------------------------------------------------------------------------*/
static void
rx_error()
{

}

/*---------------------------------------------------------------------------*/
static void
stop()
{

}

/*---------------------------------------------------------------------------*/
/* BV extension driver */
/*---------------------------------------------------------------------------*/
osf_ext_d_t osf_ext_d_rntx = {
    "osf_rntx",
    &init,
    &configure,
    &start,
    &tx_ok,
    &rx_ok,
    &rx_error,
    &stop,
};
#endif
