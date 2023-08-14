/*
 * Copyright (c) 2017, RISE SICS
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

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/ipv6/uip-sr.h"
#include "net/ipv6/uip.h"
#if (WEB_SERVER_STAT != 0)
#include "net/mac/osf/osf-stat.h"
#endif

#include "sys/log.h"
#define LOG_MODULE "WEBS"
#define LOG_LEVEL LOG_LEVEL_INFO

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
static const char *TOP = "<html>\n  <head>\n    <title>Contiki-NG</title>\n  </head>\n<body>\n";
static const char *BOTTOM = "\n</body>\n</html>\n";
static char buf[256];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)
#define SEND(s) do { \
  SEND_STRING(s, buf); \
  blen = 0; \
} while(0);

/* Use simple webserver with only one page for minimum footprint.
 * Multiple connections can result in interleaved tcp segments since
 * a single static buffer is used for all segments.
 */
#include "httpd-simple.h"

/*---------------------------------------------------------------------------*/
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
  uint16_t a;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        ADD("::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        ADD(":");
      }
      ADD("%x", a);
    }
  }
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  static uip_ds6_nbr_t *nbr;

  PSOCK_BEGIN(&s->sout);
  SEND_STRING(&s->sout, TOP);

  ADD("  Neighbors\n  <ul>\n");
  SEND(&s->sout);
  for(nbr = uip_ds6_nbr_head();
      nbr != NULL;
      nbr = uip_ds6_nbr_next(nbr)) {
    ADD("    <li>");
    ipaddr_add(&nbr->ipaddr);
    ADD("</li>\n");
    SEND(&s->sout);
  }
  ADD("  </ul>\n");
  SEND(&s->sout);

#if (UIP_MAX_ROUTES != 0)
  {
    static uip_ds6_route_t *r;
    ADD("  Routes\n  <ul>\n");
    SEND(&s->sout);
    for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {
      ADD("    <li>");
      ipaddr_add(&r->ipaddr);
      ADD("/%u (via ", r->length);
      ipaddr_add(uip_ds6_route_nexthop(r));
      ADD(") %lus", (unsigned long)r->state.lifetime);
      ADD("</li>\n");
      SEND(&s->sout);
    }
    ADD("  </ul>\n");
    SEND(&s->sout);
  }
#endif /* UIP_MAX_ROUTES != 0 */

#if (UIP_SR_LINK_NUM != 0)
  if(uip_sr_num_nodes() > 0) {
    static uip_sr_node_t *link;
    ADD("  Routing links\n  <ul>\n");
    SEND(&s->sout);
    for(link = uip_sr_node_head(); link != NULL; link = uip_sr_node_next(link)) {
      if(link->parent != NULL) {
        uip_ipaddr_t child_ipaddr;
        uip_ipaddr_t parent_ipaddr;

        NETSTACK_ROUTING.get_sr_node_ipaddr(&child_ipaddr, link);
        NETSTACK_ROUTING.get_sr_node_ipaddr(&parent_ipaddr, link->parent);

        ADD("    <li>");
        ipaddr_add(&child_ipaddr);

        ADD(" (parent: ");
        ipaddr_add(&parent_ipaddr);
        ADD(") %us", (unsigned int)link->lifetime);

        ADD("</li>\n");
        SEND(&s->sout);
      }
    }
    ADD("  </ul>");
    SEND(&s->sout);
  }
#endif /* UIP_SR_LINK_NUM != 0 */

