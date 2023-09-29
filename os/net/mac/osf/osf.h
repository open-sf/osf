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
 *         OSF main file.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */
 
#ifndef OSF_H_
#define OSF_H_
#include "net/mac/osf/nrf52840-osf.h"

#if BUILD_WITH_TESTBED
#include "services/testbed/testbed.h"
#endif

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y) )
#endif

/* Statically configue the timesync */
#ifdef OSF_CONF_TS
#define OSF_TS                        OSF_CONF_TS
#else
#define OSF_TS                        1
#endif

/* Maximum number of sources / destinations supported in the network. This is
   used in some round calculations when doing bit manipulations. */
#ifdef BUILD_WITH_TESTBED
#define OSF_MAX_NODES                 TB_MAX_SRC_DEST
#elif OSF_CONF_MAX_NODES
#define OSF_MAX_NODES                 OSF_CONF_MAX_NODES
#else
#define OSF_MAX_NODES                 8
#endif


#ifdef OSF_CONF_PERIOD_MS
#define OSF_PERIOD_MS                 OSF_CONF_PERIOD_MS
#else
#define OSF_PERIOD_MS                 500
#endif

/* We can use OSF_CONF_PHY to override default round PHY configuration and
   set the same PHY and use statlen for all round types */
#ifdef OSF_CONF_PHY
#define OSF_CONF_ROUND_S_PHY          OSF_CONF_PHY
#define OSF_CONF_ROUND_T_PHY          OSF_CONF_PHY
#define OSF_CONF_ROUND_A_PHY          OSF_CONF_PHY
#endif

/* Per-round PHY configuration */
#ifdef OSF_CONF_ROUND_S_PHY
#define OSF_ROUND_S_PHY               OSF_CONF_ROUND_S_PHY
#else
#define OSF_ROUND_S_PHY               PHY_BLE_500K
#endif
#ifdef OSF_CONF_ROUND_T_PHY
#define OSF_ROUND_T_PHY               OSF_CONF_ROUND_T_PHY
#else
#define OSF_ROUND_T_PHY               PHY_BLE_2M
#endif
#ifdef OSF_CONF_ROUND_A_PHY
#define OSF_ROUND_A_PHY               OSF_CONF_ROUND_A_PHY
#else
#define OSF_ROUND_A_PHY               PHY_BLE_2M
#endif

/* Per-round STATLEN configuration */
#ifdef OSF_CONF_ROUND_S_STATLEN
#define OSF_ROUND_S_STATLEN           OSF_CONF_ROUND_S_STATLEN
#else
#define OSF_ROUND_S_STATLEN           1
#endif
#ifdef OSF_CONF_ROUND_T_STATLEN
#define OSF_ROUND_T_STATLEN           OSF_CONF_ROUND_T_STATLEN
#else
#define OSF_ROUND_T_STATLEN           1
#endif
#ifdef OSF_CONF_ROUND_A_STATLEN
#define OSF_ROUND_A_STATLEN           OSF_CONF_ROUND_A_STATLEN
#else
#define OSF_ROUND_A_STATLEN           1
#endif

/* Per-round PRIMITIVE configuration */
typedef enum osf_primitive_t {
  OSF_PRIMITIVE_ROF,
  OSF_PRIMITIVE_GLOSSY
} osf_primitive_t;

#ifdef OSF_CONF_PRIMITIVE
#define OSF_CONF_ROUND_S_PRIMITIVE    OSF_CONF_PRIMITIVE
#define OSF_CONF_ROUND_T_PRIMITIVE    OSF_CONF_PRIMITIVE
#define OSF_CONF_ROUND_A_PRIMITIVE    OSF_CONF_PRIMITIVE
#endif

#ifdef OSF_CONF_ROUND_S_PRIMITIVE
#define OSF_ROUND_S_PRIMITIVE         OSF_CONF_ROUND_S_PRIMITIVE
#else
#define OSF_ROUND_S_PRIMITIVE         OSF_PRIMITIVE_ROF
#endif
#ifdef OSF_CONF_ROUND_T_PRIMITIVE
#define OSF_ROUND_T_PRIMITIVE         OSF_CONF_ROUND_T_PRIMITIVE
#else
#define OSF_ROUND_T_PRIMITIVE         OSF_PRIMITIVE_ROF
#endif
#ifdef OSF_CONF_ROUND_A_PRIMITIVE
#define OSF_ROUND_A_PRIMITIVE         OSF_CONF_ROUND_A_PRIMITIVE
#else
#define OSF_ROUND_A_PRIMITIVE         OSF_PRIMITIVE_ROF
#endif

