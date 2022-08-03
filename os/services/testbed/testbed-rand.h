/**
 * \file
 *         Testbed random number generator.
 * \author
 *         Michael Baddeley <michael.g.baddeley@gmail.com> *
 */


#ifndef TB_RAND_H_
#define TB_RAND_H_

#include "lib/random.h"

extern uint8_t  tb_rand_buf[];
extern uint8_t  tb_rand_buf_index;
extern uint32_t tb_rand_seed;

#define TB_RAND_BUF_MAX            255

#define TB_RAND(var, len)          do { tb_rand_seed = (tb_rand_seed * 1103515245 + 12345) & UINT32_MAX; var = (tb_rand_seed % len); } while(0)
#define TB_RAND_QUICK(var)         do { var = tb_rand_buf[tb_rand_buf_index++]; tb_rand_buf_index = tb_rand_buf_index % 255; } while(0)
#define TB_RAND_FILL_BUF(len)      do { uint32_t i; for (i = 0; i < len; i++) {TB_RAND(tb_rand_buf[i], TB_RAND_BUF_MAX);}  } while(0)

void tb_rand_init(uint32_t seed);

#endif /* TB_RAND_H */
