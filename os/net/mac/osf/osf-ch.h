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

#ifndef OSF_HOPPING_H_
#define OSF_HOPPING_H_

#include "net/mac/osf/osf.h"

#define  OSF_CH_NONE            0
#define  OSF_CH_SEEDED          1
#define  OSF_CH_SCAN            2

#ifdef OSF_CONF_HOPPING
#define OSF_HOPPING             OSF_CONF_HOPPING
#else
#define OSF_HOPPING             OSF_CH_SEEDED
#endif

#define CH_DEFAULT               80
#define CH_SCAN_LIST           {  2, 26, 80 }
#define CH_SCAN_LIST_LEN          3
#define CH_ALL_LIST            {  5, 10, 15, 20, 25, 30, 35, 40, 45, 50, \
                                 55, 60, 65, 70, 75, 80 };
#define CH_ALL_LIST_LEN          16

#if CH_ALL_LIST_LEN != CH_SCAN_LIST_LEN
#define CH_RAND_LIST_LEN        (CH_ALL_LIST_LEN - CH_SCAN_LIST_LEN)
#else
#define CH_RAND_LIST_LEN        CH_ALL_LIST_LEN
#endif

#define CH_LIST_MAX_LEN          5

extern uint8_t osf_channels[];
extern uint8_t scan_channels[];
extern uint8_t osf_ch_len;
extern uint8_t osf_scan_len;
extern uint8_t osf_ch_index;
extern uint8_t osf_scan_index;

/*---------------------------------------------------------------------------*/
void osf_ch_init(void);
void osf_ch_init_scan_index();
void osf_ch_init_index(uint16_t epoch);

#endif /* OSF_HOPPING_H_ */