/* Rules for SF primitives */
#define OSF_DOTX_ROF()                (osf.round->is_initiator || osf.last_rx_ok)
#define OSF_DOTX_GLOSSY()             ((osf.round->is_initiator && osf.slot == 0) || (osf.last_slot_type == OSF_SLOT_R && osf.last_rx_ok))

/* NTX determines the number of TXs after a successful RX. E.g., if NTX is 3
   and you RX on the 2nd slot and max slots is 6 then: .RTTT.. */
#ifdef OSF_CONF_NTX
#define OSF_NTX                       OSF_CONF_NTX
#else
#define OSF_NTX                       3
#endif

/* Per-round NTX configuration */
#ifdef OSF_CONF_ROUND_S_NTX
#define OSF_ROUND_S_NTX               OSF_CONF_ROUND_S_NTX
#else
#define OSF_ROUND_S_NTX               OSF_NTX
#endif
#ifdef OSF_CONF_ROUND_T_NTX
#define OSF_ROUND_T_NTX               OSF_CONF_ROUND_T_NTX
#else
#define OSF_ROUND_T_NTX               OSF_NTX
#endif
#ifdef OSF_CONF_ROUND_A_NTX
#define OSF_ROUND_A_NTX               OSF_CONF_ROUND_A_NTX
#else
#define OSF_ROUND_A_NTX               OSF_NTX
#endif

/* Per-round MAX SLOTS configuration */
/* Max slots dictate the maximum possible number of slots in the round. E.g.,
   if NTX is 3 and you RX on the 4th slot and max slots is 6 then: ...RTT */
#ifdef OSF_CONF_ROUND_S_MAX_SLOTS
#define OSF_ROUND_S_MAX_SLOTS         OSF_CONF_ROUND_S_MAX_SLOTS
#else
#define OSF_ROUND_S_MAX_SLOTS         (OSF_ROUND_S_NTX * 2)
#endif
#ifdef OSF_CONF_ROUND_T_MAX_SLOTS
#define OSF_ROUND_T_MAX_SLOTS         OSF_CONF_ROUND_T_MAX_SLOTS
#else
#define OSF_ROUND_T_MAX_SLOTS         (OSF_ROUND_T_NTX * 2)
#endif
#ifdef OSF_CONF_ROUND_A_MAX_SLOTS
#define OSF_ROUND_A_MAX_SLOTS         OSF_CONF_ROUND_A_MAX_SLOTS
#else
#define OSF_ROUND_A_MAX_SLOTS         (OSF_ROUND_A_NTX * 2)
#endif

/* MAX MAX SLOTS*/
#define OSF_MAX_MAX_SLOTS             (MAX(OSF_ROUND_S_MAX_SLOTS, MAX(OSF_ROUND_T_MAX_SLOTS, OSF_ROUND_A_MAX_SLOTS)))

/* Turn frequncy hopping on/off */
#ifdef OSF_CONF_CH
#define OSF_CH                        OSF_CONF_CH
#else
#define OSF_CH                        1
#endif

/* Default TX power */
#ifdef OSF_CONF_TXPOWER
#define OSF_TXPOWER                   OSF_CONF_TXPOWER
#else
#define OSF_TXPOWER                   ZerodBm
#endif

/* Number of failed/missed epochs before desync and rejoin */
#ifdef OSF_CONF_RESYNC_THRESHOLD
#define OSF_RESYNC_THRESHOLD          OSF_CONF_RESYNC_THRESHOLD
#else
#define OSF_RESYNC_THRESHOLD          10  /* Quit and rejoin after N missed sync rounds */
#endif

/* Maximum number of rounds in a protocol schedule */
#define OSF_SCHEDULE_LEN_MAX          25

/* Scanning interval */
#define OSF_SCAN_TIME                 (US_TO_RTIMERTICKSX(20000))

/* Pullback before a round to account for drift (in one direction) */
#define OSF_RX_GUARD                  (US_TO_RTIMERTICKSX(50))

/* Pre epoch guard for stuff just before the epoch
   (e.g., for polling the testbed process). */
