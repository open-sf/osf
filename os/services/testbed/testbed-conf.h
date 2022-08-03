/**
 * \file
 *         Testbed configuration file (i.e. D-Cube, NULLTB, ...).
 * \author
 *         Michael Baddeley <michael.g.baddeley@gmail.com> *
 */

#ifndef TESTBED_CONF_H_
#define TESTBED_CONF_H_

#define TB_NULLTB                         1
#define TB_DCUBE                          2

#define TB_TO_STR(TB) \
 ((TB == TB_NULLTB)  ? ("NULLTB") : \
  (TB == TB_DCUBE)   ? ("DCUBE")  : ("Unknown"))

#if CONF_TESTBED
#define TESTBED                           CONF_TESTBED
#else
#error "ERROR: Must set TESTBED through CONF_TESTBED!"
#endif

/* Testbed and platform specific incudles */
#if TESTBED == TB_NULLTB
  #include "services/testbed/nulltb/nulltb.h"
#elif TESTBED == TB_DCUBE
  #if CONTIKI_TARGET_NRF52840
    #include "services/testbed/dcube/nrf52840/dcube-nrf.h"
  #elif CONTIKI_TARGET_SKY
    #include "services/testbed/dcube/sky/dcube-sky.h"
  #else
    #error "ERROR: Unknown testbed target!"
  #endif
#else
  #error "ERROR: Unknown testbed!"
#endif

#endif /* TESTBED_CONF_H_ */
