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
 *         OSF packet.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

#ifndef OSF_PACKET_H_
#define OSF_PACKET_H_

#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-proto.h"
#include "net/mac/osf/extensions/osf-ext.h"

#define OSF_PACKET_WITH_S1 0

/*---------------------------------------------------------------------------*/
/* NB: Currently using statlen, so LEN and S0 are both 0 */
typedef struct {
  /* In 802154 the len field comes first, and the len also includes the CRC
     bytes */
  uint8_t len;
} ieee_t;

typedef struct {
  uint8_t s0;
  uint8_t len;
#if OSF_PACKET_WITH_S1
  uint8_t s1;
#endif
} ble_t;

typedef union __attribute__((packed)) osf_pkt_phy {
  ieee_t ieee;
  ble_t  ble;
} osf_pkt_phy_t;

// #define OSF_PKT_PHY_LEN(P)         ((P != PHY_IEEE) ? sizeof(ble_t) : sizeof(ieee_t))
#define OSF_PKT_PHY_LEN(P, S)         (((S) ? 0 : ((P != PHY_IEEE) ? sizeof(ble_t) : sizeof(ieee_t))))


/*---------------------------------------------------------------------------*/
/* OSF header */
typedef struct __attribute__((packed)) osf_pkt_hdr {
  uint8_t  slot;
  uint8_t  src;
  uint8_t  dst;
} osf_pkt_hdr_t;
#define OSF_PKT_HDR_LEN            sizeof(osf_pkt_hdr_t)

/*---------------------------------------------------------------------------*/
/* MAX possible packet length. 127 -2 for crc in IEEE */
#define OSF_MAXLEN(P)              ((P != PHY_IEEE) ? 255 : 125)

/* OSF max data length */
#ifdef OSF_CONF_DATA_LEN_MAX
#define OSF_DATA_LEN_MAX            OSF_CONF_DATA_LEN_MAX
#else
#define OSF_DATA_LEN_MAX            248
#endif

/*---------------------------------------------------------------------------*/
/* S round packet */
typedef struct __attribute__((packed)) osf_pkt_s_round {
  uint16_t epoch;
#if OSF_ROUND_S_PAYLOAD
  uint16_t id;
  uint8_t  payload[OSF_DATA_LEN_MAX];
#endif
#if OSF_MPHY
  uint8_t  pattern;
#endif
#if OSF_EXT_BV
  uint8_t  bv_crc[2];
#endif
} osf_pkt_s_round_t;

#define OSF_PKT_S_RND_LEN sizeof(osf_pkt_s_round_t)
/*---------------------------------------------------------------------------*/
/* T round packet */
typedef struct __attribute__((packed)) osf_pkt_t_round {
  uint16_t id;
  uint8_t  payload[OSF_DATA_LEN_MAX];
} osf_pkt_t_round_t;

#define OSF_PKT_T_RND_LEN sizeof(osf_pkt_t_round_t)

/*---------------------------------------------------------------------------*/
/* A round packet */
typedef struct __attribute__((packed)) osf_pkt_a_round {
#if OSF_PROTO_STA_ACK_TOGGLING
  uint8_t  ack[OSF_BITMASK_LEN];
} osf_pkt_a_round_t;
#else
  // Use dst field of osf_round_hdr
} osf_pkt_a_round_t;
#endif

#define OSF_PKT_A_RND_LEN sizeof(osf_pkt_a_round_t)

/*---------------------------------------------------------------------------*/
#define OSF_PKT_RND_LEN(R) \
  ((R == OSF_ROUND_S) ? OSF_PKT_S_RND_LEN : \
   (R == OSF_ROUND_T) ? OSF_PKT_T_RND_LEN : \
   (R == OSF_ROUND_A) ? OSF_PKT_A_RND_LEN : 0)

#define OSF_PKT_RND_LEN_MAX (MAX(OSF_PKT_S_RND_LEN, MAX(OSF_PKT_T_RND_LEN, OSF_PKT_A_RND_LEN)))

#if OSF_SHRINK_ROUND
/* Max reserved AIR time in bytes */
#define OSF_PKT_AIR_MAXLEN(R, P) \
  ((R == OSF_ROUND_S) ? 100 : \
   (R == OSF_ROUND_T) ? (OSF_MAXLEN(P)) : \
   (R == OSF_ROUND_A) ? 12 : 0)
#else
#define OSF_PKT_AIR_MAXLEN(R, P) \
  ((R == OSF_ROUND_S) ? (OSF_MAXLEN(P)) : \
   (R == OSF_ROUND_T) ? (OSF_MAXLEN(P)) : \
   (R == OSF_ROUND_A) ? (OSF_MAXLEN(P)) : 0)
#endif

/*---------------------------------------------------------------------------*/
#define OST_PBUF_N_MAX            (OSF_MAX_MAX_SLOTS/2) // Glossy can have MAX_SLOTS/2 RXs

/* Packet buffer */
typedef struct {
  uint8_t u8[255];
} osf_buf_t;
extern osf_buf_t                   osf_aligned_buf;
/* Macro to access osf_aligned_buf as an array of bytes */
#define osf_buf                 ((uint8_t *)&osf_aligned_buf.u8[0])
extern uint16_t                     osf_buf_len;
/* Pointers to access the various buf sections */
extern osf_pkt_phy_t               *osf_buf_phy;
extern osf_pkt_hdr_t               *osf_buf_hdr;
extern void                        *osf_buf_rnd_pkt;

#endif /* OSF_PACKET_H_ */