#if BUILD_WITH_TESTBED
#define OSF_PRE_EPOCH_GUARD           (US_TO_RTIMERTICKSX(20000))
#else
#define OSF_PRE_EPOCH_GUARD           (US_TO_RTIMERTICKSX(0))
#endif

/* Post epoch guard for stuff just after the epoch in osf_post_epoch_process
   (e.g., for logging) */
#if OSF_CONF_LOGGING
#define OSF_POST_EPOCH_GUARD          (US_TO_RTIMERTICKSX(10000))
#else
#define OSF_POST_EPOCH_GUARD          (US_TO_RTIMERTICKSX(0))
#endif

/* We need just enough processing guard to complete round setup before we hit
   t_epoch_ref. */
#define OSF_ROUND_GUARD               (US_TO_RTIMERTICKSX(500))

/* NB: IFS ticks between TX's. Min needed is...
    osf_phy_conf->tx_rx_addr_offset_ticks + <some-processing-time> + RRU
   If you wish to get this down even further, the code path will need
   to be reduced. Equally, if you want to do stuff in between TX slots,
   then this value needs to be increased. */
#define OSF_TIFS_TICKS                (US_TO_RTIMERTICKSX(2000))
/* NB: REF_SHIFT: The TS takes it's reference time from TXEN, while the
   receivers sync on EVENTS_ADDRESS/FRAMESTART. The receivers therefore need
   to pull their ref time back by a processing offset between EVENTS_ADDRESS
   on the TX and RX, the header airtime, and the radio rampup */
#define OSF_REF_SHIFT                 (my_radio_get_phy_conf(OSF_ROUND_S_PHY)->header_air_ticks \
                                        + my_radio_get_phy_conf(OSF_ROUND_S_PHY)->tx_rx_addr_offset_ticks \
                                        + RADIO_RAMPUP_TIME)

#define OSF_PERIOD                    (US_TO_RTIMERTICKSX((OSF_PERIOD_MS) * 1000))
#define ROUND_LEN_RULE                ((osf.slot < osf.rconf->max_slots) && (osf.n_tx < osf.rconf->ntx))

/*---------------------------------------------------------------------------*/
/* Bit manipulation */
#if BUILD_WITH_TESTBED
#define OSF_BITMASK_LEN               ((uint8_t)(OSF_MAX_NODES/8) + ((OSF_MAX_NODES%8) > 0))
#else
#define OSF_BITMASK_LEN               ((uint8_t)(OSF_MAX_NODES/8) + ((OSF_MAX_NODES%8) > 0))
#endif

#define OSF_SET_VALUE(var, val)       (var |= val)
#define OSF_CHK_VALUE(var, val)       !!(var & val)
#define OSF_SET_BIT(var, pos)         (var |= (1u << pos))
#define OSF_CHK_BIT(var, pos)         !!(var & (1u << pos))
#define OSF_CLR_BIT(var, pos)         (var &= ~(1u << pos))
#define OSF_TOGGLE_BIT(var, pos)      (var ^= (1u << pos))
#define OSF_SET_BIT_BYTE(var, pos)    (var[(pos/8)] |= (1u << (pos % 8)))
#define OSF_CHK_BIT_BYTE(var, pos)    !!(var[(pos/8)] & (1u << (pos % 8)))
#define OSF_CLR_BIT_BYTE(var, pos)    (var[(pos/8)] &= ~(1u << (pos % 8)))
#define OSF_TOGGLE_BIT_BYTE(var, pos) (var[(pos/8)] ^= (1u << (pos % 8)))
#define OSF_CLR_ALL_BITS(var, len)     memset(var, 0, len);

/*---------------------------------------------------------------------------*/
/* TESTING */
#ifdef OSF_CONF_TEST_MISS_RXS
#define OSF_TEST_MISS_RXS             OSF_CONF_TEST_MISS_RXS
#else
#define OSF_TEST_MISS_RXS             0
#endif

#ifdef OSF_CONF_TEST_NODE
#define OSF_TEST_NODE                 OSF_CONF_TEST_NODE
#else
#define OSF_TEST_NODE                 0
#endif

/*---------------------------------------------------------------------------*/
/* OSF state machine */
enum osf_state {
  OSF_STATE_OFF,
  OSF_STATE_ON,
  OSF_STATE_WAITING,
  OSF_STATE_READY,
  OSF_STATE_RECEIVING,
  OSF_STATE_RECEIVED,
  OSF_STATE_TRANSMITTING
};
extern uint8_t osf_state;

