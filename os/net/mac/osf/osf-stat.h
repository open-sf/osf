/*
 * Copyright (c) 2022, technology Innovation Institute (TII).
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         OSF stats.
 * \author
 *         Michael Baddeley <michael@ssrc.tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 *
 */

#ifndef OSF_STAT_H_
#define OSF_STAT_H_

typedef struct osf_mac_stat {

  uint32_t osf_mac_t_total;          /* Total amount of T rounds, when join */
  uint32_t osf_mac_tx_total;         /* T rounds, used for TX, with retransmission */
  uint32_t osf_mac_tx_ret_total;     /* T rounds, used for TX, retransmission only */

  uint32_t osf_mac_ack_total;        /* ACK OK */
  uint32_t osf_mac_no_ack_total;     /* No ACK after retransmissions */

  uint32_t osf_mac_rx_total;         /* T rounds, used for RX */
  uint32_t osf_mac_rx_slots_total;   /* Slots of T rounds, used for RX */

  uint32_t osf_rx_dup_total;         /* Dublicated packets */
  uint32_t osf_rx_wrong_addr_total;  /* Wrong link address packets */
  uint32_t osf_rx_too_big_total;     /* Longer than packet lenght */

  uint32_t osf_rx_tx_crc_error_total;   /* CRC errors, T round */
  uint32_t osf_rx_ack_crc_error_total;  /* CRC errors, A round */
  uint32_t osf_rx_sync_crc_error_total; /* CRC errors, S round */
  /**/
  uint32_t osf_join_total;           /* Join events */
  uint32_t osf_ts_lost_total;        /* Total lost of SYNC frames */
  uint32_t osf_sync_epoch_err_total; /* Epoc from sync frame is not mach */
  uint32_t osf_rejoin_ts_total;      /* Timesync lost all connections */

  uint32_t osf_rt_miss_epoch_total;  /* TimerX, miss epoch */
  uint32_t osf_rt_miss_round_total;  /* TimerX, miss round */
  uint32_t osf_rt_miss_slot_total;   /* TimerX, miss slot */

  uint32_t osf_rt_miss_rx_total;     /* TimerX, miss rx abs */
  uint32_t osf_rt_miss_tx_total;     /* TimerX, miss tx_abs */

  uint32_t osf_rt_miss_timeout_total;/* TimerX, rx timeout miss */
  uint32_t osf_rt_miss_glossy_total; /* TimerX, rx glossy miss */


} osf_mac_stat_t;

extern osf_mac_stat_t osf_stat;

void osf_stat_init();

#endif /* OSF_STAT_H_ */
