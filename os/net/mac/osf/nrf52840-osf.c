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
 *         OSF nRF52840 driver (originally forked from the Blueflood
 *         nrf-radio-driver.c).
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 *         Beshr Al Nahas <beshr@chalmers.se>
 */

#include <inttypes.h>

#include "contiki.h"
#include "contiki-net.h"
#include "node-id.h"
#include "dev/leds.h"
#include "watchdog.h"

#include "nrf_radio.h"
#include "nrf_ppi.h"
#include "nrf_timer.h"
#include "nrf_clock.h"
#include "net/mac/osf/nrf52840-osf.h"

#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-packet.h"
#include "net/mac/osf/osf-timer.h"
#include "net/mac/osf/osf-ch.h"
#include "net/mac/osf/osf-debug.h"
#include "net/mac/osf/osf-stat.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "NRF52840-OSF"
#define LOG_LEVEL LOG_LEVEL_NONE

#define UNUSED(x) (void)(x)

osf_phy_conf_t *osf_phy_conf;

/*---------------------------------------------------------------------------*/
static void print_radio_config();

/*---------------------------------------------------------------------------*/
static uint32_t swap_bits(uint32_t inp)
{
  uint32_t i;
  uint32_t retval = 0;

  inp = (inp & 0x000000FFUL);

  for (i = 0; i < 8; i++) {
    retval |= ((inp >> i) & 0x01) << (7 - i);
  }

  return retval;
}

/*---------------------------------------------------------------------------*/
static uint32_t bytewise_bitswap(uint32_t inp)
{
  return (swap_bits(inp >> 24) << 24) | (swap_bits(inp >> 16) << 16)
      | (swap_bits(inp >> 8) << 8) | (swap_bits(inp));
}


/*---------------------------------------------------------------------------*/
static nrf_radio_packet_conf_t nrf_phy_conf_2M = {
  0, // lflen
  0, // s0len (bytes)
  0, // s1len (bits)
  RADIO_PCNF0_S1INCL_Automatic, // s1incl
  0, // cilen
  RADIO_PCNF0_PLEN_16bit,       // plen
  RADIO_PCNF0_CRCINC_Exclude,   // crcincl
  0, // termlen
  255, // maxlen
  255, // statlen
  2, // balen
  RADIO_PCNF1_ENDIAN_Big,       // big_endian
  OSF_PHY_WHITENING             // whiten
};

osf_phy_conf_t osf_phy_conf_2M = {
  "osf_phy_conf_2M",
  RADIO_MODE_MODE_Ble_2Mbit,
  &nrf_phy_conf_2M,
  RADIO_CRCCNF_LEN_Two, RADIO_CRCCNF_SKIPADDR_Skip, 0x11021UL, 0xFFFFUL,
  US_TO_RTIMERTICKSX(20),       // header_air_ticks - 2B PREAMBLE + 2B BA + 1B PREFIX (5*8/2)
  0,                            // post_addr_air_ticks
  0,                            // payload_air_ticks
  US_TO_RTIMERTICKSX(8),        // footer_air_ticks - 2B CRC
  92,                           // tx_rx_addr_offset_ticks - 5.746us measured on LA, average over 10 rounds)
  85,                           // tx_rx_end_offset_ticks - 5.284us (measured on LA)
  0,                            // packet_air_ticks
  0                             // slot_duration
};

/*---------------------------------------------------------------------------*/
static nrf_radio_packet_conf_t nrf_phy_conf_1M = {
  0, // lflen
  0, // s0len (bytes)
  0, // s1len (bits)
  RADIO_PCNF0_S1INCL_Automatic, // s1incl
  0, // cilen
  RADIO_PCNF0_PLEN_8bit,        // plen
  RADIO_PCNF0_CRCINC_Exclude,   // crcincl
  0, // termlen
  255, // maxlen
  255, // statlen
  2, // balen
  RADIO_PCNF1_ENDIAN_Big,       // big_endian
  OSF_PHY_WHITENING             // whitee
};

osf_phy_conf_t osf_phy_conf_1M = {
  "osf_phy_conf_1M",
  RADIO_MODE_MODE_Ble_1Mbit,
  &nrf_phy_conf_1M,
  RADIO_CRCCNF_LEN_Two, RADIO_CRCCNF_SKIPADDR_Skip,  0x11021UL, 0xFFFFUL,
  US_TO_RTIMERTICKSX(32),       // header_air_ticks - 1B PREAMBLE + 2B BA + 1B PREFIX (4*8)
  0,                            // post_addr_air_ticks
  0,                            // payload_air_ticks
  US_TO_RTIMERTICKSX(16),       // footer_air_ticks - 2B CRC
  174,                          // tx_rx_addr_offset_ticks - 10.856us (measured on LA, average over 10 rounds)
  158,                          // tx_rx_end_offset_ticks - 9.9us (measured on LA)
  0,                            // packet_air_ticks
  0                             // slot_duration
};

/*---------------------------------------------------------------------------*/
// NB: STATLEN only works for 500 when S0=1 or LF=8. This shouldn't be the case.
//     IF STATLEN is set, then it doesn't appear to read LF. If STATLEN is
//     not set and LF=8, then it will use the 1st byte in RAM as the length
//     field..
// NB: https://www.edn.com/bluetooth-5-variations-complicate-phy-testing/
static nrf_radio_packet_conf_t nrf_phy_conf_500K = {
  0, // lflen
  0, // s0len (bytes)
  0, // s1len (bits)
  RADIO_PCNF0_S1INCL_Automatic, // s1incl
  2, // cilen
  RADIO_PCNF0_PLEN_LongRange,   // plen
  RADIO_PCNF0_CRCINC_Exclude,   // crcincl
  3, // termlen
  255, // maxlen
  255, // statlen
  3, // balen
  RADIO_PCNF1_ENDIAN_Big,       // big_endian
  OSF_PHY_WHITENING             // whiten
};

