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

#include "contiki.h"
#include "net/mac/mac.h"
#include "lib/list.h"
#include "lib/queue.h"
#include "net/packetbuf.h"

#include "services/deployment/deployment.h"

#include "osf.h"
#include "osf-packet.h"
#include "osf-buffer.h"
#include "osf-debug.h"
#include "osf-log.h"
#include "osf-stat.h"

#include "sys/log.h"
#define LOG_MODULE "OSF-BUF"
#define LOG_LEVEL LOG_LEVEL_NONE

QUEUE(osf_tx_buf);
QUEUE(osf_rx_buf);
static uint8_t           osf_buf_head;
static uint8_t           osf_buf_element_tail;
static osf_buf_element_t osf_buf_array[OSF_BUF_MAX_SIZE];

/* Superfluous data check */
static uint16_t          current_data_id = 0;
static uint16_t          last_seq_no_rx[256]  = {0}; // Node IDs go up to 255
static uint8_t           last_was_superfluous[255] = {0};
static uint8_t           last_was_acked[255] = {0};

QUEUE(osf_log_queue);
static uint8_t           log_index;
static osf_log_t         log_array[OSF_LOG_MAX];

/*---------------------------------------------------------------------------*/
/* Logging */
/*---------------------------------------------------------------------------*/
void
osf_buf_log_init()
{
  log_index = 0;
  queue_init(osf_log_queue);
}

/*---------------------------------------------------------------------------*/
static void
osf_buf_log_data(osf_log_type_t type, uint16_t id, uint8_t src, uint8_t dst,
                 uint8_t *data, uint8_t len, uint8_t rtx, uint8_t slot)
{
  osf_log_t *l = &log_array[log_index];
  if(l != NULL) {
    l->round = osf.epoch;
    l->id = id;
    l->src = src;
    l->dst = dst;
    l->type = type;
    l->len = len;
    l->rtx = rtx;
    l->slot = slot;
    if (len < OSF_LOG_MAX_PRINT_LEN) {
      memcpy(l->data, data, len);
    } else {
      memcpy(l->data, data, OSF_LOG_MAX_PRINT_LEN);
    }
    queue_enqueue(osf_log_queue, l);
  }
  log_index = (log_index + 1) % OSF_LOG_MAX;
}

/*---------------------------------------------------------------------------*/
void
osf_buf_log_print()
{
  uint8_t i;
  osf_log_t *l;
  while ((l = queue_dequeue(osf_log_queue)) != NULL) {
    LOG_INFO("BUF: %-3s | ", OSF_LOG_TYPE_TO_STR(l->type));
    uint8_t print_len = (l->len > OSF_LOG_MAX_PRINT_LEN) ? OSF_LOG_MAX_PRINT_LEN : l->len;
    for(i = 0; i < print_len; i++) {
      LOG_INFO_("%02x ", l->data[i]);
    };
    if (l->len > OSF_LOG_MAX_PRINT_LEN) {
      LOG_INFO_("... %02x ",  l->data[l->len - 1]);
    }
    LOG_INFO_(" l:%u id:%-3u rtx:%u s:%-3u d:%-3u rc:%-2u ep:%u\n", l->len, l->id, l->rtx, l->src, l->dst, l->slot, l->round);
  }
}

/*---------------------------------------------------------------------------*/
/* Initialisation */
/*---------------------------------------------------------------------------*/
void
osf_buf_init() {
  LOG_INFO("OSF-BUF init... \n");
  osf_buf_head = 0;
  osf_buf_element_tail = 0;
  memset(osf_buf_array, 0, sizeof(osf_buf_element_t) * OSF_BUF_MAX_SIZE);
  queue_init(osf_tx_buf);
  queue_init(osf_rx_buf);
  LOG_INFO("- MAX QUEUE SIZE: %u\n", OSF_BUF_MAX_SIZE);
  osf_buf_log_init();
}

