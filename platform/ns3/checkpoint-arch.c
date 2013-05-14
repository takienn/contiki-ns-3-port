/**
 * \file
 *  Checkpoint library implementation for the Tmote Sky platform.
 *
 * \author
 */

#include "contiki.h"

static void
checkpoint_thread_loop(void *data)
{
}
/*---------------------------------------------------------------------------*/
int
checkpoint_arch_size()
{
  return 10258;
}
/*---------------------------------------------------------------------------*/
void
checkpoint_arch_checkpoint(int fd)
{
}
/*---------------------------------------------------------------------------*/
void
checkpoint_arch_rollback(int fd)
{
}
/*---------------------------------------------------------------------------*/
checkpoint_arch_init(void)
{
}
/*---------------------------------------------------------------------------*/
static void
serial_interrupt_checkpoint()
{
}
/*---------------------------------------------------------------------------*/
static void
serial_interrupt_rollback()
{
}
/*---------------------------------------------------------------------------*/
static void
serial_interrupt_metrics()
{
}

static int
serial_input_byte_intercept(unsigned char c)
{
}
/*---------------------------------------------------------------------------*/
static void
handle_get_command(void)
{
}
/*---------------------------------------------------------------------------*/
static int
hex_decode_char(char c)
{
}