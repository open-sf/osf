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
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-net.h"
#include "sys/log.h"

#define LOG_MODULE "OSF-NET"
#define LOG_LEVEL LOG_LEVEL_INFO

QUEUE(osf_net_buf);
static uint8_t           osf_net_head;
static osf_node_t        osf_net_array[OSF_NET_MAX_JOINED_NODES];

/*---------------------------------------------------------------------------*/
void
osf_net_init(void)
{
  LOG_INFO("OSF network management initialized\n");
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_net_get_joined_nodes(uint8_t *nodes, uint8_t max_count)
{
  uint8_t count = (osf.joined_count < max_count) ? osf.joined_count : max_count;
  uint8_t i;
  
  for(i = 0; i < count; i++) {
    nodes[i] = osf.joined_nodes[i];
  }
  
  return count;
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_net_get_joined_count(void)
{
  return osf.joined_count;
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_net_is_node_joined(uint8_t node_id)
{
  return osf_is_node_joined(node_id);
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_net_get_node_index(uint8_t node_id)
{
  return osf_get_node_index(node_id);
}

/*---------------------------------------------------------------------------*/
uint8_t
osf_net_get_node_at_index(uint8_t index)
{
  if(index >= osf.joined_count) {
    return 0; /* Invalid index */
  }
  return osf.joined_nodes[index];
}

/*---------------------------------------------------------------------------*/
void
osf_net_print_status(void)
{
  LOG_INFO("=== OSF Network Status ===\n");
  LOG_INFO("Timesync node: %u\n", osf_timesync);
  LOG_INFO("My node ID: %u\n", node_id);
  LOG_INFO("Am I timesync: %s\n", node_is_timesync ? "YES" : "NO");
  LOG_INFO("Am I synced: %s\n", node_is_synced ? "YES" : "NO");
  LOG_INFO("Am I joined: %s\n", node_is_joined ? "YES" : "NO");
  
  if(node_is_joined) {
    LOG_INFO("My join index: %u\n", osf.my_join_index);
  }
  
  LOG_INFO("Total joined nodes: %u/%u\n", osf.joined_count, OSF_NET_MAX_JOINED_NODES);
  
  if(osf.joined_count > 0) {
    LOG_INFO("Joined nodes list:\n");
    uint8_t i;
    for(i = 0; i < osf.joined_count; i++) {
      LOG_INFO("  [%u] Node %u%s\n", i, osf.joined_nodes[i], 
               (osf.joined_nodes[i] == osf_timesync) ? " (TIMESYNC)" : 
               (osf.joined_nodes[i] == node_id) ? " (ME)" : "");
    }
  }
  LOG_INFO("========================\n");
}