/*---------------------------------------------------------------------------*/
/* Helper functions */
/*---------------------------------------------------------------------------*/
static inline osf_buf_element_t *
new_element(list_t list)
{
  osf_buf_element_t *el = NULL;
  if(list_length(list) >= OSF_BUF_MAX_SIZE) {
      el = queue_dequeue(list);
      if(el != NULL) {
        osf_buf_head = (osf_buf_head + 1) & (OSF_BUF_MAX_SIZE - 1);
        LOG_DBG("RM FULL %p (%u)[%u-%u]/%u\n", el, list_length(list),
            osf_buf_element_tail, osf_buf_head, OSF_BUF_MAX_SIZE);
      }
  }
  el = &osf_buf_array[osf_buf_element_tail];
  memset(el, 0, sizeof(osf_buf_element_t));
#if OSF_BUF_LIFO
  list_push(list, el);
#else
  queue_enqueue(list, el);
#endif
  osf_buf_element_tail = (osf_buf_element_tail + 1) & (OSF_BUF_MAX_SIZE - 1);
  LOG_DBG("ADD %p (%u)[%u-%u]/%u\n", el, list_length(list),
      osf_buf_element_tail, osf_buf_head, OSF_BUF_MAX_SIZE);
  return el;
}

/*---------------------------------------------------------------------------*/
static inline osf_buf_element_t *
remove_element(list_t list)
{
  osf_buf_element_t *el = NULL;
  el = queue_dequeue(list);
  if(el != NULL) {
    osf_buf_head = (osf_buf_head + 1) & (OSF_BUF_MAX_SIZE - 1);
    LOG_DBG("RM %p (%u)[%u-%u]/%u\n", el, list_length(list),
        osf_buf_element_tail, osf_buf_head, OSF_BUF_MAX_SIZE);
  }
  return el;
}

