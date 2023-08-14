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

#ifndef OSF_PROTO_H_
#define OSF_PROTO_H_

/*---------------------------------------------------------------------------*/
/* Common */
#ifdef OSF_CONF_ROUND_S_PAYLOAD
#define OSF_ROUND_S_PAYLOAD                    OSF_CONF_ROUND_S_PAYLOAD
#else
#if (OSF_PROTOCOL == OSF_PROTO_BCAST)
#define OSF_ROUND_S_PAYLOAD                    1 // In general, we should always use S payload for broadcast
#else
#define OSF_ROUND_S_PAYLOAD                    0
#endif
#endif

/*---------------------------------------------------------------------------*/
#ifdef OSF_CONF_PROTO_STA_N_TA
#define OSF_PROTO_STA_N_TA                     OSF_CONF_PROTO_STA_N_TA
#else
#define OSF_PROTO_STA_N_TA                     4
#endif
#if ((2*OSF_PROTO_STA_N_TA) >= (OSF_SCHEDULE_LEN_MAX))
#error "ERROR: Number of TA pairs plus S round > OSF_SCHEDULE_LEN_MAX!"
#endif

/*---------------------------------------------------------------------------*/
/* API */
void osf_round_configure(osf_round_conf_t *rconf, osf_round_t *rnd, osf_phy_conf_t *phy, uint8_t ntx, uint8_t max_slots);
void osf_round_conf_print(osf_round_conf_t *rconf, osf_round_t *rnd);
void osf_proto_print(osf_proto_t *proto);

#endif /* OSF_PROTO_H_ */