/*---------------------------------------------------------------------------*/
/* OSF slot state */
#define OSF_SLOT_0 '.'  /* None */
#define OSF_SLOT_T 'T'  /* Transmit */
#define OSF_SLOT_R 'R'  /* Receive */
#define OSF_SLOT_C 'C'  /* CRC */
#define OSF_SLOT_H 'H'  /* Hop */
#define OSF_SLOT_X 'X'  /* Skip rest of epoch */
#define OSF_SLOT_S 'S'  /* Skip rest of round */
#define OSF_SLOT_M 'M'  /* Miss slot */
#define OSF_SLOT_E 'E'  /* Stop RX / No EVENTS_END */
#define OSF_SLOT_O 'O'  /* Stop RX / No EVENTS_END */

/* OSF slot data struct */
// // TODO: different types of slots (e.g., TX/RX)
// typedef struct osf_slot {
//   uint8_t        type;
//   rtimer_clock_t dur;
// } osf_slot_t;

/*---------------------------------------------------------------------------*/
/* OSF round data struct */
typedef enum osf_round_type {
  OSF_ROUND_S,
  OSF_ROUND_T,
  OSF_ROUND_A
} osf_round_type_t;

#define OSF_ROUND_TO_STR(R) \
  ((R == OSF_ROUND_S) ? ("OSF_ROUND_S") : \
   (R == OSF_ROUND_T) ? ("OSF_ROUND_T") : \
   (R == OSF_ROUND_A) ? ("OSF_ROUND_A") : ("???"))
#define OSF_ROUND_TO_STR_SHORT(R) \
  ((R == OSF_ROUND_S) ? ("S") : \
   (R == OSF_ROUND_T) ? ("T") : \
   (R == OSF_ROUND_A) ? ("A") : ("???"))

typedef enum osf_round_role {
  OSF_ROLE_NONE,
  OSF_ROLE_SRC,
  OSF_ROLE_DST,
  OSF_ROLE_FWD
} osf_round_role_t;

#define OSF_ROLE_TO_STR(R) \
  ((R == OSF_ROLE_NONE) ? ("N") : \
   (R == OSF_ROLE_SRC)  ? ("S") : \
   (R == OSF_ROLE_DST)  ? ("D") : \
   (R == OSF_ROLE_FWD)  ? ("F") : ("???"))

/* OSF round data struct */
typedef struct osf_round {
  /* Round details */
  char*                 name;                           /* readable round name */
  osf_round_type_t      type;                           /* OSF round type */
  uint8_t               sync;                           /* is a sync round */
  osf_primitive_t       primitive;                      /* i.e., RX-TX (RoF) or RX-TX-TX (Glossy) */
  uint8_t               statlen;                        /* use static length (i.e., no length field) */
  uint8_t               is_initiator;                   /* is an initiator this round */
  /* Round API */
  void                 (*init)();
  void                 (*configure)();
  uint8_t              (*send)();
  uint8_t              (*receive)();
  void                 (*no_rx)();
} osf_round_t;

/* Round externs for use with protocols */
extern osf_round_t osf_round_s;
extern osf_round_t osf_round_tx;
extern osf_round_t osf_round_a;

/* OSF round configuration */
typedef struct osf_round_conf {
osf_round_t             *round;                          /* round logic */
osf_phy_conf_t          *phy;                            /* PHY configuration */
uint8_t                  ntx;                            /* number of TX in round */
uint8_t                  is_last;                        /* after this round exit the epoch */
uint8_t                  max_slots;                      /* max number of timeslots in round */
rtimer_clock_t           duration;                       /* total round duration */
rtimer_clock_t           t_offset;                       /* offset from the epoch ref*/
rtimer_clock_t           t_slots[OSF_NTX];               /* rxtx timeslot timings TODO: Not used */
uint8_t                  sources[OSF_BITMASK_LEN];       /* permitted sources in this slot */
} osf_round_conf_t;

/*---------------------------------------------------------------------------*/
/* OSF protocol */
#define OSF_PROTO_BCAST  (0x00)
#define OSF_PROTO_STA    (0x01)

