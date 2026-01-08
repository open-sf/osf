/*
 * Copyright (c) 2025, Technology Innovation Institute
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
 *         OSF network management implementation.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 */

#include "contiki.h"
#include "contiki-net.h"
#include "net/ipv6/uip.h"
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-net.h"
#include "net/mac/osf/osf-debug.h"
#include "net/mac/osf/osf-stat.h"
#include "net/nbr-table.h"
#include "sys/log.h"
#include "sys/node-id.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define LOG_MODULE "OSF-NET"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Neighbor table for OSF network nodes */
NBR_TABLE(osf_net_node_t, osf_net_nodes);

static uint8_t osf_net_count = 0;

/*---------------------------------------------------------------------------*/
/* Constants */
/*---------------------------------------------------------------------------*/
#define OSF_NET_INVALID_INDEX         0xFF
#define OSF_NET_MAX_JOINED_NODES      64
#define OSF_NET_JOIN_FREQUENCY        10

/*---------------------------------------------------------------------------*/
/* Helper Functions */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Network Management */
/*---------------------------------------------------------------------------*/
void osf_net_init() {
  LOG_INFO("OSF-NET init. MAX JOINED NODES: %u...\n", OSF_NET_MAX_JOINED_NODES);
  
  /* Register the neighbor table */
  nbr_table_register(osf_net_nodes, NULL);
  
#if BUILD_WITH_TESTBED
  volatile tb_pattern_t *pattern = tb_get_pattern();
  switch (pattern->traffic_pattern) {
    case P2P:
    case P2MP:
      osf_timesync = tb_get_sources()[0];
      break;
    case MP2P:
      osf_timesync = tb_get_destinations()[0];
      break;
    case MP2MP:
    default:
      LOG_ERR("Unhandled PATTERN type! %s (%u)\n",
              PATTERN_TO_STR(pattern->traffic_pattern),
              pattern->traffic_pattern);
      return;
  }
  LOG_INFO("- OSF Timesync (%s) set to node %u using TESTBED",
           PATTERN_TO_STR(pattern->traffic_pattern), osf_timesync);
#elif BUILD_WITH_DEPLOYMENT
  osf_timesync = OSF_TS;
  LOG_INFO("- OSF Timesync set to node %u using DEPLOYMENT", osf_timesync);
#else
  /* TODO: Setting the timesync manually, but there should be a better way to do this. */
  if(node_id == OSF_TS) {
    osf_timesync = node_id;
  }
#endif
  /* Check if we are the timesync. If so, we should join the network. */
  if (node_id == osf_timesync) {
    node_is_timesync = 1;
    node_is_synced = 1;
    node_is_joined = 1;
    DEBUG_LEDS_ON(TS_LED);
    LOG_INFO_("... I (%x) am TS! (TS is %x)\n", node_id, osf_timesync);
    osf_net_add_node(node_id, &linkaddr_node_addr, OSF_DEFAULT_NETWORK_ID); // add myself to the network
    osf_net_join(node_id, OSF_DEFAULT_NETWORK_ID); // officially join the network
  } else {
    LOG_INFO_("... I (%x) am NOT TS! (TS is %x)\n", node_id, osf_timesync);
  }
  return;
}

/*---------------------------------------------------------------------------*/
uint8_t osf_net_add_node(const linkaddr_t *lladdr) {
  osf_net_node_t *n = NULL;
  /* Check to see if this node already exists in the nodes table */
  n = (osf_net_node_t *)nbr_table_get_from_lladdr(osf_net_nodes, lladdr);
  if(n == NULL) {
    /* Allocate a neighbor */
    n = (osf_net_node_t *)nbr_table_add_lladdr(osf_net_nodes, lladdr, NBR_TABLE_REASON_MAC, NULL);
    n->id = ++osf_net_count;
    linkaddr_copy(&n->lladdr, lladdr);
    LOG_INFO("ADDED node %x | %x\n", n->id, osf_net_count_nodes());
    return n->id;
  } else {
    LOG_ERR("Node %x already exists in the nbr-table\n", lladdr.u8[LINKADDR_SIZE-1]);
    return 0;
  }
}

/*---------------------------------------------------------------------------*/
void osf_net_join(uint8_t ts_id, uint8_t net_id) {
  if(!node_is_joined) {
    osf_timesync = ts_id;
    node_is_joined = 1;
    osf.join_epoch = osf.epoch;
    osf_stat.osf_join_total++; /* Statistics */
    DEBUG_LEDS_ON(JOINED_LED);
    LOG_INFO("Joined NET_ID %u with TS %x\n", net_id, osf_timesync);
  }
}

/*---------------------------------------------------------------------------*/
osf_net_node_t *osf_net_find_node(uint8_t id) { 
  osf_net_node_t *node;

  for(node = nbr_table_head(osf_net_nodes); node != NULL; 
      node = nbr_table_next(osf_net_nodes, node)) {
    if(node->id == id) {
      return node;
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
uint8_t osf_net_count_nodes(void) {
  uint8_t count = 0;
  osf_net_node_t *node;

  for (node = nbr_table_head(osf_net_nodes); node != NULL; 
       node = nbr_table_next(osf_net_nodes, node)) {
    if (node->joined == 1) {
      count++;
    }
  }
  return count;
}

/*---------------------------------------------------------------------------*/
uint8_t osf_net_count_join_requests(void) {
  uint8_t count = 0;
  osf_net_node_t *node;

  for (node = nbr_table_head(osf_net_nodes); node != NULL; 
       node = nbr_table_next(osf_net_nodes, node)) {
    if (node->joined == 0) {
      count++;
    }
  }
  return count;
}

/*---------------------------------------------------------------------------*/
osf_net_node_t *osf_net_get_next_join_request(void) {
  osf_net_node_t *node;

  for (node = nbr_table_head(osf_net_nodes); node != NULL; 
       node = nbr_table_next(osf_net_nodes, node)) {
    if (node->joined == 0) {
      return node;
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
uint16_t osf_net_get_next_join_epoch(void) {
  return osf.epoch + OSF_NET_JOIN_FREQUENCY;
}