osf_phy_conf_t osf_phy_conf_500K = {
  "osf_phy_conf_500K",
  RADIO_MODE_MODE_Ble_LR500Kbit,
  &nrf_phy_conf_500K,
  RADIO_CRCCNF_LEN_Three, RADIO_CRCCNF_SKIPADDR_Skip, 0x0100065B, 0x555555UL,
  // US_TO_RTIMERTICKSX(336),       // header_air_ticks - 10B PRE @1M + 3B BA and 1B PREFIX @125K (80us + 256us)
  US_TO_RTIMERTICKSX(324) + 2,  // header_air_ticks - MEASURED 324.128us ON LA (READY to ADDR). Why? Only Nordic knows.
  // US_TO_RTIMERTICKSX(104),       // post_addr_air_ticks - 2bit CI and 3bit TERM1 + 1B S0 @125K (16us + 24us + 64us).
  // US_TO_RTIMERTICKSX(67),       // post_addr_air_ticks - 67us on LA. Should be 104us. Why? Only Nordic knows. -3us coming from somewhere.
  0,                            // post_addr_air_ticks - Calculated in my_radio_set_phy_airtime();
  0,                            // payload_air_ticks
  // US_TO_RTIMERTICKSX(54),       // footer_air_ticks - 3B CRC + 3bit TERM2 @500K (48us + 6us)
  US_TO_RTIMERTICKSX(57),       // footer_air_ticks - 57us on LA. Should be 54us. Why? Only Nordic knows. +3us coming from somewhere.
  1188,                         // tx_rx_addr_offset_ticks - 74.263us (measured on LA, average over 10 rounds)
  396,                          // tx_rx_end_offset_ticks - 24.74us (measured on LA)
  0,                            // packet_air_ticks
  0                             // slot_duration
};

/*---------------------------------------------------------------------------*/
static nrf_radio_packet_conf_t nrf_phy_conf_125K = {
  0, // lflen
  0, // s0len (bytes)
  0, // s1len (bits)
  RADIO_PCNF0_S1INCL_Automatic, // s1incl
  2, // cilen
  RADIO_PCNF0_PLEN_LongRange,   // plen
  RADIO_PCNF0_CRCINC_Exclude,   // crcincl
  3, // termlen
  255, // maxlen
  255, // statlen
  3, // balen
  RADIO_PCNF1_ENDIAN_Big,       // big_endian
  OSF_PHY_WHITENING             // whiten
};

osf_phy_conf_t osf_phy_conf_125K = {
  "osf_phy_conf_125K",
  RADIO_MODE_MODE_Ble_LR125Kbit,
  &nrf_phy_conf_125K,
  RADIO_CRCCNF_LEN_Three, RADIO_CRCCNF_SKIPADDR_Skip, 0x0100065B, 0x555555UL,
  // US_TO_RTIMERTICKSX(336),      // header_air_ticks - 10B PRE @1M + 3B BA and 1B PREFIX @125K (80us + 256us)
  US_TO_RTIMERTICKSX(324) + 2,  // header_air_ticks - MEASURED 324.128us ON LA (READY to ADDR). Why? Only Nordic knows.
  // US_TO_RTIMERTICKSX(40),       // post_addr_air_ticks - 2bit CI and 3bit TERM1 @125K (16us + 24 us)
  // US_TO_RTIMERTICKSX(48),       // post_addr_air_ticks - 48us on LA. Should be 40us. Why? Only Nordic knows. A single bit must be coming from somewhere.
  0,                            // post_addr_air_ticks - Calculated in my_radio_set_phy_airtime();
  0,                            // payload_air_ticks
  US_TO_RTIMERTICKSX(216),      // footer_air_ticks - 3B CRC (192) + 3bit TERM2 (24)
  1187,                         // tx_rx_addr_offset_ticks - 74.217us (measured on LA, average over 10 rounds)
  566,                          // tx_rx_end_offset_ticks - 35.416us (measured on LA)
  0,                            // packet_air_ticks
  0                             // slot_duration
};

/*---------------------------------------------------------------------------*/
static nrf_radio_packet_conf_t nrf_phy_conf_802154 = {
  0, // lflen
  0, // s0len (bytes)
  0, // s1len (bits)
  RADIO_PCNF0_S1INCL_Automatic, // s1incl
  0, // cilen
  RADIO_PCNF0_PLEN_32bitZero,   // plen
  RADIO_PCNF0_CRCINC_Exclude,   // crcincl
  0, // termlen
  127, // maxlen
  127, // statlen
  0, // balen
  RADIO_PCNF1_ENDIAN_Little,    // big_endian
  RADIO_PCNF1_WHITEEN_Disabled  // whiten
};

osf_phy_conf_t osf_phy_conf_802154 = {
  "osf_phy_conf_802154",
  RADIO_MODE_MODE_Ieee802154_250Kbit,
  &nrf_phy_conf_802154,
  RADIO_CRCCNF_LEN_Two, RADIO_CRCCNF_SKIPADDR_Ieee802154, 0x11021, 0x0UL,
  US_TO_RTIMERTICKSX(140),      // header_air_ticks - 4B PRE + 1B SFD  @250K (128us +32 us)
  // 2241,                         // header_air_ticks - MEASURED 140.064us on LA (READY to ADDR). Why? Only Nordic knows.
  0,                            // post_addr_air_ticks -  Calculated in my_radio_set_phy_airtime();
  0,                            // payload_air_ticks
  US_TO_RTIMERTICKSX(64),       // footer_air_ticks - 2B CRC @256K
  692,                          // tx_rx_addr_offset_ticks - 43.242us (measured on LA, average over 10 rounds)
  626,                          // tx_rx_end_offset_ticks - 39.156us (measured on LA)
  0,                            // packet_air_ticks
  0                             // slot_duration
};

/*---------------------------------------------------------------------------*/

static uint8_t nrf_tx_power = OSF_TXPOWER;

/*---------------------------------------------------------------------------*/
void
set_txpower(uint8_t p)
{
  nrf_tx_power = p;
}