#define OSF_GET_PROTO(P)                            \
(                                                   \
  (P == OSF_PROTO_BCAST) ? &osf_proto_bcast :       \
  ((P == OSF_PROTO_STA)  ? &osf_proto_sta   : NULL) \
)

#define OSF_PROTO_TO_STR(R) \
  ((R == OSF_PROTO_BCAST) ? ("OSF_PROTO_BCAST") : \
   (R == OSF_PROTO_STA)   ? ("OSF_PROTO_STA") : ("???"))

#ifdef OSF_CONF_PROTO
#define OSF_PROTOCOL                  OSF_CONF_PROTO
#else
#define OSF_PROTOCOL                  OSF_PROTO_STA
#endif

/* OSF protocol data struct */
typedef struct osf_proto {
  /* Protocol details */
  uint8_t               type;                           /* type */
  rtimer_clock_t        duration;                       /* duration */
  /* Protocol API */
  void                (*init)();                        /* protocol init */
  void                (*configure)();                   /* protocol configure */
  osf_round_conf_t*   (*next_round)();                  /* protocol next round */
  /* Protocol schedule (rounds) */
  uint8_t               index;                          /* protocol schedule index */
  uint8_t               len;                            /* protocol schedule length  */
  osf_round_conf_t      sched[OSF_SCHEDULE_LEN_MAX];    /* schedule of rounds */
  /* Protocol state */
  uint8_t               sent[OSF_SCHEDULE_LEN_MAX];     /* send to id */
  uint8_t               received[OSF_SCHEDULE_LEN_MAX]; /* received from id */
  osf_round_role_t      role;                           /* current role in protocol */
} osf_proto_t;

/* Protocol externs */
extern osf_proto_t osf_proto_bcast;
extern osf_proto_t osf_proto_sta;

/*---------------------------------------------------------------------------*/
/* Data structure to hold global OSF status and configuration */
typedef struct osf {
  /* Epoch */
  rtimer_clock_t        period;
  uint16_t              epoch;
  uint16_t              join_epoch;
  /* Protocol */
  osf_proto_t          *proto;
  /* Round */
  osf_round_conf_t     *rconf;
  osf_round_t          *round;
  uint8_t               n_tx;
  uint8_t               n_rx_ok;
  uint8_t               n_rx_crc;
  /* Slot */
  uint8_t               slot;
  uint8_t               last_rx_ok;
  uint8_t               last_sync_slot;
  char                  last_slot_type;
  /* Epoch */
  uint16_t              n_syncs;
  uint8_t               n_rnd_since_rx;
  uint8_t               failed_epochs;
  /* Timing */
  rtimer_clock_t        t_epoch_ref;
  int                   t_epoch_drift;
  int                   t_slot_drift;
  /* Network */
  uint8_t               sources[OSF_MAX_NODES];
  uint8_t               src_len;
  uint8_t               destinations[OSF_MAX_NODES];
  uint8_t               dst_len;
  uint8_t               border_routers[OSF_MAX_NODES];
  uint8_t               br_len;
} osf_t;

extern rtimer_clock_t t_ref;
extern osf_t osf;

/*---------------------------------------------------------------------------*/
/* General externs */
extern uint8_t osf_is_on;
extern uint8_t osf_timesync;

extern uint8_t node_is_timesync;
extern uint8_t node_is_synced;
extern uint8_t node_is_joined;
extern uint8_t node_is_source;
extern uint8_t node_is_destination;
extern uint8_t node_is_br;

extern uint8_t was_out_of_sync;
extern uint8_t exp_buf[OSF_CONF_DATA_LEN_MAX];

/*---------------------------------------------------------------------------*/
/* Callback for notifying other processes of received data */
typedef void (*osf_input_callback_t)(uint8_t *data, uint8_t len);

/*---------------------------------------------------------------------------*/
/* TODO: Make these part of netstack struct */
void    osf_configure(uint8_t *sources, uint8_t src_len,
                      uint8_t *destinations, uint8_t dst_len,
                      uint8_t *border_routers, uint8_t br_len);
void    osf_init(void);
void    osf_register_input_callback(osf_input_callback_t cb);
void    osf_sync(void);
void    osf_join(void);
uint8_t osf_send(uint8_t *data, uint8_t len, uint8_t dst);
uint8_t osf_receive(uint8_t src, uint8_t dst, uint8_t *data, uint8_t len);

rtimer_clock_t osf_get_reference_time();

#endif /* OSF_H_ */
