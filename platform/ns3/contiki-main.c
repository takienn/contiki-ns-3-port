#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>

#include "contiki.h"
#include "net/netstack.h"
#include "net/uip.h"
#include "dev/nsradio.h"
#include "dev/socket-sensor.h"

#include "ipclayer.h"

#include "loader/dlloader.h"
#include "net/rime/rimeaddr.h"
#include "net/rime.h"
#include "net/rime/rime-udp.h"

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7])
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ",lladdr.u8[0], lladdr.u8[1], lladdr.u8[2], lladdr.u8[3],lladdr.u8[4], lladdr.u8[5], lladdr.u8[6], lladdr.u8[7])
#else
#define PRINTF(...)
#define PRINTADDR(addr)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(lladdr)
#endif

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#if UIP_CONF_IPV6
PROCINIT(&etimer_process, &tcpip_process);
#else
PROCINIT(&etimer_process, &tcpip_process, &uip_fw_process);
#endif

SENSORS(&socket_sensor);

int done_val = 0;

u8_t convert(const unsigned char c, uint8_t lower) {
	u8_t result;
	if (c > 0x39) {
		if (lower) {
			result = (c - 0x57);
		} else {
			result = 0x10 * (c - 0x57);
		}
	} else {
		if (lower) {
			result = (c - 0x30);
		} else {
			result = 0x10 * (c - 0x30);
		}
	}
	return result;
}

/*  Sets uip_lladdr value so that uip can derive a full address from the link layer address */
#ifdef UIP_CONF_IPV6
void assign_rimeaddr(const unsigned char * addr) {
	uint8_t i, j = 0;
	union {
		u8_t u8[8];
	} lladdr;

	for (; i < 8; i++) {
		lladdr.u8[i] = 0;
		lladdr.u8[i] += convert(addr[j], 0);
		lladdr.u8[i] += convert(addr[j + 1], 1);
		j += 3;
	}
	rimeaddr_set_node_addr((rimeaddr_t *) &lladdr);
	memcpy(&uip_lladdr.addr, &lladdr, sizeof(uip_lladdr.addr));
	PRINTLLADDR(lladdr);
}
//#else
//void assign_addr(char *ipaddr, char *netmask) {
//
//	uip_ipaddr_t addr;
//
//	uip_gethostaddr(&addr);
//	uiplib_ip6addrconv(ipaddr, &addr);
//	uip_sethostaddr(&addr);
//
//	uip_getnetmask(&addr);
//	uiplib_ip4addrconv(netmask, &addr);
//	uip_setnetmask(&addr);
//}
#endif

void log_message(char *m1, char *m2) {
	printf("%s%s\n", m1, m2);
}

/*---------------------------------------------------------------------------*/
int ContikiMain(char *node_id, int mode, const char *addr, char *app) {

	static int counter = 0;
	static sem_t *sem_go, *sem_done;

	char sem_go_name[128] = "/ns_contiki_sem_go_";
	char sem_done_name[128] = "/ns_contiki_sem_done_";

	if ((sem_go = sem_open(sem_go_name, 0)) == SEM_FAILED )
			perror("contiki sem_open(sem_go) failed");

	if((sem_done = sem_open(sem_done_name, 0)) == SEM_FAILED)
			perror("contiki sem_open(sem_done) failed");



	PRINTF("Contiki %d is on\n", getpid());
	ipc_init(node_id);
	PRINTF("Contiki %d executed ipc_init\n", getpid());

	random_init(atoi(node_id));

	/* Assign node llid */
	printf("Assigning lladdr %s\n", addr);
	assign_rimeaddr(addr);

	/* Prepare process list */
	process_init();
	PRINTF("Contiki %d executed process_init\n", getpid());
	/* Prepares a list of timers; starts the ctimer process */
	ctimer_init();
	PRINTF("Contiki %d executed ctimer_init\n", getpid());
	/* Run configured Radio, RDC, MAC and Network init functions */
	//queuebuf_init();
	netstack_init();
	PRINTF("Contiki %d executed netstack_init\n", getpid());
	/* Start all processes listed in PROCINIT macro */
	procinit_init();
	PRINTF("Contiki %d executed procinit\n", getpid());
	PRINTF("Contiki %d executed dlloader_load\n", getpid());
	/* Run Autostart processes (Application Layer) */
	autostart_start(autostart_processes);
	PRINTF("Contiki %d executed autostart_start\n", getpid());
	dlloader_load(app, NULL);
	while (1) {

		PRINTF("contiki %d before new cycle %d\n",getpid(), counter);
		sem_post(sem_done);
		sem_wait(sem_go);
		PRINTF("contiki %d new cycle %d\n", getpid(), counter);

		/* Run a single cycle of the scheduler (event and poll processing) */
		process_run();
		/* Synchronize etimer with real-time */
		etimer_request_poll();
	}

	return 0;
}
/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[]) {
	if (argc < 4) {
		PRINTF("missing arguments!\n");
		exit(0);
	}
	ContikiMain(argv[1], atoi(argv[2]), argv[3], argv[4]);

	return 0;

}