/*---------------------------------------------------------------------------*/
static void
configure_errata(osf_phy_conf_t *conf) {
  /*
   * Workarounds should be applied only when switching to/from LE Coded PHY
   * so no need to apply them every time.
   *
   * nRF52840 Rev 2, build codes CKAA-Dx0, QIAA-Dx0
   * [153] RADIO: RSSI parameter adjustment
   * RSSI changes over temperature.
   * Temperature ≤ +10°C or > +30°C.
   * Add the following compensation to the RSSI sample value based on temperature measurement
   * (the onchip TEMP peripheral can be used to measure temperature):
     • For TEMP ≤ -30°C, RSSISAMPLE = RSSISAMPLE +3
     • For TEMP > -30°C and TEMP ≤ -10°C, RSSISAMPLE = RSSISAMPLE +2
     • For TEMP > -10°C and TEMP ≤ +10°C, RSSISAMPLE = RSSISAMPLE +1
     • For TEMP > +10°C and TEMP ≤ +30°C, RSSISAMPLE = RSSISAMPLE + 0
     • For TEMP > +30°C and TEMP ≤ +50°C, RSSISAMPLE = RSSISAMPLE - 1
     • For TEMP > +50°C and TEMP ≤ +70°C, RSSISAMPLE = RSSISAMPLE - 2
     • For TEMP > +70°C, RSSISAMPLE = RSSISAMPLE - 3
   *
   * nRF52840 Engineering A Errata v1.2
   * [164] RADIO: Low sensitivity in long range mode
   *
   * RF52840 Rev 2, build codes CKAA-Dx0, QIAA-Dx0
   * [172] RADIO: BLE long range co-channel performance
   *
   * nRF52840 Rev 1 Errata
   * nRF52840 Rev 2, build codes CKAA-Dx0, QIAA-Dx0
   * [191] RADIO: High packet error rate in BLE Long Range mode
   * BLE long range (Ble_LR125Kbit or Ble_LR500Kbit).
   * Blocker signal present at the same or nearby RF frequency.
   * Fails BLE test with co-channel interference (RF-PHY/RCV/BV-29-C) without FW workaround.
   * Workaround is incorporated into S140 SoftDevice v6.1.1 and the DTM example in SDK v15.3.0. See the
   * following document for a description of the workarounds:
   * nRF52840 Errata Attachment Anomaly 172 Addendum
   * https://infocenter.nordicsemi.com/pdf/nRF52_PAN_172_add_v1.0.pdf
   *
   * nRF52840 Rev 2, build codes CKAA-Dx0, QIAA-Dx0
   * [204] RADIO: Switching between TX and RX causes unwanted emissions
   * Always use DISABLE when switching from TX to RX.
   *
   * nRF52840 Rev 2, build codes CKAA-Dx0, QIAA-Dx0
   * [245] RADIO: CRC is wrong when data whitening is
   * enabled and address field is included in CRC calculation
   * In RX, if data whitening is enabled and the CRC checker is configured to take
   * the address field into CRC calculations.
   * CRC failures are reported though received packet contents are good.
   *
   * This configuration applies to IC Rev. Revision 3, build codes QFAA-Fx0, QIAA-Fx0, CKAA-Fx0.
   * [254] RADIO: External PAs, FEMs, and LNAs need additional Radio configuration
   */
  switch (conf->mode) {
    case PHY_BLE_1M:
    case PHY_BLE_2M:
      /* [164] */
      // *(volatile uint32_t *)0x4000173C &= ~0x80000000;
      /* [191] */
      *(volatile uint32_t *)0x40001740 = ((*((volatile uint32_t *)0x40001740)) & 0x7FFFFFFFUL);
      break;
    case PHY_BLE_500K:
    case PHY_BLE_125K:
      /* [164] */
      // *(volatile uint32_t *)0x4000173C |= 0x80000000;
      // *(volatile uint32_t *)0x4000173C = ((*(volatile uint32_t *)0x4000173C & 0xFFFFFF00) | 0x5C);
      /* [191] */
      *(volatile uint32_t *)0x40001740 = ((*((volatile uint32_t *)0x40001740)) & 0x7FFF00FF) |  0x80000000 | (((uint32_t)(196)) << 8);
      break;
    case PHY_IEEE:
      /* [191] */
      *(volatile uint32_t *)0x40001740 = ((*((volatile uint32_t *)0x40001740)) & 0x7FFFFFFF);
      break;
    default:
      break;
  }
}

/*---------------------------------------------------------------------------*/
static void
configure_phy(osf_phy_conf_t *phy, uint8_t len, uint8_t statlen)
{
  NRF_RADIO->MODE = phy->mode;
  /* Configure nrf phy conf */
  switch(phy->mode) {
    case PHY_BLE_2M:
    case PHY_BLE_1M:
      NRF_RADIO->SHORTS |= RADIO_SHORTS_END_DISABLE_Msk;
      break;
    case PHY_BLE_500K:
    case PHY_BLE_125K:
    case PHY_IEEE:
      NRF_RADIO->SHORTS |= RADIO_SHORTS_PHYEND_DISABLE_Msk;
      /* For BLE LR and IEEE only center frequency is valid */
      NRF_RADIO->MODECNF0 |= (RADIO_MODECNF0_DTX_Center << RADIO_MODECNF0_DTX_Pos);
      break;
    default:
      LOG_ERR("Unknown PHY configuration (%u)!\n", phy->mode);
      break;
  }
  /* Set phy conf fields */
  phy->conf->lflen = (!statlen) ? 8 : 0;
  phy->conf->s0len = (!statlen || phy->mode == PHY_BLE_500K) ? 1 : 0;
  phy->conf->s1len = (!statlen && OSF_PACKET_WITH_S1) ? 1 : 0;
  phy->conf->statlen = (!statlen) ? 0 : len;
  phy->conf->maxlen = (!statlen) ? OSF_MAXLEN(phy->mode) : len;
  phy->conf->crcinc = (!statlen && phy->mode == PHY_IEEE) ? RADIO_PCNF0_CRCINC_Include : RADIO_PCNF0_CRCINC_Exclude;
  /* For statlen our payload air ticks can be our actual length */
  if(!statlen) {
    if(phy->mode != PHY_IEEE) {
      osf_buf_phy->ble.s0 = 1;
      osf_buf_phy->ble.len = len;
#if OSF_PACKET_WITH_S1
      osf_buf_phy->ble.s1 = 1;
#endif
    } else {
      phy->conf->crcinc = RADIO_PCNF0_CRCINC_Include;
      osf_buf_phy->ieee.len = len + phy->crclen;
    }
  }
  my_radio_set_phy_airtime(phy, len, statlen);
  /* Calculate total slot time */
  phy->packet_air_ticks = phy->header_air_ticks
                          + phy->post_addr_air_ticks
                          + phy->payload_air_ticks
                          + phy->footer_air_ticks;
  phy->slot_duration = phy->packet_air_ticks + OSF_TIFS_TICKS;
  /* Configure CRC */
  nrf_radio_packet_configure(phy->conf);
  nrf_radio_crc_configure(phy->crclen, phy->crcaddr, phy->crcpoly);
  nrf_radio_crcinit_set(phy->crcinit);
}

