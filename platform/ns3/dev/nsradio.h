/*! \file nsradio.h
 *  \brief Driver for the virtual radio used to communicate over an IPC shared memory to the ns-3 medium.  */

#ifndef NSRADIO_H
#define NSRADIO_H

#include "contiki.h"	/* Test this */
#include "contiki-net.h"  /* Test this */
#include "dev/radio.h"

/* For shared memory IPC */
#include <sys/types.h>
#include "../ipclayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef MACPHY_OVERLAY
/* Whether MACPHY Overlay mode is used or not (default: no; PHY Overlay) */
#define MACPHY_OVERLAY                 0
#endif 

PROCESS_NAME(nsradio_process);

/*! \fn static void pollhandler(void)
 *  \brief Action to take when the radio is polled.  The shared memory is inspected for incoming data an IO is performed if necessary */
static void pollhandler(void);

/*! \struct radio_driver nsradio_driver
 *  \brief The NS Radio Driver structure. */
extern const struct radio_driver nsradio_driver;

/*! \fn static int init (void)
 *  \brief Starts the nsradio process responsible for ns-3 communication. */
static int init (void);

/*! \fn static int prepare (const void *payload, unsigned short payload_len)
 *  \brief Prepares data to be sent over the radio.  Does nothing. */
static int prepare(const void *payload, unsigned short payload_len);

/*! \fn static int transmit(unsigned short transmit_len)
 *  \brief Transmits data using radio.  Does nothing and returns successful acknowledgement. */
static int transmit(unsigned short transmit_len);

/*! \fn static int radio_send(const void *payload, unsigned short payload_len)  
 *  \brief Prepares and transmits data via the emulated radio.  Writes to shared memory to send data over IPC to ns-3. */
static int radio_send(const void *payload, unsigned short payload_len);

/*! \fn static int radio_read(void *buf, unsigned short buf_len)
 *  \brief Extracts data from the emulated radio.  Invokes a ipcread call to pull from the ns-3 shared memory. */
static int radio_read(void *buf, unsigned short buf_len);

/*! \fn static int channel_clear(void)
 *  \brief Checks if the channel is clear.  Does nothing *nd returns clear. */
static int channel_clear(void);

/*! \fn static int receiving_packet(void)
 *  \brief Checks if the channel is currently receiving a packet.  Does nothing and returns clear. */
static int receiving_packet(void);

/*! \fn static int pending_packet(void)
 *  \brief Checks the virtual radio hardware buffer if a packet is waiting.  Does nothing and returns clear.  */
static int pending_packet(void);

/*! \fn static int on(void)
 *  \brief Turns the virtual radio hardware on.  Does nothing and returns a successful acknowledgement.  */
static int on(void);

/*! \fn static int off(void)
 *  \brief Turns the virtual radio hardware off.  Does nothing and returns a seccessful acknowledgement.  */
static int off(void);

#endif /* NSRADIO_H */