/*---------------------------------------------------------------------------*/
/* TX */
/*---------------------------------------------------------------------------*/
uint8_t
osf_buf_tx_put(uint8_t *data, uint8_t len, uint8_t dst)
{
  osf_buf_element_t *el = new_element(osf_tx_buf);

  if ((el != NULL))  {
    if(len <= OSF_DATA_LEN_MAX) {
      /* Packet specific stuff */
      if(osf.epoch % 3 == 0 || current_data_id == 0) {
        el->id = ++current_data_id;
      } 
      else {
        el->id = current_data_id;
      }
      el->src = node_id;
      el->dst = dst;
      /* Needed for this buf */
      el->len = len;
      memcpy(el->data, data, len);
      el->rtx = 0;
      return 1;
    } else {
      LOG_ERR("FATAL!!! TX len %u > MAX payload %u\n", len, OSF_DATA_LEN_MAX);
    }
  } else {
    LOG_WARN("q is full %u/%u\n", list_length(osf_tx_buf), OSF_BUF_MAX_SIZE);
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_buf_tx_packetbuf_put(uint8_t len, uint8_t dst, void *callback, void *ptr)
{
  osf_buf_element_t *el = new_element(osf_tx_buf);

  if ((el != NULL))  {
    if(len <= OSF_DATA_LEN_MAX) {
      /* Packet specific stuff */
      el->id = ++current_data_id;
      el->src = node_id;
      el->dst = dst;
      /* Needed for this buf */
      el->len = len;
      el->callback = callback;
      el->ptr = ptr;
      packetbuf_copyto(el->data);
      el->rtx = 0;
      return 1;
    } else {
      LOG_ERR("FATAL!!! TX len %u > MAX payload %u\n", len, OSF_DATA_LEN_MAX);
    }
  } else {
    LOG_WARN("q is full %u/%u\n", list_length(osf_tx_buf), OSF_BUF_MAX_SIZE);
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
osf_buf_element_t *
osf_buf_tx_get()
{
  osf_buf_element_t *el = list_head(osf_tx_buf);
  if (el != NULL)  {
    if(OSF_BUF_RETRANSMISSIONS != 0 && el->rtx > OSF_BUF_RETRANSMISSIONS) {
      osf_buf_tx_remove_head();
      osf_stat.osf_mac_no_ack_total++; // Statistics
    }
    if(!el->rtx) {
      osf_buf_log_data(TX, el->id, node_id, el->dst, el->data, el->len, el->rtx, 0);
    } else {
      osf_buf_log_data(RTR, el->id, node_id, el->dst, el->data, el->len, el->rtx, 0);
      osf_stat.osf_mac_tx_ret_total++; // Statistics
    }
    el->rtx++;
  }
  return el;
}

/*---------------------------------------------------------------------------*/
osf_buf_element_t *
osf_buf_tx_peek()
{
  osf_buf_element_t *el = list_head(osf_tx_buf);
  return el;
}


/*---------------------------------------------------------------------------*/
uint8_t
osf_buf_tx_remove_head()
{
  osf_buf_element_t *el = remove_element(osf_tx_buf);
  if (el != NULL)  {
    return 1;
  } else {
    return 0;
  }
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_buf_tx_length()
{
  return list_length(osf_tx_buf);
}

/*---------------------------------------------------------------------------*/
/* RX */
/*---------------------------------------------------------------------------*/
uint8_t
osf_buf_rx_put(uint8_t id, uint8_t src, uint8_t dst, uint8_t *data, uint8_t len)
{
  osf_buf_element_t *el = new_element(osf_rx_buf);
  if ((el != NULL))  {
    if(len <= OSF_DATA_LEN_MAX) {
      el->id = id;
      el->src = src;
      el->dst = dst;
      el->len = len;
      memcpy(el->data, data, len);
      el->rtx = 0;
      el->round = osf.epoch;
      return 1;
    } else {
      LOG_ERR("RX len %u > MAX payload %u\n", len, OSF_DATA_LEN_MAX);
    }
  } else {
    LOG_WARN("RX q is full %u/%u\n", list_length(osf_rx_buf), OSF_BUF_MAX_SIZE);
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
osf_buf_element_t *
osf_buf_rx_get()
{
  osf_buf_element_t *el = remove_element(osf_rx_buf);
  return el;
}

/*---------------------------------------------------------------------------*/
osf_buf_element_t *
osf_buf_rx_peek()
{
  osf_buf_element_t *el = list_head(osf_rx_buf);
  return el;
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_buf_rx_remove_head()
{
  osf_buf_element_t *el = remove_element(osf_rx_buf);
  if (el != NULL)  {
    return 1;
  } else {
    return 0;
  }
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_buf_rx_length()
{
  return list_length(osf_rx_buf);
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_buf_receive(uint16_t id, uint8_t src, uint8_t dst, uint8_t *data, uint8_t len, uint8_t slot)
{
  last_was_superfluous[src] = 1;
  /* Superflous check using packet id */
  if(OSF_BUF_PKT_ID_LT(last_seq_no_rx[src], id)) {
    last_seq_no_rx[src] = id;
    /* No need, just add to RX buffer */
    //osf_receive(src, dst, data, len);;
    last_was_superfluous[src] = 0;
    last_was_acked[src] = 0;
  }
  if(last_was_superfluous[src]) {
    osf_buf_log_data(DUP, id, src, dst, data, len, 0, slot);
    LOG_DBG("Drop TX duplicate %u!\n", id);
    osf_stat.osf_rx_dup_total++; // Statictics
  } else {
    osf_buf_rx_put(id, src, dst, data, len);
    osf_buf_log_data(RX, id, src, dst, data, len, 0, slot);
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_buf_superfluous(uint16_t src)
{
  return last_was_superfluous[src];
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_buf_ack(uint16_t src)
{
  if(!last_was_acked[src]) {
    last_was_acked[src] = 1;
    return 1;
  } else {
    return 0;
  }
}