/*---------------------------------------------------------------------------*/
void
configure_address(osf_phy_conf_t *conf, uint8_t addr)
{
  /* In general it is bad practice to start an address with either 0x00, 0xFF,
     0xA or 0x5. 0x00 and 0xFF because they contain no bit shifts, and 0xA or
     0x5 because they form a continuation of the preamble (the preamble is
     either 0xAA or 0x55). */
  if (conf->mode != PHY_IEEE) {
    /* Radio address config */
    NRF_RADIO->PREFIX0 = ((uint32_t) swap_bits(0xC3) << 24) // Prefix byte of address 3 converted to nRF24L series format
        | ((uint32_t) swap_bits(0xC2) << 16) // Prefix byte of address 2 converted to nRF24L series format
        | ((uint32_t) swap_bits(0xC1) << 8)  // Prefix byte of address 1 converted to nRF24L series format
        | ((uint32_t) swap_bits(0xC0) << 0); // Prefix byte of address 0 converted to nRF24L series format

    NRF_RADIO->PREFIX1 = ((uint32_t) swap_bits(0xC7) << 24) // Prefix byte of address 7 converted to nRF24L series format
        | ((uint32_t) swap_bits(0xC6) << 16) // Prefix byte of address 6 converted to nRF24L series format
        | ((uint32_t) swap_bits(0xC4) << 0); // Prefix byte of address 4 converted to nRF24L series format
    NRF_RADIO->BASE0 = bytewise_bitswap(0x01234050UL); // Base address for prefix 0 converted to nRF24L series format
    NRF_RADIO->BASE1 = bytewise_bitswap(0x89ABCDEFUL); // Base address for prefix 1-7 converted to nRF24L series format
    /* Seed the address with a custom address byte.
       FIXME: I've tried just or-ing the last byte, but I guess the addresses
             aren't sufficiently different to prevent flipped bits from making
             us think we are on the correct round when actually we aren't. For now,
             let's just put some random addresses in. */
    if(addr == 0) {
      NRF_RADIO->BASE0 = bytewise_bitswap(0x01234050UL);
    } else if (addr == 1) {
      NRF_RADIO->BASE0 = bytewise_bitswap(0x02135060UL);
    } else {
      NRF_RADIO->BASE0 = bytewise_bitswap(0xBF943732UL);
    }
    /* Use logical address 0 (prefix0 + base0) = 0x8E89BED6 when transmitting and receiving */
    NRF_RADIO->TXADDRESS = 0x00UL;
    NRF_RADIO->RXADDRESSES = 0x01UL;
  /* IEEE */
  } else {
    // TODO: What do we actually do for IEEE?
    // NRF_RADIO->SFD = 0xA7UL; // default SFD for 802.15.4, no need to set it explicitly
    NRF_RADIO->SFD = 0xAEUL; // custom SFD for 802.15.4
  }
}

/*---------------------------------------------------------------------------*/
osf_phy_conf_t *
my_radio_get_phy_conf(uint8_t mode)
{
  switch (mode) {
    case PHY_BLE_2M:
      return &osf_phy_conf_2M;
    case PHY_BLE_1M:
      return &osf_phy_conf_1M;
    case PHY_BLE_500K:
      return &osf_phy_conf_500K;
    case PHY_BLE_125K:
      return &osf_phy_conf_125K;
    case PHY_IEEE:
      return &osf_phy_conf_802154;
    default:
      LOG_ERR("Unknown PHY configuration!\n");
      return NULL;
  }
}

/*---------------------------------------------------------------------------*/
rtimer_clock_t
my_radio_set_phy_airtime(osf_phy_conf_t *phy, uint8_t len, uint8_t statlen)
{
  /* NB: If we aren't using statlen we need to set maxlen. This is mainly for the
     slot duration calculations we make in the proto configure() for each round.
     if we don't set it here (even though we set in configure_phy() then the
     incorrect air time is calculated for non-statlen packets. */
  phy->conf->maxlen = (!statlen) ? OSF_MAXLEN(phy->mode) : len;
  /* For static length payloads just use the len */
  if(statlen) {
    if(phy->mode == PHY_BLE_125K) {
      phy->post_addr_air_ticks = OSF_PHY_BITS_TO_RTIMERTICKSX(5, PHY_BLE_125K); // 2bit CI + 3bit TERM1 @125K (16us + 24us).
    } else if (phy->mode == PHY_BLE_500K){
      // NB: WE SHOULDN'T need this but LA shows a missing 27.232us on 500K. FIXME: Only nordic knows why.
      phy->post_addr_air_ticks = OSF_PHY_BITS_TO_RTIMERTICKSX(5, PHY_BLE_125K) + US_TO_RTIMERTICKSX(27) + 3; // 2bit CI + 3bit TERM1 @125K (16us + 24us).
    } else {
      phy->post_addr_air_ticks = 0;
    }
    phy->payload_air_ticks = OSF_PHY_BYTES_TO_RTIMERTICKSX(len, phy->mode);
  /* For variable length payloads we need to assume a worst case (MTU) */
  } else {
    if(phy->mode == PHY_BLE_500K || phy->mode == PHY_BLE_125K) {
      /* Post-address */
      phy->post_addr_air_ticks = OSF_PHY_BITS_TO_RTIMERTICKSX(5, PHY_BLE_125K) // 2bit CI + 3bit TERM1 @125K (16us + 24us).
                                 + OSF_PHY_BYTES_TO_RTIMERTICKSX(1, phy->mode) // S0 @os/net/mac/osfk (optional).
                                 + OSF_PHY_BITS_TO_RTIMERTICKSX(8, phy->mode); // LF @125k (optional).
#if OSF_PACKET_WITH_S1
      phy->post_addr_air_ticks += OSF_PHY_BITS_TO_RTIMERTICKSX(1, phy->mode); // S1 @125k (optional).
#endif
      /* Payload */
      phy->payload_air_ticks = OSF_PHY_BYTES_TO_RTIMERTICKSX(phy->conf->maxlen, phy->mode);
    } else {
      phy->post_addr_air_ticks = OSF_PHY_BYTES_TO_RTIMERTICKSX(OSF_PKT_PHY_LEN(phy->mode, statlen), phy->mode); // S0, LF, and S1 @125k (optional)
      phy->payload_air_ticks = OSF_PHY_BYTES_TO_RTIMERTICKSX(phy->conf->maxlen, phy->mode);
    }
  }
  // NB: We SHOULDN'T need this, but the LA shows a missing...
  //     500K - 11.232us
  //     125K - 8.232us...
  //     IEEE - 4us exactly
  // FIXME: Only Nordic knows why...
  if(phy->mode == PHY_BLE_500K) {
    phy->post_addr_air_ticks += US_TO_RTIMERTICKSX(11) + 3;
  } else if (phy->mode == PHY_BLE_125K) {
    phy->post_addr_air_ticks += US_TO_RTIMERTICKSX(8) + 3;
  } else if (phy->mode == PHY_IEEE) {
    phy->post_addr_air_ticks += US_TO_RTIMERTICKSX(4);
  }
  /* Calculate total slot time */
  phy->packet_air_ticks = phy->header_air_ticks
                          + phy->post_addr_air_ticks
                          + phy->payload_air_ticks
                          + phy->footer_air_ticks;
  phy->slot_duration = phy->packet_air_ticks + OSF_TIFS_TICKS;

  return phy->slot_duration;
}

