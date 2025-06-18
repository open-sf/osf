/*
 * Copyright (c) 2021, technology Innovation Institute (TII).
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
 *         sky66112 pa driver
 * \author
 *         Yevgen Gyl <yevgen.gyl@unikie.com>
 */

/**
 * \addtogroup nrf52840-devices Device drivers
 * @{
 */

#ifndef SKY66112PA_H
#define SKY66112PA_H

#include "dev/pa.h"

/*
 * Default FEM configuration for SKY66112-11 PA
 * These can be overridden in a project or board-specific header before including this file.
 */
#ifndef SKY66112PA_MODE_TX
#define SKY66112PA_MODE_TX    PA_TX_Plus20dBm
#endif

#ifndef SKY66112PA_MODE_RX
#define SKY66112PA_MODE_RX    PA_RX_LNA
#endif

#ifndef SKY66112PA_ANT_RX
#define SKY66112PA_ANT_RX     PA_LNA_ANT1
#endif

#ifndef SKY66112PA_ANT_TX
#define SKY66112PA_ANT_TX     PA_LNA_ANT1
#endif

#ifndef SKY66112PA_TX_ATT
#define SKY66112PA_TX_ATT     0x1F
#endif

extern const struct pa_driver sky66112pa_driver;

#endif /* SKY66112PA_H */
/** @} */
