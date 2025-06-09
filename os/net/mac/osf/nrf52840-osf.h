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
#if CONTIKI_TARGET_NRF52840

#ifndef _NRF_RADIO_DRIVER_H_
#define _NRF_RADIO_DRIVER_H_

#include "contiki.h"
#include "nrf_radio.h"
#include "net/mac/osf/osf-timer.h"
#include "nrf52840-ieee.h"

/*---------------------------------------------------------------------------*/
/* HAL split */
/*---------------------------------------------------------------------------*/
#define OSF_RADIO_IRQ_REGISTER_HANDLER nrf52840_radioirq_register_handler
#define OSF_RADIO_HAL_START nrf52840_hal_start
#define OSF_RADIO_HAL_STOP nrf52840_hal_stop

void nrf52840_hal_start();
void nrf52840_hal_stop();

#define TS_LED                          LED_1
#define ROUND_LED                       LED_2
#define SYNCED_LED                      LED_3
#define CRCERR_LED                      LED_4

/*---------------------------------------------------------------------------*/
/* TX POWER */
/*---------------------------------------------------------------------------*/
#define UNSUPPORTED_TX_POWER        (50UL)

#if (defined(NRF52840_XXAA) || defined(NRF52833_XXAA))
#define Pos8dBm                     RADIO_TXPOWER_TXPOWER_Pos8dBm
#define Pos7dBm                     RADIO_TXPOWER_TXPOWER_Pos7dBm
#define Pos6dBm                     RADIO_TXPOWER_TXPOWER_Pos6dBm
#define Pos5dBm                     RADIO_TXPOWER_TXPOWER_Pos5dBm
#define Pos2dBm                     RADIO_TXPOWER_TXPOWER_Pos2dBm
#else 
#define Pos8dBm                     (UNSUPPORTED_TX_POWER+8UL)
#define Pos7dBm                     (UNSUPPORTED_TX_POWER+7UL)
#define Pos6dBm                     (UNSUPPORTED_TX_POWER+6UL)
#define Pos5dBm                     (UNSUPPORTED_TX_POWER+5UL)
#define Pos2dBm                     (UNSUPPORTED_TX_POWER+2UL)
#endif
#define Pos4dBm                     RADIO_TXPOWER_TXPOWER_Pos4dBm
#define Pos3dBm                     RADIO_TXPOWER_TXPOWER_Pos3dBm
#define ZerodBm                     RADIO_TXPOWER_TXPOWER_0dBm
#define Neg4dBm                     RADIO_TXPOWER_TXPOWER_Neg4dBm
#define Neg8dBm                     RADIO_TXPOWER_TXPOWER_Neg8dBm
#define Neg12dBm                    RADIO_TXPOWER_TXPOWER_Neg12dBm
#define Neg16dBm                    RADIO_TXPOWER_TXPOWER_Neg16dBm
#define Neg20dBm                    RADIO_TXPOWER_TXPOWER_Neg20dBm
#define Neg30dBm                    RADIO_TXPOWER_TXPOWER_Neg30dBm /* deprecated ! */
#define Neg40dBm                    RADIO_TXPOWER_TXPOWER_Neg40dBm

#define OSF_TXPOWER_TO_STR(P) \
  ((P == Pos8dBm)  ? ("+8dBm") : \
   (P == Pos7dBm)  ? ("+7dBm") : \
   (P == Pos6dBm)  ? ("+6dBm") : \
   (P == Pos5dBm)  ? ("+5dBm") : \
   (P == Pos4dBm)  ? ("+4dBm") : \
   (P == Pos3dBm)  ? ("+3dBm") : \
   (P == Pos2dBm)  ? ("+2dBm") : \
   (P == ZerodBm)  ? ("0dBm")  : \
   (P == Neg4dBm)  ? ("-4dBm") : \
   (P == Neg8dBm)  ? ("-8dBm") : \
   (P == Neg12dBm) ? ("-12dBm") : \
   (P == Neg30dBm) ? ("-30dBm") : \
   (P == Neg16dBm) ? ("-16dBm") : \
   (P == Neg20dBm) ? ("-20dBm") : \
   (P == Neg40dBm) ? ("-40dBm") : ("???"))

/*---------------------------------------------------------------------------*/
/* RSSI */
/*---------------------------------------------------------------------------*/
#define my_radio_rssi() ((int8_t)(-(int8_t)(NRF_RADIO->RSSISAMPLE)))

/*---------------------------------------------------------------------------*/
/* PHY */
/*---------------------------------------------------------------------------*/
#define PHY_NRF_UNSUPPORTED        (100UL)
#define PHY_NRF_1M                RADIO_MODE_MODE_Nrf_1Mbit          // 0
#define PHY_NRF_2M                RADIO_MODE_MODE_Nrf_2Mbit          // 1
#define PHY_BLE_1M                RADIO_MODE_MODE_Ble_1Mbit          // 3
#define PHY_BLE_2M                RADIO_MODE_MODE_Ble_2Mbit          // 4