/*---------------------------------------------------------------------------*/
void
my_radio_init(osf_phy_conf_t *phy_conf, void *my_tx_buffer, uint8_t len, uint8_t statlen, uint8_t addr)
{
  UNUSED(print_radio_config);

  /* Disable and clear all interrupts */
  NVIC_DisableIRQ(RADIO_IRQn);
  nrf_radio_int_disable(0xFFFFFFFF);
  NVIC_ClearPendingIRQ(RADIO_IRQn);
  NVIC_SetPriority(RADIO_IRQn, 0);

  /* Reset all states in the radio peripheral */
  NRF_RADIO->POWER = 0UL;
  NRF_RADIO->POWER = 1UL;

#if RADIO_TXRX_PIN
  // TODO: Should move all debug stuff to debug
  NRF_GPIOTE->CONFIG[RADIO_TXRX_GPIOTE_CH] = (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) | (RADIO_TXRX_PIN << GPIOTE_CONFIG_PSEL_Pos);
#endif

  /* TX power */
  set_txpower(OSF_TXPOWER);
  NRF_RADIO->TXPOWER = nrf_tx_power;

  /* NB: Default ch for now, set to correct channel on TX/RX */
  NRF_RADIO->FREQUENCY = CH_DEFAULT;
#if OSF_PHY_WHITENING
  NRF_RADIO->DATAWHITEIV = CH_DEFAULT;
#endif

  /* Fast RU (40us) */
  NRF_RADIO->MODECNF0 = (RADIO_MODECNF0_RU_Fast << RADIO_MODECNF0_RU_Pos);
  /* Pull instructions from cache rather than flash */
  NRF_NVMC->ICACHECNF = NVMC_ICACHECNF_CACHEEN_Msk;
  /* Constant CPU wakeup latency. */
  NRF_POWER->TASKS_CONSTLAT = 1UL;

  /* Configure physical layer params */
  if(phy_conf != NULL) {
    osf_phy_conf = phy_conf;
    configure_phy(phy_conf, len, statlen);
    configure_errata(phy_conf);
    configure_address(phy_conf, addr);
  } else {
    osf_phy_conf = NULL;
  }

  /* Configure buffer */
  NRF_RADIO->PACKETPTR = (uint32_t)my_tx_buffer;

  /* Create a short between ready and start */
  NRF_RADIO->SHORTS |= RADIO_SHORTS_READY_START_Msk;
  /* RSSI shorts (takes 8us) - in case of 802.15.4, the PPI starts RSSISTART upon recieving a FRAMESTART event */
  NRF_RADIO->SHORTS |= RADIO_SHORTS_ADDRESS_RSSISTART_Msk;

  /* Configure PPI for RADIO EVENTS */
  // NRF_RADIO->INTENSET = RADIO_INTENSET_READY_Msk | RADIO_INTENSET_END_Msk;
  /* Ready Event */
  NRF_PPI->CH[RADIO_READY_CH].EEP = (uint32_t)&NRF_RADIO->EVENTS_READY;
  NRF_PPI->CH[RADIO_READY_CH].TEP = (uint32_t)&NRF_TIMERX->TASKS_CAPTURE[TIMESTAMP_REG];
  /* End Event */
  NRF_PPI->CH[RADIO_END_CH].EEP = (uint32_t)&NRF_RADIO->EVENTS_END;
  NRF_PPI->CH[RADIO_END_CH].TEP = (uint32_t)&NRF_TIMERX->TASKS_CAPTURE[TIMESTAMP_REG];
  /* Address Event */
  NRF_PPI->CH[RADIO_ADDRESS_CH].EEP = (uint32_t)&NRF_RADIO->EVENTS_ADDRESS;
  // NRF_RADIO->INTENSET |= RADIO_INTENSET_ADDRESS_Msk;
  // FIXME: FRAMESTART doesn't seem to work with IEEE. However ADDRESS works
  //        just fine. While this isn't an issue at the moment, if we want
  //        adhere to the standard (e.g., to listen for std packets) then this
  //        needs fixed.
  /* Address/Framestart Event */ //
  // if(phy_conf->mode == PHY_IEEE) {
  //   NRF_PPI->CH[RADIO_ADDRESS_CH].EEP = (uint32_t)&NRF_RADIO->EVENTS_FRAMESTART;
  //   NRF_RADIO->INTENSET |= RADIO_INTENSET_FRAMESTART_Msk;
  // } else {
  //   NRF_PPI->CH[RADIO_ADDRESS_CH].EEP = (uint32_t)&NRF_RADIO->EVENTS_ADDRESS;
  //   NRF_RADIO->INTENSET |= RADIO_INTENSET_ADDRESS_Msk;
  // }

  NRF_PPI->CH[RADIO_ADDRESS_CH].TEP = (uint32_t)&NRF_TIMERX->TASKS_CAPTURE[TIMESTAMP_REG];
  NRF_PPI->CHENSET = (1UL << RADIO_READY_CH) | (1UL << RADIO_ADDRESS_CH) | (1UL << RADIO_END_CH);

  /* Configure PPI for RADIO TX/RX */
  NRF_PPI->CH[RADIO_TXEN_CH].EEP = (uint32_t)&NRF_TIMERX->EVENTS_COMPARE[SCHEDULE_REG];
  NRF_PPI->CH[RADIO_TXEN_CH].TEP = (uint32_t)&NRF_RADIO->TASKS_TXEN;
  NRF_PPI->CH[RADIO_RXEN_CH].EEP = (uint32_t)&NRF_TIMERX->EVENTS_COMPARE[SCHEDULE_REG];
  NRF_PPI->CH[RADIO_RXEN_CH].TEP = (uint32_t)&NRF_RADIO->TASKS_RXEN;
  /* Reinit radio interrupts */
  // NVIC_EnableIRQ(RADIO_IRQn);
}
/*---------------------------------------------------------------------------*/
/* TX at t_abs */
uint8_t
schedule_tx_abs(uint8_t *buf, uint8_t channel, rtimer_clock_t t_abs)
{
  my_radio_off_to_tx();
  /* Check we can set the timer. We don't use rimterx_set() here as we are
     already interrupting on EVENTS_READY */
  uint8_t r;
  rtimer_clock_t now = rtimerx_now();
  /* ERR - Not enough time to schedule timer */
  if(RTIMER_CLOCK_LT(t_abs, now)) {
    LOG_DBG("{%u|ep-%-4u} rt miss TX %s slot:%u | d:+%lu us e_ref:+%lu us t_ref:+%lu us\n",
      node_id, osf.epoch, OSF_ROUND_TO_STR(osf.round->type), osf.slot, RTIMERTICKS_TO_USX(now - t_abs), RTIMERTICKS_TO_USX(now - osf.t_epoch_ref), RTIMERTICKS_TO_USX(now - t_ref));
    r = RTIMER_ERR_TIME;
    osf_stat.osf_rt_miss_tx_total++;/* Statistics */
  /* OK - Schedule RX*/
  } else {
    NRF_RADIO->TASKS_DISABLE = 1UL;
    NRF_TIMERX->EVENTS_COMPARE[SCHEDULE_REG] = 0UL;
    NRF_TIMERX->CC[SCHEDULE_REG] = t_abs;
    NRF_RADIO->FREQUENCY = channel;
  #if OSF_PHY_WHITENING
    NRF_RADIO->DATAWHITEIV = channel;
  #endif
    NRF_RADIO->PACKETPTR = (uint32_t)buf;
    r = RTIMER_OK;

    nrf_radio_int_enable(RADIO_INTENSET_READY_Msk | RADIO_INTENSET_END_Msk | RADIO_INTENSET_ADDRESS_Msk);
    NVIC_EnableIRQ(RADIO_IRQn);
  }

  return r;
}
/*---------------------------------------------------------------------------*/
void
my_radio_off_completely()
{
  /* Disable RADIO_IRQ */
  NVIC_DisableIRQ(RADIO_IRQn);
  nrf_radio_int_disable(0xFFFFFFFF);
  NVIC_ClearPendingIRQ(RADIO_IRQn);

  /* Disable scheduled TX/RX */
  NRF_PPI->CHENCLR = (1UL << RADIO_TXEN_CH);
  NRF_PPI->CHENCLR = (1UL << RADIO_RXEN_CH);

  NRF_TIMERX->EVENTS_COMPARE[SCHEDULE_REG] = 0UL;

  /* Clear event register */
  NRF_RADIO->EVENTS_DISABLED = 0UL;

  /* Clear TX/RXEN (just in case it's set) */
  NRF_RADIO->TASKS_RXEN = 0UL;
  NRF_RADIO->TASKS_TXEN = 0UL;

  /* Disable Radio */
  NRF_RADIO->TASKS_DISABLE = 1UL;
  NRF_RADIO->EVENTS_END = 0UL;
  NRF_RADIO->EVENTS_ADDRESS = 0UL;
  NRF_RADIO->EVENTS_PAYLOAD = 0UL;
  NRF_RADIO->EVENTS_READY = 0UL;
  NRF_RADIO->EVENTS_FRAMESTART = 0UL;
  NRF_RADIO->EVENTS_PHYEND = 0UL;
  NRF_RADIO->EVENTS_CRCOK = 0UL;
  NRF_RADIO->EVENTS_CRCERROR = 0UL;

#if OSF_DEBUG_GPIO
#if RADIO_TXEN_PIN
  NRF_PPI->CHENCLR = (1UL << RADIO_T0_TX_EVENT_PPI_CH); /* disable TXen debug pin PPI */
#endif
#if RADIO_RXEN_PIN
  NRF_PPI->CHENCLR = (1UL << RADIO_T0_RX_EVENT_PPI_CH); /* disable RXen debug pin PPI */
#endif
#if RADIO_TXRX_PIN
  NRF_GPIOTE->CONFIG[RADIO_TXRX_GPIOTE_CH] &= ~((RADIO_TXRX_PIN << GPIOTE_CONFIG_PSEL_Pos) | (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos)); /* disable TXRXen debug PPI */
#endif
#endif /* OSF_DEBUG_GPIO */

  NRF_RADIO->EVENTS_DISABLED = 0UL;
  NRF_RADIO->POWER = 0UL;
}
/*---------------------------------------------------------------------------*/
void
my_radio_off_to_tx()
{
  /* Disable and clear all interrupts */
  NVIC_DisableIRQ(RADIO_IRQn);
  nrf_radio_int_disable(0xFFFFFFFF);
  NVIC_ClearPendingIRQ(RADIO_IRQn);

  /* Enable scheduled TX : TimerX-compare[0] -> TxEn */
  NRF_PPI->CHENSET = (1UL << RADIO_TXEN_CH);
  /* Disable RX scheduling shortcut */
  NRF_PPI->CHENCLR = (1UL << RADIO_RXEN_CH);

#if OSF_DEBUG_GPIO
#if RADIO_TXEN_PIN
/* Enable TXen debug pin PPI --> Timer 0 compare event toggles TXen debug pin */
  NRF_PPI->CHENSET = (1UL << RADIO_T0_TX_EVENT_PPI_CH);
#endif
#if RADIO_RXEN_PIN
  /* Disable RXen debug pin PPI */
  NRF_PPI->CHENCLR = (1UL << RADIO_T0_RX_EVENT_PPI_CH);
#endif
#endif /* OSF_DEBUG_GPIO */

  NRF_TIMERX->EVENTS_COMPARE[SCHEDULE_REG] = 0UL;

  /* Clear event register */
  NRF_RADIO->EVENTS_DISABLED = 0UL;

  /* Disable radio */
  NRF_RADIO->TASKS_DISABLE = 1UL;

  NRF_RADIO->EVENTS_END = 0UL;
  NRF_RADIO->EVENTS_ADDRESS = 0UL;
  NRF_RADIO->EVENTS_READY = 0UL;
  NRF_RADIO->EVENTS_FRAMESTART = 0UL;
  NRF_RADIO->EVENTS_PHYEND = 0UL;
  NRF_RADIO->EVENTS_CRCOK = 0UL;
  NRF_RADIO->EVENTS_CRCERROR = 0UL;

  NRF_RADIO->TXPOWER = nrf_tx_power;

  NRF_RADIO->EVENTS_DISABLED = 0UL;
}
/*---------------------------------------------------------------------------*/
/* RX at t_abs */
uint8_t
schedule_rx_abs(uint8_t *buf, uint8_t channel, rtimer_clock_t t_abs)
{
  my_radio_off_to_rx();
  /* Check we can set the timer. We don't use rimterx_set() here as we are
     already interrupting on EVENTS_READY */
  uint8_t r;
  rtimer_clock_t now = rtimerx_now();
  /* ERR - Not enough time to schedule timer */
  if(RTIMER_CLOCK_LT(t_abs, now)) {
    LOG_DBG("{%u|ep-%-4u} rt miss RX %s slot:%u offset: %lu | d:+%luus e_ref:+%luus t_ref:+%luus off:%luus ref_shft:%luus\n",
      node_id, osf.epoch, OSF_ROUND_TO_STR(osf.round->type), osf.slot, RTIMERTICKS_TO_USX(osf.rconf->t_offset), RTIMERTICKS_TO_USX(now - t_abs), RTIMERTICKS_TO_USX(now - osf.t_epoch_ref), RTIMERTICKS_TO_USX(t_ref - now), RTIMERTICKS_TO_USX(t_ref - osf.t_epoch_ref), RTIMERTICKS_TO_USX(OSF_REF_SHIFT));
    r = RTIMER_ERR_TIME;
    osf_stat.osf_rt_miss_rx_total++;/* Statistics */
  /* OK - Schedule RX*/
  } else {
    NRF_TIMERX->EVENTS_COMPARE[SCHEDULE_REG] = 0UL;
    NRF_TIMERX->CC[SCHEDULE_REG] = t_abs;
    NRF_RADIO->FREQUENCY = channel;
#if OSF_PHY_WHITENING
    NRF_RADIO->DATAWHITEIV = channel;
#endif
    NRF_RADIO->PACKETPTR = (uint32_t)buf;
    r = RTIMER_OK;

    nrf_radio_int_enable(RADIO_INTENSET_READY_Msk | RADIO_INTENSET_END_Msk | RADIO_INTENSET_ADDRESS_Msk);
    NVIC_EnableIRQ(RADIO_IRQn);
  }

  return r;
}

