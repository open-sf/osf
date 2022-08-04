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
 *         OSF driver and protocol extensions.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

#ifndef OSF_EXT_H_
#define OSF_EXT_H_

#include "net/mac/osf/osf.h"

#define DO_OSF_P_EXTENSION(F, ...)     ((osf_p_extension != NULL && osf_p_extension->F != NULL) ? osf_p_extension->F(__VA_ARGS__) : "")
#define DO_OSF_D_EXTENSION(F, ...)     ((osf_d_extension != NULL && osf_d_extension->F != NULL) ? osf_d_extension->F(__VA_ARGS__) : "")

/*---------------------------------------------------------------------------*/
/* Driver Extensions */

typedef struct osf_driver_ext {
  char          *name;
  void         (*init)();
  void         (*configure)(osf_proto_t *proto);
  void         (*start)(uint8_t rnd_type, uint8_t initiator, uint8_t data_len);
  void         (*tx_ok)();
  void         (*hop)();
  void         (*rx_ok)(uint8_t rnd_type, uint8_t *data, uint8_t data_len);
  void         (*rx_error)();
  void         (*stop)();
} osf_ext_d_t;

/*---------------------------------------------------------------------------*/
#ifdef OSF_CONF_EXT_RNTX
#define OSF_EXT_RNTX              OSF_CONF_EXT_RNTX
#else
#define OSF_EXT_RNTX              0
#endif

#ifdef OSF_CONF_EXT_BV
#define OSF_EXT_BV                OSF_CONF_EXT_BV
#else
#define OSF_EXT_BV                0
#endif

/*---------------------------------------------------------------------------*/

#if OSF_EXT_RNTX
extern osf_ext_d_t                osf_ext_d_rntx;
#define OSF_DRIVER_EXTENSION      &osf_ext_d_rntx
#elif OSF_EXT_BV
extern osf_ext_d_t                osf_ext_d_bv;
#define OSF_DRIVER_EXTENSION      &osf_ext_d_bv
#else
#define OSF_DRIVER_EXTENSION      NULL
#endif

/*---------------------------------------------------------------------------*/
/* Protocol Extensions */

typedef struct osf_proto_ext {
  char          *name;
  void         (*init)();
  void         (*configure)(osf_proto_t *proto, osf_round_conf_t *rconf);
  void         (*hop)();
  void         (*next)(osf_proto_t *proto, osf_round_conf_t *rconf);
} osf_ext_p_t;

extern osf_ext_p_t                *osf_p_extension;

/*---------------------------------------------------------------------------*/
#ifdef OSF_CONF_EXT_ND
#define OSF_EXT_ND                OSF_CONF_EXT_ND
#else
#define OSF_EXT_ND                0
#endif

/*---------------------------------------------------------------------------*/
#ifdef OSF_CONF_EXT_BACKOFF
#define OSF_EXT_BACKOFF           OSF_CONF_EXT_BACKOFF
#else
#define OSF_EXT_BACKOFF           0
#endif

/*---------------------------------------------------------------------------*/
#if OSF_EXT_ND
extern osf_ext_p_t                osf_ext_p_nd;
#define OSF_PROTO_EXTENSION       &osf_ext_p_nd

#elif OSF_EXT_BACKOFF
extern osf_ext_p_t                osf_ext_p_backoff;
#define OSF_PROTO_EXTENSION       &osf_ext_p_backoff

#else
#define OSF_PROTO_EXTENSION       NULL
#endif

#endif /* OSF_EXT_H_ */
