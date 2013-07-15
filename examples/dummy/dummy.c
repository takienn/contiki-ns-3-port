/**
 * \file
 *         A very dummy Contiki application do absolutly nothing
 * \author
 *         Taqi Ad-Din <takai.kenn@gmail.com>
 */

#include "contiki.h"
#include<stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(dummy_process, "The Dummy Process");
AUTOSTART_PROCESSES(&dummy_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dummy_process, ev, data)
{
	printf("missing arguments!\n");

  PROCESS_BEGIN();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