/*---------------------------------------------------------------------------*/
void
my_radio_rx_hop_channel(uint8_t channel)
{
  NRF_RADIO->TASKS_DISABLE = 1UL;
  NRF_RADIO->FREQUENCY = channel;
#if OSF_PHY_WHITENING
  NRF_RADIO->DATAWHITEIV = channel;
#endif
  NRF_RADIO->TASKS_RXEN = 1UL;
}

/*---------------------------------------------------------------------------*/
void
my_radio_off_to_rx()
{
  /* Disable and clear all interrupts */
  NVIC_DisableIRQ(RADIO_IRQn);
  nrf_radio_int_disable(0xFFFFFFFF);
  NVIC_ClearPendingIRQ(RADIO_IRQn);

  /* Enable scheduled RX */
  NRF_PPI->CHENSET = (1UL << RADIO_RXEN_CH);
  /* Disable TX scheduling shortcut */
  NRF_PPI->CHENCLR = (1UL << RADIO_TXEN_CH);

  NRF_TIMERX->EVENTS_COMPARE[SCHEDULE_REG] = 0UL;

  /* Clear event register */
  NRF_RADIO->EVENTS_DISABLED = 0UL;

  /* Disable radio */
  NRF_RADIO->TASKS_DISABLE = 1UL;
  NRF_RADIO->EVENTS_END = 0UL;
  NRF_RADIO->EVENTS_ADDRESS = 0UL;
  NRF_RADIO->EVENTS_PAYLOAD = 0UL;
  NRF_RADIO->EVENTS_READY = 0UL;
  NRF_RADIO->EVENTS_FRAMESTART = 0UL;
  NRF_RADIO->EVENTS_PHYEND = 0UL;

#if OSF_DEBUG_GPIO
#if RADIO_TXEN_PIN
  /* Enable RXen debug pin PPI */
  NRF_PPI->CHENSET = (1UL << RADIO_T0_RX_EVENT_PPI_CH);
#endif
#if RADIO_RXEN_PIN
  /* Disable TXen debug pin PPI */
  NRF_PPI->CHENCLR = (1UL << RADIO_T0_TX_EVENT_PPI_CH);
#endif
#endif /* OSF_DEBUG_GPIO */

  NRF_RADIO->EVENTS_DISABLED = 0UL;
}

