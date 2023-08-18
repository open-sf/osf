/*
 * Copyright (c) 2018, RISE SICS.
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
 *         Node-id management
 * \author
 *         Simon Duquennoy <simon.duquennoy@ri.se>
 */

#include "contiki.h"
#include "sys/node-id.h"
#include "net/linkaddr.h"

#if BUILD_WITH_TESTBED
#include "services/testbed/testbed.h"
#endif
#if BUILD_WITH_DEPLOYMENT
#include "services/deployment/deployment.h"
#endif

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "NODEID"
#define LOG_LEVEL LOG_LEVEL_DBG

uint16_t node_id = 0;

void
node_id_init(void) {

#if BUILD_WITH_TESTBED && TB_PATCHING && BUILD_WITH_DEPLOYMENT
  /* Configure through patching. Failing that, configure through deployment */
  deployment_init();
  LOG_DBG("Init ID through patching.\n");
  node_id = dc_cfg.node_id;
  if(!node_id) {
    LOG_WARN("ID is 0. Init through deployment.\n");
    deployment_init();
  }

#elif BUILD_WITH_TESTBED && TB_PATCHING
  /* Configure directly through testbed patching */
  LOG_DBG("Init ID through patching.\n");
  node_id = dc_cfg.node_id;

#elif BUILD_WITH_DEPLOYMENT
  /* Configure through deployment struct */
  LOG_DBG("Init ID through deployment: ");
  deployment_init();
  LOG_DBG_("%u\n", node_id);
  if(!node_id) {
    LOG_WARN("Could not find node id!\n");
  } 

#else

  /* Initialize with a default value derived from linkaddr */
  LOG_DBG("Init ID through linkaddr.\n");
  node_id = linkaddr_node_addr.u8[LINKADDR_SIZE - 1]
            + (linkaddr_node_addr.u8[LINKADDR_SIZE - 2] << 8);

#endif /* BUILD_WITH_DEPLOYMENT */

}
