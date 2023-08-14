/*
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

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdlib.h>

#include "sys/log.h"
#define LOG_MODULE "UDPS"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;

/*---------------------------------------------------------------------------*/
PROCESS(udp_server_process, "UDP server");

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
  uint16_t plen = datalen > 6 ? 6 : datalen;
  LOG_DBG("Received request '%.*s' from ", plen, (char *)data);
  LOG_DBG_6ADDR(sender_addr);
  LOG_DBG_(" : %d\n", sender_port);
#if WITH_SERVER_REPLY
  static uint8_t buf[UIP_BUFSIZE];
  if(datalen >= 2 && data[0] == 'E') {
    memset(buf, 0, 6);
    memcpy(buf, data, 6);
    buf[5] = 0;
    datalen = atoi((const char *)&buf[1]);
    memset(buf, 0x55, sizeof(buf));
    if(datalen > UIP_BUFSIZE) {
      datalen = UIP_BUFSIZE;
    }
    LOG_INFO("Sending %d bytes to source\r\n", datalen);
    simple_udp_sendto_port(&udp_conn, buf, datalen, sender_addr, sender_port);
  } else {
    /* Send back the same string to the client as an echo reply */
    LOG_INFO("Sending echo %d bytes\n", datalen);
    simple_udp_sendto_port(&udp_conn, data, datalen, sender_addr, sender_port);
  }
#endif /* WITH_SERVER_REPLY */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize DAG root */
  /*NETSTACK_ROUTING.root_start(); */

  /* Initialize UDP connection */
  (void)simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                            UIP_HTONS(0), udp_rx_callback);

  LOG_INFO("Register UDP server, port %d\r\n", UDP_SERVER_PORT);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