/*---------------------------------------------------------------------------*/
void
my_radio_set_maxlen(uint8_t maxlen)
{
  NRF_RADIO->PCNF1 = (NRF_RADIO->PCNF1 & ~(RADIO_PCNF1_MAXLEN_Msk)) | \
    ((maxlen) << (RADIO_PCNF1_MAXLEN_Pos));
}

/*---------------------------------------------------------------------------*/
static void
print_radio_config()
{
  /* Print configuration info and timing constants for verification */
  uint32_t mcuvar = NRF_FICR->INFO.VARIANT;
  uint8_t var[4] = { 0, 0, 0, 0 };
  memcpy(var, &mcuvar, sizeof(uint32_t));
  LOG_DBG("OSF Radio config... (DEBUG)\n");
  LOG_DBG("MCU                        : nRF%lX, MCU.VARIANT : %c%c%c%c\n", NRF_FICR->INFO.PART, var[3], var[2], var[1], var[0]);
  LOG_DBG("MCU.PACKAGE                : 0x%lX\n", NRF_FICR->INFO.PACKAGE);
  LOG_DBG("MCU.RAM : 0x%lX, MCU.FLASH : 0x%lX\n", NRF_FICR->INFO.RAM, NRF_FICR->INFO.FLASH);

  LOG_DBG("NRF_POWER->MAINREGSTATUS   : 0x%08lX\n", NRF_POWER->MAINREGSTATUS);
  LOG_DBG("NRF_POWER->RESETREAS       : 0x%08lX\n", NRF_POWER->RESETREAS);
  if(NRF_POWER->RESETREAS & 0x00000001) {
    LOG_WARN("- Reset from pin-reset detected !\n");
  }
  if(NRF_POWER->RESETREAS & 0x00000002) {
    LOG_ERR("- Reset from watchdog detected !\n");
  }
  if(NRF_POWER->RESETREAS & 0x00000004) {
    LOG_ERR("- Reset from soft reset detected !\n");
  }
  if(NRF_POWER->RESETREAS & 0x00000008) {
    LOG_ERR("- Reset from CPU lock-up detected !\n");
  }
  NRF_POWER->RESETREAS = 0xFFFFFFFF;
  LOG_DBG("NRF_POWER->POFCON       : 0x%08lX\n", NRF_POWER->POFCON);
  LOG_DBG("NRF_POWER->USBREGSTATUS : 0x%08lX\n", NRF_POWER->USBREGSTATUS);

  LOG_DBG("NRF_CLOCK->LFCLKSRC     : 0x%08lX\n", NRF_CLOCK->LFCLKSRC);
  LOG_DBG("NRF_CLOCK->LFCLKSTAT    : 0x%08lX\n", NRF_CLOCK->LFCLKSTAT);
  LOG_DBG("NRF_CLOCK->HFCLKSTAT    : 0x%08lX\n", NRF_CLOCK->HFCLKSTAT);
  LOG_DBG("NRF_CLOCK->LFRCMODE     : 0x%08lX\n", NRF_CLOCK->LFRCMODE);

  LOG_DBG("NRF_WDT->RUNSTATUS      : 0x%08lX\n", NRF_WDT->RUNSTATUS);
  LOG_DBG("NRF_WDT->CRV            : 0x%08lX\n", NRF_WDT->CRV);
  LOG_DBG("NRF_WDT->CONFIG         : 0x%08lX\n", NRF_WDT->CONFIG);
  LOG_DBG("NRF_WDT->RREN           : 0x%08lX\n", NRF_WDT->RREN);

  LOG_DBG("NRF_RADIO->MODE         : 0x%08lX\n", NRF_RADIO->MODE);
  LOG_DBG("NRF_RADIO->MODECNF0     : 0x%08lX\n", NRF_RADIO->MODECNF0);
  LOG_DBG("NRF_RADIO->SHORTS       : 0x%08lX\n", NRF_RADIO->SHORTS);
  LOG_DBG("NRF_RADIO->TXPOWER      : 0x%08lX\n", NRF_RADIO->TXPOWER);

  LOG_DBG("NRF_RADIO->PCNF0        : 0x%08lX\n", NRF_RADIO->PCNF0);
  LOG_DBG("NRF_RADIO->PCNF1        : 0x%08lX\n", NRF_RADIO->PCNF1);

  LOG_DBG("NRF_RADIO->CRCCNF       : 0x%08lX\n", NRF_RADIO->CRCCNF);
  LOG_DBG("NRF_RADIO->CRCPOLY      : 0x%08lX\n", NRF_RADIO->CRCPOLY);
  LOG_DBG("NRF_RADIO->CRCINIT      : 0x%08lX\n", NRF_RADIO->CRCINIT);

  LOG_DBG("NRF_RADIO->BASE0        : 0x%08lX\n", NRF_RADIO->BASE0);
  LOG_DBG("NRF_RADIO->PREFIX0      : 0x%08lX\n", NRF_RADIO->PREFIX0);
  LOG_DBG("NRF_RADIO->BASE1        : 0x%08lX\n", NRF_RADIO->BASE1);
  LOG_DBG("NRF_RADIO->PREFIX1      : 0x%08lX\n", NRF_RADIO->PREFIX1);
  LOG_DBG("NRF_RADIO->BASE0        : 0x%08lX\n", NRF_RADIO->BASE0);
  LOG_DBG("NRF_RADIO->TXADDRESS    : 0x%08lX\n", NRF_RADIO->TXADDRESS);
  LOG_DBG("NRF_RADIO->RXADDRESSES  : 0x%08lX\n", NRF_RADIO->RXADDRESSES);
  LOG_DBG("NRF_RADIO->SFD          : 0x%08lX\n", NRF_RADIO->SFD);
  LOG_DBG("NRF_RADIO->TIFS         : 0x%08lX\n", NRF_RADIO->TIFS);
  LOG_DBG("NRF_RADIO->INTENSET     : 0x%08lX\n", NRF_RADIO->INTENSET);

  LOG_DBG("PPI config              : 0x%08lX\n", NRF_PPI->CHEN);

  /* Errata registers. */
  LOG_DBG("0x4000173C              : 0x%08lX\n", *(volatile uint32_t *)0x4000173C);
  LOG_DBG("0x40001740              : 0x%08lX\n", *(volatile uint32_t *)0x40001740);
}
