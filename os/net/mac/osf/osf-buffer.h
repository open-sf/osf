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
 *         OSF MAC buffer.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

#ifndef OSF_BUF_H_
#define OSF_BUF_H_

/* D-Cube tests need a LIFO */
#define OSF_BUF_LIFO                      1

/*---------------------------------------------------------------------------*/
/* Bit manipulation */
/*---------------------------------------------------------------------------*/
#define BITHACK_BITMASK_BYTE_LEN(max)     ((uint8_t)(max/8) + ((max%8) > 0))
#define BITHACK_SET_VALUE(var, val)       (var |= val)
#define BITHACK_CHK_VALUE(var, val)       !!(var & val)
#define BITHACK_SET_BIT(var, pos)         (var |= (1u << pos))
#define BITHACK_CHK_BIT(var, pos)         !!(var & (1u << pos))
#define BITHACK_CLR_BIT(var, pos)         (var &= ~(1u << pos))
#define BITHACK_TOGGLE_BIT(var, pos)      (var ^= (1u << pos))
#define BITHACK_SET_BIT_BYTE(var, pos)    (var[(pos/8)] |= (1u << (pos % 8)))
#define BITHACK_CHK_BIT_BYTE(var, pos)    !!(var[(pos/8)] & (1u << (pos % 8)))
#define BITHACK_CLR_BIT_BYTE(var, pos)    (var[(pos/8)] &= ~(1u << (pos % 8)))
#define BITHACK_TOGGLE_BIT_BYTE(var, pos) (var[(pos/8)] ^= (1u << (pos % 8)))
#define BITHACK_CLR_ALL_BITS(var, len)     memset(var, 0, len);

/*---------------------------------------------------------------------------*/
/* Logging */
/*---------------------------------------------------------------------------*/
#define OSF_LOG_MAX           16
#define OSF_LOG_MAX_PRINT_LEN 18

typedef enum {
  TX,
  RX,
  ACK,
  FWD,
  RTR,
  RD,
  WR,
  DUP
} osf_log_type_t;

#define OSF_LOG_TYPE_TO_STR(L) \
 ((L == TX)      ? ("TX")   : \
  (L == RX)      ? ("RX")   : \
  (L == ACK)     ? ("ACK")  : \
  (L == FWD)     ? ("FWD")  : \
  (L == RD)      ? ("RD")   : \
  (L == WR)      ? ("WR")   : \
  (L == RTR)     ? ("RTR")  : \
  (L == DUP)     ? ("DUP")  : ("UNKWN"))

typedef struct osf_log {
  struct atm_log     *next;
  osf_log_type_t      type;
  uint8_t             rtx;
  uint16_t            id;
  uint8_t             src;
  uint8_t             dst;
  uint16_t            round;
  uint8_t             slot;
  uint8_t             turn;
  uint8_t             len;
  uint8_t             data[OSF_DATA_LEN_MAX];
} osf_log_t;

void osf_buf_log_print(void);

/*---------------------------------------------------------------------------*/
/* Data Structures */
/*---------------------------------------------------------------------------*/
#define OSF_BUF_PKT_ID_LT(a, b) ((signed short)((a)-(b)) < 0)

typedef struct osf_buf_element {
  struct atm_data    *next;
  uint16_t            rtx;
  uint16_t            id;
  uint8_t             src;
  uint8_t             dst;
  uint16_t            round;
  uint8_t             slot;
  uint8_t             turn;
  uint8_t             len;
  void                *callback;
  void                *ptr;
  uint8_t             data[OSF_DATA_LEN_MAX];
} osf_buf_element_t;

/*---------------------------------------------------------------------------*/
/* Queue */
/*---------------------------------------------------------------------------*/
/* Size of buffer */
#ifdef OSF_CONF_BUF_MAX_SIZE
#define OSF_BUF_MAX_SIZE                 OSF_CONF_BUF_MAX_SIZE
#else
#define OSF_BUF_MAX_SIZE                 16 // NB: must be a power of two!
#endif

/* Number of retransmisions. A value of 0 will keep the packet
   until intentionally dropped */
#ifdef OSF_CONF_BUF_RETRANSMISSIONS
#define OSF_BUF_RETRANSMISSIONS          OSF_CONF_BUF_RETRANSMISSIONS
#else
#define OSF_BUF_RETRANSMISSIONS          0
#endif /* OSF_RESEND_THRESHOLD */


/*---------------------------------------------------------------------------*/
/* API */
/*---------------------------------------------------------------------------*/
void               osf_buf_init();
uint8_t            osf_buf_tx_put(uint8_t *data, uint8_t len, uint8_t dst);
uint8_t            osf_buf_tx_packetbuf_put(uint8_t len, uint8_t dst, void *callback, void *ptr);
osf_buf_element_t *osf_buf_tx_get();
osf_buf_element_t *osf_buf_tx_peek();
uint8_t            osf_buf_tx_remove_head();
uint8_t            osf_buf_tx_length();
uint8_t            osf_buf_rx_put(uint8_t id, uint8_t src, uint8_t dst, uint8_t *data, uint8_t len);
osf_buf_element_t *osf_buf_rx_get();
osf_buf_element_t *osf_buf_rx_peek();
uint8_t            osf_buf_rx_remove_head();
uint8_t            osf_buf_rx_length();
uint8_t            osf_buf_receive(uint16_t id, uint8_t src, uint8_t dst, uint8_t *data, uint8_t len, uint8_t slot);
uint8_t            osf_buf_superfluous(uint16_t src);
uint8_t            osf_buf_ack(uint16_t src);

void               osf_buf_log_init();
void               osf_buf_log_print();

#endif /* OSF_BUF_H_ */
