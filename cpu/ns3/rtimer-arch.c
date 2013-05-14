/**
 * \file
 *         ns-3 implementation for the Contiki real-time module rt
 * \author
 *         Taqi Ad-Din <tkennouc@inria.fr>
 */

#include <signal.h>
#include <sys/time.h>
#include <stddef.h>

#include "sys/rtimer.h"
#include "sys/clock.h"

#include "ipclayer.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
static void
interrupt(int sig)
{
  signal(sig, interrupt);
  rtimer_run_next();
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  signal(SIGALRM, interrupt);
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  rtimer_clock_t c;

  c = t - (unsigned short)clock_time();

  PRINTF("rtimer_arch_schedule time %u %u in %d.%d seconds\n", t, c, c / 1000,
	 (c % 1000) * 1000);

  // Convert time to nanoseconds
  ipc_settimer((uint64_t)c, 1);
}
/*---------------------------------------------------------------------------*/