#if (WEB_SERVER_STAT != 0)
  ADD("  SF driver statistics\n  <ul>\n");
  SEND(&s->sout);
  uint32_t utilization = \
    ((osf_stat.osf_mac_tx_total + osf_stat.osf_mac_rx_total) * 100) / osf_stat.osf_mac_t_total;  
  ADD("<li>");
  ADD("T rounds : %ld, utilization %ld %%", osf_stat.osf_mac_t_total, utilization);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  uint32_t nok = (osf_stat.osf_mac_no_ack_total * 100) / osf_stat.osf_mac_tx_total;
  ADD("TX : %ld, rettra %ld, ack %ld, nack %ld ( %ld %% )", \
      osf_stat.osf_mac_tx_total, osf_stat.osf_mac_tx_ret_total, osf_stat.osf_mac_ack_total, osf_stat.osf_mac_no_ack_total, nok);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("RX : %ld, dup %ld, wrong_addr %ld, mcast %ld, too_big %ld", \
      osf_stat.osf_mac_rx_total, osf_stat.osf_rx_dup_total, osf_stat.osf_rx_wrong_addr_total, \
      osf_stat.osf_rx_multicast_total, osf_stat.osf_rx_too_big_total);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  uint32_t ncrc_total = osf_stat.osf_rx_tx_crc_error_total + osf_stat.osf_rx_sync_crc_error_total + osf_stat.osf_rx_ack_crc_error_total;
  uint32_t ncrc = (osf_stat.osf_rx_tx_crc_error_total * 100) / osf_stat.osf_mac_rx_slots_total;
  ADD("RX slots : crc_errs %ld, S %ld, A %ld, T %ld ( %ld %% )", 
      ncrc_total, osf_stat.osf_rx_sync_crc_error_total, osf_stat.osf_rx_ack_crc_error_total, osf_stat.osf_rx_tx_crc_error_total, ncrc);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("Queue : RX peak %ld, TX peak %ld", osf_stat.osf_rx_queue_peak, osf_stat.osf_tx_queue_peak);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("Timeouts : rx_abs %ld, tx_abs %ld", osf_stat.osf_rt_miss_rx_total, osf_stat.osf_rt_miss_tx_total);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("Timeouts miss : epochs %ld, rounds %ld, slots %ld", osf_stat.osf_rt_miss_epoch_total, osf_stat.osf_rt_miss_round_total, osf_stat.osf_rt_miss_slot_total);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("Timeouts : rx %ld, glossy %ld", osf_stat.osf_rt_miss_timeout_total, osf_stat.osf_rt_miss_glossy_total);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("Join : %ld, lost_sync_frames %ld, err_epoch %ld", osf_stat.osf_join_total, osf_stat.osf_ts_lost_total, osf_stat.osf_sync_epoch_err_total);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("  </ul>");
  SEND(&s->sout);
#if UIP_STATISTICS
  ADD("  uIP statistics\n  <ul>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("IP : sent %ld, received %ld, forwarded %ld", uip_stat.ip.sent, uip_stat.ip.recv, uip_stat.ip.forwarded);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("IP : drop %ld", uip_stat.ip.drop);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("IP : vhlerr %ld, hblenerr %ld, lblenerr %ld", uip_stat.ip.vhlerr, uip_stat.ip.hblenerr, uip_stat.ip.lblenerr);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("IP : fragerr %ld, chkerr %ld, protoerr %ld", uip_stat.ip.fragerr, uip_stat.ip.chkerr, uip_stat.ip.protoerr);
  ADD("</li>\n");
  SEND(&s->sout);
  /*-----------*/
  ADD("<li>");
  ADD("ICMP : sent %ld, received %ld", uip_stat.icmp.sent, uip_stat.icmp.recv);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("ICMP : drop %ld", uip_stat.icmp.drop);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("ICMP : typeerr %ld, chkerr %ld", uip_stat.icmp.typeerr, uip_stat.icmp.chkerr);
  ADD("</li>\n");
  SEND(&s->sout);
  /*-----------*/
#if UIP_UDP  
  ADD("<li>");
  ADD("UDP : sent %ld, received %ld", uip_stat.udp.sent, uip_stat.udp.recv);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("UDP : drop %ld, chkerr %ld", uip_stat.udp.drop, uip_stat.udp.chkerr);
  ADD("</li>\n");
  SEND(&s->sout);  
#endif /* UIP_UDP */
  /*-----------*/ 
#if UIP_TCP
  ADD("<li>");
  ADD("TCP : sent %ld, received %ld", uip_stat.tcp.sent, uip_stat.tcp.recv);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("TCP : drop %ld", uip_stat.tcp.drop);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("TCP : chkerr %ld, ackerr %ld, rst %ld", uip_stat.tcp.chkerr, uip_stat.tcp.ackerr, uip_stat.tcp.rst);
  ADD("</li>\n");
  SEND(&s->sout);
  ADD("<li>");
  ADD("TCP : rexmit %ld, syndrop %ld, synrst %ld", uip_stat.tcp.rexmit, uip_stat.tcp.syndrop, uip_stat.tcp.synrst);
  ADD("</li>\n");
  SEND(&s->sout);
#endif /*UIP_TCP */
  /*-----------*/
   ADD("<li>");
  ADD("ND6 : sent %ld, received %ld, drop %ld", uip_stat.nd6.sent, uip_stat.nd6.recv, uip_stat.nd6.drop);
  ADD("</li>\n");
  SEND(&s->sout);   
#endif /* UIP_STATISTICS */
#endif /* WEB_SERVER_STAT != 0 */

  SEND_STRING(&s->sout, BOTTOM);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
PROCESS(webserver_nogui_process, "Web server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  LOG_INFO("Start Webserver, port 80\r\n");

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return generate_routes;
}
/*---------------------------------------------------------------------------*/
