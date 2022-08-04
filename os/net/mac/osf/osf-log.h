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
 *         OSF logging.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

#ifndef OSF_LOG_H_
#define OSF_LOG_H_

#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-packet.h"

/*---------------------------------------------------------------------------*/
/* Turn logs off/on */
/*---------------------------------------------------------------------------*/
#ifdef OSF_CONF_LOGGING
#define OSF_LOGGING                   OSF_CONF_LOGGING
#else
#define OSF_LOGGING                   1
#endif

#if OSF_LOGGING
#define OSF_LOG_SLOTS_STATE           1
#define OSF_LOG_SLOTS_NODE            0
#define OSF_LOG_SLOTS_RSSI            0
#define OSF_LOG_SLOTS_TD              0
#define OSF_LOG_SLOTS_CH              0
#define OSF_LOG_LAST_PACKET           1
#define OSF_LOG_RX_STATS              0
#define OSF_LOG_MSG                   1
#define OSF_LOG_HEARTBEAT             0
#else /* OSF_LOGGING */
#define OSF_LOG_SLOTS_STATE           0
#define OSF_LOG_SLOTS_NODE            0
#define OSF_LOG_SLOTS_RSSI            0
#define OSF_LOG_SLOTS_TD              0
#define OSF_LOG_SLOTS_CH              0
#define OSF_LOG_LAST_PACKET           0
#define OSF_LOG_RX_STATS              0
#define OSF_LOG_MSG                   0
#define OSF_LOG_HEARTBEAT             0
#endif /* OSF_LOGGING */

#define OSF_LOG_PRINT_LEN_MAX         20

/*---------------------------------------------------------------------------*/
/* Msg logs */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Radio buffer logs */
/*---------------------------------------------------------------------------*/
typedef struct radio_buffer {
 uint8_t             buf[255];
 uint8_t             len;
 uint8_t             is_tx;
 uint8_t             rnd_pkt_len;
 uint8_t             print_phy;
 uint8_t             round_idx;
 uint8_t             slot;
 uint32_t            crc;
 osf_round_type_t    round;
} radio_buffer_t;

/*---------------------------------------------------------------------------*/
/* Message logs */
/*---------------------------------------------------------------------------*/
// TODO: Ideally this should all be a contiki PR to allow buffering of log
//       messages until you want to print them. It would also be useful for
//       the TSCH logging system
#define OSF_LOG_MSG_SIZE_MAX          100
#define OSF_LOG_MSG_LEN_MAX           255

/* Poor man's approach to %s %u %d %x */
typedef struct osf_log_msg {
  struct osf_log_msg_t  *next;
  const char            *prefix;
  const char            *module;
  uint8_t                slot;
  uint8_t                index;
  uint16_t               epoch;
  enum {
    OSF_LOG_MSG_S,
    OSF_LOG_MSG_U,
    OSF_LOG_MSG_D,
    OSF_LOG_MSG_X,
    OSF_LOG_MSG_B,
  } type;
  uint8_t                msg[OSF_LOG_MSG_LEN_MAX];
  uint8_t                len;
} osf_log_msg_t;

/* Macros to map to the different msg log types */
// TODO: Can maybe look at using LOG_CONF_OUTPUT_PREFIX
#define osf_log_s(prefix, str) do { \
  const char *msg = str; \
  osf_log_msg(prefix, LOG_MODULE, (uint8_t *)msg, strlen(msg), OSF_LOG_MSG_S); \
} while(0);

#define osf_log_u(prefix, var, len) do { \
  osf_log_msg(prefix, LOG_MODULE, (uint8_t *)var, len, OSF_LOG_MSG_U); \
} while(0);

#define osf_log_d(prefix, var, len) do { \
  osf_log_msg(prefix, LOG_MODULE, (uint8_t *)var, len, OSF_LOG_MSG_D); \
} while(0);

#define osf_log_x(prefix, var, len) do { \
  osf_log_msg(prefix, LOG_MODULE, (uint8_t *)var, len, OSF_LOG_MSG_X); \
} while(0);

#define osf_log_b(prefix, var, len) do { \
  osf_log_msg(prefix, LOG_MODULE, (uint8_t *)var, len, OSF_LOG_MSG_B); \
} while(0);

/*---------------------------------------------------------------------------*/
/* Slots */
/*---------------------------------------------------------------------------*/
typedef struct osf_log_slots {
#if OSF_LOG_SLOTS_STATE
  char    state[OSF_MAX_MAX_SLOTS];
#endif
#if OSF_LOG_SLOTS_NODE
  uint8_t node[OSF_MAX_MAX_SLOTS];
#endif
#if OSF_LOG_SLOTS_RSSI
  int8_t  rssi[OSF_MAX_MAX_SLOTS];
#endif
#if OSF_LOG_SLOTS_TD
  int32_t td[OSF_MAX_MAX_SLOTS];
#endif
#if OSF_LOG_SLOTS_CH
  int8_t  ch[OSF_MAX_MAX_SLOTS];
#endif
} osf_log_slots_t;

/*---------------------------------------------------------------------------*/
/* Stats */
/*---------------------------------------------------------------------------*/
typedef struct osf_log_stats {
    uint32_t rx_failed_total;
    uint32_t rx_ok_total;
    uint32_t rx_crc_failed;
    uint32_t rx_prr;
    uint32_t rx_none;
    uint32_t berr_total;
    uint16_t berr;                /* per round */
    uint32_t berr_per_pkt_max;
    uint32_t berr_per_byte_max;
} osf_log_stats_t;

extern osf_log_stats_t osf_log_stats;

/*---------------------------------------------------------------------------*/
#define HEXC(c) (((c) & 0xf) <= 9 ? ((c) & 0xf) + '0' : ((c) & 0xf) + 'a' - 10)

/*---------------------------------------------------------------------------*/
/* API */
/*---------------------------------------------------------------------------*/
void osf_log_init();
void osf_log_print();

#if OSF_LOG_SLOTS_STATE
void osf_log_slot_state(char state);
#else
#define osf_log_slot_state(state)
#endif
#if OSF_LOG_SLOTS_NODE
void osf_log_slot_node(uint8_t node);
#else
#define osf_log_slot_node(node)
#endif
#if OSF_LOG_SLOTS_RSSI
void osf_log_slot_rssi();
#else
#define osf_log_slot_rssi()
#endif
#if OSF_LOG_SLOTS_TD
void osf_log_slot_td();
#else
#define osf_log_slot_td()
#endif
#if OSF_LOG_SLOTS_CH
void osf_log_slot_ch();
#else
#define osf_log_slot_ch()
#endif
#if OSF_LOG_LAST_PACKET
void osf_log_radio_buffer(uint8_t *buf, uint8_t len, uint8_t is_tx, uint8_t rnd_pkt_len, uint8_t statlen, osf_round_type_t round);
void osf_log_test();
#else
#define osf_log_radio_buffer(buf, len, rnd_pkt_len, is_tx, statlen, round)
#endif

void osf_log_msg(const char *prefix, const char *module, const uint8_t *msg, uint8_t len, uint8_t type);
void osf_log_rx_end(uint8_t last_crc_is_ok);

#endif /* OSF_LOG_H_ */