#if NRF52840_WITH_CODED_PHY
#define PHY_BLE_500K              RADIO_MODE_MODE_Ble_LR500Kbit      // 5
#define PHY_BLE_125K              RADIO_MODE_MODE_Ble_LR125Kbit      // 6
#else
#define PHY_BLE_500K              (PHY_NRF_UNSUPPORTED+5UL)
#define PHY_BLE_125K              (PHY_NRF_UNSUPPORTED+6UL)
#endif
#if NRF52840_WITH_IEEE_PHY
#define PHY_IEEE                  RADIO_MODE_MODE_Ieee802154_250Kbit // 15
#else

#define PHY_IEEE                  (PHY_NRF_UNSUPPORTED+15UL)
#endif

#define OSF_PHY_TO_STR(P) \
  ((P == PHY_NRF_1M) ? ("NRF_1M") : \
   (P == PHY_NRF_2M) ? ("NRF_2M") : \
   (P == PHY_BLE_1M) ? ("1M") : \
   (P == PHY_BLE_2M) ? ("2M") : \
   (P == PHY_BLE_500K) ? ("500K") : \
   (P == PHY_BLE_125K) ? ("125K") : \
   (P == PHY_IEEE) ? ("IEEE") : ("???"))


 typedef struct osf_phy_conf {
   char*                    name;
   /* Packet */
   nrf_radio_mode_t         mode;
   nrf_radio_packet_conf_t *conf;
   uint8_t                  crclen;
   nrf_radio_crc_addr_t     crcaddr;
   uint32_t                 crcpoly;
   uint32_t                 crcinit;
   /* Timings */
   rtimer_clock_t           header_air_ticks;
   rtimer_clock_t           post_addr_air_ticks;
   rtimer_clock_t           payload_air_ticks;
   rtimer_clock_t           footer_air_ticks;
   rtimer_clock_t           tx_rx_addr_offset_ticks;
   rtimer_clock_t           tx_rx_end_offset_ticks;
   rtimer_clock_t           packet_air_ticks;
   rtimer_clock_t           slot_duration;
#if OSF_EXT_ND
   /* Noise Detection */
   int8_t                   noise_threshold;
#endif
 } osf_phy_conf_t;


extern osf_phy_conf_t *osf_phy_conf;
/* OSF PHYs (Basically std w/ statlen)*/
extern osf_phy_conf_t osf_phy_conf_1M;
extern osf_phy_conf_t osf_phy_conf_2M;
extern osf_phy_conf_t osf_phy_conf_500K;
extern osf_phy_conf_t osf_phy_conf_125K;
extern osf_phy_conf_t osf_phy_conf_802154;

/* Custom PHYs */
extern osf_phy_conf_t osf_phy_conf_2M_LF;

/*---------------------------------------------------------------------------*/
#define PHY_BIT_TIME_MULTIPLIER(P) \
  ( \
    (P == PHY_BLE_1M) ? 1 : \
    ((P == PHY_BLE_2M) ? 0.5 : \
     ((P == PHY_BLE_500K) ? 2 : \
      ((P == PHY_BLE_125K) ? 8 : \
       ((P == PHY_IEEE) ? 4 : 0)))) \
  )

#define OSF_PHY_BYTES_TO_USX(B, P)           (uint32_t)( ((B)<<3) * PHY_BIT_TIME_MULTIPLIER(P) )
#define OSF_PHY_BITS_TO_USX(B, P)            (uint32_t)( (B) * PHY_BIT_TIME_MULTIPLIER(P) )
#define OSF_PHY_BYTES_TO_RTIMERTICKSX(B, P)  (rtimer_clock_t)( (RTIMERX_MICROSECOND * ((B)<<3)) * PHY_BIT_TIME_MULTIPLIER(P) )
#define OSF_PHY_BITS_TO_RTIMERTICKSX(B, P)   (rtimer_clock_t)( (RTIMERX_MICROSECOND * B) * PHY_BIT_TIME_MULTIPLIER(P) )

/*---------------------------------------------------------------------------*/
/* Timing constants */
#define RADIO_RAMPUP_TIME                    (647) // SHOULD be 40us + about 2.2us processing time. 40.436us measured

/*---------------------------------------------------------------------------*/
#ifdef OSF_PHY_CONF_WHITENING
#define OSF_PHY_WHITENING OSF_PHY_CONF_WHITENING
#else
#define OSF_PHY_WHITENING RADIO_PCNF1_WHITEEN_Enabled
#endif /* USE_WHITENING */

/*---------------------------------------------------------------------------*/
osf_phy_conf_t* my_radio_get_phy_conf(uint8_t mode);
rtimer_clock_t my_radio_set_phy_airtime(osf_phy_conf_t *phy, uint8_t len, uint8_t statlen);
void my_radio_init(osf_phy_conf_t *phy_conf, void *my_tx_buffer, uint8_t len, uint8_t statlen, uint8_t addr);
void my_radio_set_tx_power(uint8_t p);
void my_radio_rx_hop_channel(uint8_t channel);
uint8_t schedule_rx_abs(uint8_t *buf, uint8_t channel, rtimer_clock_t t_abs);
uint8_t schedule_tx_abs(uint8_t *buf, uint8_t channel, rtimer_clock_t t_abs);
void my_radio_off_completely();
void my_radio_off_to_tx();
void my_radio_off_to_rx();
void my_radio_set_maxlen(uint8_t maxlen);

#endif /* _NRF_RADIO_DRIVER_H_ */

#endif /* CONTIKI_TARGET_NRF52840 */
