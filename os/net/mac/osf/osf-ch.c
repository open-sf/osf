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
 *         OSF channel hopping.
 * \author
 *         Michael Baddeley <michael.baddeley@tii.ae>
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */
 
#include "contiki.h"
#include "contiki-net.h"
#include "net/mac/osf/osf.h"
#include "net/mac/osf/osf-ch.h"

#include "sys/log.h"
#define LOG_MODULE "OSF-CH"
#define LOG_LEVEL LOG_LEVEL_DBG

#define UNUSED(x) (void)(x)

#if OSF_HOPPING == OSF_CH_SEEDED
#define OSF_RAND_BUF_MAX           255
#define OSF_RAND(var, mod)         do { rand_seed = (rand_seed * 1103515245 + 12345) & UINT32_MAX; var = (rand_seed % mod); } while(0)
#define OSF_RAND_PSEUDO(seed, var) do { seed = (seed * 1103515245 + 12345) & UINT32_MAX; var = (seed % OSF_RAND_BUF_MAX); } while(0)
#define OSF_RAND_QUICK(var)        do { var = rand_buf[rand_buf_index++]; rand_buf_index = rand_buf_index % OSF_RAND_BUF_MAX; } while(0)
#define OSF_FILL_RAND_BUF()        do { uint32_t i; for (i = 0; i < OSF_RAND_BUF_MAX; i++) {OSF_RAND(rand_buf[i], OSF_RAND_BUF_MAX);}  } while(0)

static uint8_t  rand_buf[OSF_RAND_BUF_MAX];
static uint8_t  rand_buf_index;
static uint32_t rand_seed;
#endif

#if OSF_HOPPING == OSF_CH_NONE
uint8_t scan_channels[1]                       = {CH_DEFAULT};
uint8_t osf_channels[1]                        = {CH_DEFAULT};
#elif OSF_HOPPING == OSF_CH_SCAN
uint8_t scan_channels[CH_SCAN_LIST_LEN]        = CH_SCAN_LIST;
uint8_t osf_channels[CH_SCAN_LIST_LEN]         = CH_SCAN_LIST;
#elif OSF_HOPPING == OSF_CH_SEEDED
static uint8_t all_channels[CH_ALL_LIST_LEN]   = CH_ALL_LIST;
static uint8_t rand_channels[CH_RAND_LIST_LEN] = {0};
uint8_t scan_channels[CH_SCAN_LIST_LEN]        = CH_SCAN_LIST;
uint8_t osf_channels[CH_LIST_MAX_LEN]          = {0};
#else
#error "ERROR: Unknown CH pattern"
#endif

uint8_t osf_ch_index = 0;
uint8_t osf_scan_index = 0;

uint8_t osf_ch_len = (sizeof(osf_channels) / sizeof(osf_channels[0]));
uint8_t osf_scan_len = (sizeof(scan_channels) / sizeof(scan_channels[0]));

static void
print_ch_list(char *name, uint8_t *list, uint8_t len)
{
  int i;
  LOG_INFO("%-6s %2u [", name, len);
  for (i = 0; i < len; i++) {
    LOG_INFO_("%2u,", list[i]);
  }
  LOG_INFO_("]\n");
}

#if OSF_HOPPING == OSF_CH_SEEDED
/*---------------------------------------------------------------------------*/
static void
rand_init(uint32_t seed)
{
  rand_seed = seed;
}

/*---------------------------------------------------------------------------*/
void
osf_ch_shuffle_channels(uint16_t epoch)
{
  uint8_t i = 1;
  uint8_t ch;
  /* Set the seed as the epoch */
  rand_init(epoch);
  /* Fill every second element of the array with random channels */
  while(i < osf_ch_len) {
    OSF_RAND(ch, CH_RAND_LIST_LEN);
    osf_channels[i] = rand_channels[ch];
    i = i + 2;
  }
}

/*---------------------------------------------------------------------------*/
void
osf_ch_space_channels() {
  uint8_t i = 1;
  uint8_t j = 0;
  while(i < osf_ch_len) {
    osf_channels[i] = rand_channels[j];
    j = (j + 5) % CH_RAND_LIST_LEN;
    i = i + 2;
  }
}
#endif /* OSF_HOPPING == OSF_CH_SEEDED */

/*---------------------------------------------------------------------------*/
void
osf_ch_init()
{
  osf_scan_index = 0;
  osf_ch_index = 0;

  LOG_INFO("Channel hopping init...\n");

#if OSF_HOPPING == OSF_CH_SEEDED
  memset(&rand_buf, 0, OSF_RAND_BUF_MAX);
  rand_buf_index = 0;
  // fill rand channels with all channels minus scan channels
  uint8_t i, j;
  uint8_t rand_index = 0;
  for(i = 0; i < CH_ALL_LIST_LEN; i++) {
    uint8_t is_scan_ch = 0;
    // check to see if this channel is a scan channel
    for(j = 0; j < osf_scan_len; j++) {
      if(all_channels[i] == scan_channels[j]) {
        is_scan_ch = 1;
      }
    }
    // if is not a scan channel, then add it to our list of random channels
    if(!is_scan_ch) {
      rand_channels[rand_index] = all_channels[i];
      rand_index++;
    }
  }

  // seed odd osf_channels with seeded scanning channels
  i = 0;
  j = 0;
  while(i < osf_ch_len) {
    osf_channels[i] = scan_channels[(j++) % osf_scan_len];
    i = i + 2;
  }
  // fill non-seeded osf_channels wth random channels
  osf_ch_shuffle_channels(1234);
  osf_ch_space_channels();
  print_ch_list("- ALL", all_channels, CH_ALL_LIST_LEN);
  print_ch_list("- RAND", rand_channels, CH_RAND_LIST_LEN);
#endif /* OSF_HOPPING == OSF_CH_SEEDED */
  print_ch_list("- SCAN", scan_channels, osf_scan_len);
  print_ch_list("- OSF", osf_channels, osf_ch_len);
}

/*---------------------------------------------------------------------------*/
void
osf_ch_init_scan_index()
{
#if OSF_HOPPING != OSF_CH_NONE
  osf_scan_index = 0;
#endif
}

/*---------------------------------------------------------------------------*/
void
osf_ch_init_index(uint16_t seed)
{
#if OSF_HOPPING == OSF_CH_SEEDED
  /* Get a 'random' channel index */
  rand_init(seed);
  OSF_RAND(osf_ch_index, osf_ch_len);
#else
  osf_ch_index = 0;
#endif
}
