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

#ifndef OSF_NET_H_
#define OSF_NET_H_

#include "contiki.h"

/*---------------------------------------------------------------------------*/
/* Network management constants */
#define OSF_NET_MAX_JOINED_NODES      64
#define OSF_NET_INVALID_INDEX         0xFF

/*---------------------------------------------------------------------------*/
/* Data Structures */

typedef struct osf_node {
  struct osf_node    *next;
  uint16_t            id;
} osf_node_t;

/*---------------------------------------------------------------------------*/
/* Network management functions */

void osf_net_init(void);

uint8_t osf_net_get_joined_nodes(uint8_t *nodes, uint8_t max_count);
uint8_t osf_net_get_joined_count(void);
uint8_t osf_net_is_node_joined(uint8_t node_id);
uint8_t osf_net_get_node_index(uint8_t node_id);
uint8_t osf_net_get_node_at_index(uint8_t index);
void osf_net_print_status(void);

#endif /* OSF_NET_H_ */
