#include "dev/nsradio.h"
#include <stdio.h>

#define DEBUG 0

/* IPC Operations */
size_t ipc_read(void *buf);
void ipc_write(uint8_t *buf, size_t len);

static FILE *file_send, *file_receive;
static int counter_send, counter_receive;

//#include <stdio.h>
//#include <inttypes.h>


/* Contiki Process functions */

PROCESS(nsradio_process, "IPC Radio");

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nsradio_process, ev, data)
{

	counter_send = 0;
	counter_receive = 0;

  PROCESS_POLLHANDLER(pollhandler());
  PROCESS_BEGIN();

  process_poll(&nsradio_process);

  PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_EXIT);


  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

static void
pollhandler(void)
{
  process_poll(&nsradio_process);

  packetbuf_clear();

  int len;
  /* Check if there is incoming data via the network stack */
  len = radio_read(packetbuf_dataptr(), PACKETBUF_SIZE);
  if (len > 0) {
    packetbuf_set_datalen(len);
    NETSTACK_RDC.input();
  }
}

/*---------------------------------------------------------------------------*/

#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",lladdr->addr[0], lladdr->addr[1], lladdr->addr[2], lladdr->addr[3],lladdr->addr[4], lladdr->addr[5])

#define PING6_NB 5
#define PING6_DATALEN 16

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])



/* Radio functions */

/*---------------------------------------------------------------------------*/
static int
init(void)
{
  process_start(&nsradio_process, NULL);
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
prepare(const void *payload, unsigned short payload_len)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
transmit(unsigned short transmit_len)
{
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
radio_send(const void *payload, unsigned short payload_len)
{
  prepare(payload, payload_len);
  /*  Write to ipc memory using payload/payload_len.
   *  In PHY Overlay mode, payload contains the compressed IPv6 header and the payload. 
   *  In MACPHY Overlay mode, payload contains the layer 3/4 headers and the payload */

//  puts("radio_send");
//  fflush(stdout);
//  fwrite(payload, 1 ,payload_len, stdout);
//  puts("\n");
//  fflush (stdout);
  static uint8_t tmpbuf[65534];
  memcpy(tmpbuf,payload, payload_len);
  ipc_write(tmpbuf, (size_t)payload_len);

  counter_send++;

  file_send = fopen("radio_send.log", "a");
  	if (file_send == NULL)
  		perror("fopen error");

  fprintf(file_send, "contiki %d sent packet of size %d\n", getpid(), payload_len);

  fclose(file_send);


  return transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
static int
radio_read(void *buf, unsigned short buf_len)
{
  static char newbuf[65535];
   /*  Read from IPC  */
//TODO: error handling and size checks.
	buf_len = (unsigned short)ipc_read(newbuf);
	memcpy(buf, newbuf, buf_len);

  //if (ret > 0 && ret <= buf_len)
    //memcpy(buf, newbuf, buf_len);
	counter_receive++;

	if(buf_len > 0)
	{
  	file_receive = fopen("radio_receive.log", "a");
  	if(file_receive == NULL)
  		perror("fopen error");

	fprintf(file_receive, "node %d received packet of size %d\n", getpid(),  buf_len);

	fclose(file_receive);
	}

  return buf_len;
}
/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
on(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
off(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
const struct radio_driver nsradio_driver =
  {
    init,
    prepare,
    transmit,
    radio_send,
    radio_read,
    channel_clear,
    receiving_packet,
    pending_packet,
    on,
    off,
  };
/*---------------------------------------------------------------------------*/
