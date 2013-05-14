/**
 * \file
 *         Clock implementation for NS-3.
 * \author
 *         Taqi Ad-Din Kennouche  <tkennouc@zimbra.inria.fr>
 */

#include "sys/clock.h"

uint64_t ipc_time(void);
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
	//xxx ipc_time() returns milliseconds
	uint64_t now = ipc_time();
	return (clock_time_t) now;
}
/*---------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
	return (unsigned long) clock_time() / 1000;
}
/*---------------------------------------------------------------------------*/
void
clock_delay(unsigned int d)
{
	/* Does not do anything. */
}
/*---------------------------------------------------------------------------*/
