/**
 * \file
 *         Testbed random number generator.
 * \author
 *         Michael Baddeley <michael.g.baddeley@gmail.com> *
 */

#include <contiki.h>
#include "lib/random.h"
#include "testbed-rand.h"

#include "sys/log.h"
#define LOG_MODULE "TESTBED"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Software rand */
uint8_t  tb_rand_buf[TB_RAND_BUF_MAX];
uint8_t  tb_rand_buf_index = 0;
uint32_t tb_rand_seed;

void
tb_rand_init(uint32_t seed)
{
  tb_rand_seed = seed;
  TB_RAND_FILL_BUF(TB_RAND_BUF_MAX);
}
