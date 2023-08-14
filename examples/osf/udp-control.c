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
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uiplib.h"
#include "services/deployment/deployment.h"
#include "net/mac/osf/osf-stat.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "sys/log.h"
#define LOG_MODULE "UDPC"
#define LOG_LEVEL LOG_LEVEL_INFO

#if UDP_CONTROL_PORT
#define UDP_SERVER_PORT  UDP_CONTROL_PORT
#else
#define UDP_SERVER_PORT 7777
#endif

/* Node_id for acceses to Global Internet */
extern uint8_t isn;
extern int8_t radio_rssi;
static struct simple_udp_connection udp_conn;
static uint8_t buf[UIP_BUFSIZE];
static char string[150];

/*---------------------------------------------------------------------------*/
PROCESS(udp_control_process, "UDP control");
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
  uint16_t replylen = 0;
  LOG_DBG("Received request '%.*s' %d bytes from ", plen, (char *)data, plen);
  LOG_DBG_6ADDR(sender_addr);
  LOG_DBG_(" : %d\n", sender_port);
  memset(buf, 0, sizeof(buf));

  /* Mesh topology reguest */
  if(plen >= 3 && !memcmp((const char*)data, "?NN", 3)) {
    buf[0] = '!';
    buf[1] = 'N';
    buf[2] = 'N';
    replylen = 3;

    /* Total amount of nodes */
    if (plen == 3) {
      snprintf(string, sizeof(string), "%d", deployment_node_count());
      memcpy(&buf[3], string, strlen(string));
      replylen += strlen(string);
    } else {
      char data1[8];
      memset(data1, 0x00, sizeof(data1));
      memcpy(data1, &data[3], datalen - 3);
      uint16_t node = atoi((const char *)data1);
     
      /* ISN node */
      if(node == 255) {
        snprintf(string, sizeof(string), "%d", isn);
        memcpy(&buf[3], string, strlen(string));
        replylen += strlen(string);

      /* Current node */  
      } else  if(node == 0) {
        snprintf(string, sizeof(string), "%d", node_id);
        memcpy(&buf[3], string, strlen(string));
        replylen += strlen(string);

        /* Node IPV6 address */
      } else if(node <= deployment_node_count()){
        uip_ipaddr_t ipaddr;
        memset(&ipaddr, 0x00, sizeof(uip_ipaddr_t));
        ipaddr.u8[0] = UIP_DS6_DEFAULT_PREFIX_0;
        ipaddr.u8[1] = UIP_DS6_DEFAULT_PREFIX_1;
        deployment_iid_from_id(&ipaddr, node);
        char buf1[UIPLIB_IPV6_MAX_STR_LEN];
        uiplib_ipaddr_snprint(buf1, sizeof(buf1), &ipaddr);
        memcpy(&buf[3], buf1, strlen(buf1));
        replylen += strlen(buf1); 
      }
    }

    LOG_DBG("Sending %d bytes to source\r\n", replylen);
    simple_udp_sendto_port(&udp_conn, buf, replylen, sender_addr, sender_port);

    /* RSSI request */
  } else if(plen == 3 && !memcmp((const char*)data, "?RS", 3)) {
      buf[0] = '!';
      buf[1] = 'R';
      buf[2] = 'S';
      replylen = 3;
      snprintf(string, sizeof(string), "%d dBm", radio_rssi);
      memcpy(&buf[3], string, strlen(string));
      replylen += strlen(string);

      LOG_DBG("Sending %d bytes to source\r\n", replylen);
      simple_udp_sendto_port(&udp_conn, buf, replylen, sender_addr, sender_port);
     
     /* Transfer Statistics request */
    } else if(plen == 3 && !memcmp((const char*)data, "?S", 2)) {

      buf[0] = '!';
      buf[1] = 'S';
      buf[2] = data[2];
      replylen = 3;

      /* Reset counters */
      if(data[2] == 'Z') {
        osf_stat_init();

      /* S round counters */
      } else if(data[2] == 'S') {

      snprintf(string, sizeof(string), "total %lu, crc_erros %lu", \
        osf_stat.osf_rx_sync_total, osf_stat.osf_rx_sync_crc_error_total);
      memcpy(&buf[3], string, strlen(string));
      replylen += strlen(string);

      /* T round counters */
      } else if(data[2] == 'T') {

      snprintf(string, sizeof(string), "total %lu, crc_erros %lu", \
        osf_stat.osf_rx_tx_total, osf_stat.osf_rx_tx_crc_error_total);
      memcpy(&buf[3], string, strlen(string));
      replylen += strlen(string);
        
      /* A round counters */  
      } else if(data[2] == 'A') {
        
        snprintf(string, sizeof(string), "total %lu, crc_erros %lu", \
          osf_stat.osf_rx_ack_total, osf_stat.osf_rx_ack_crc_error_total);
        memcpy(&buf[3], string, strlen(string));
        replylen += strlen(string);

      }

      LOG_DBG("Sending %d bytes to source\r\n", replylen);
      simple_udp_sendto_port(&udp_conn, buf, replylen, sender_addr, sender_port);

    } else {

    /* Send back the same string to the client as an echo reply */
    LOG_DBG("Sending echo %d bytes\n", datalen);
    simple_udp_sendto_port(&udp_conn, data, datalen, sender_addr, sender_port);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_control_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize UDP connection */
  (void)simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                            UIP_HTONS(0), udp_rx_callback);

  LOG_INFO("Register UDP control server, port %d\r\n", UDP_SERVER_PORT);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
