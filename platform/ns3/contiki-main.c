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

#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((u8_t *)addr)[0], ((u8_t *)addr)[1], ((u8_t *)addr)[2], ((u8_t *)addr)[3], ((u8_t *)addr)[4], ((u8_t *)addr)[5], ((u8_t *)addr)[6], ((u8_t *)addr)[7], ((u8_t *)addr)[8], ((u8_t *)addr)[9], ((u8_t *)addr)[10], ((u8_t *)addr)[11], ((u8_t *)addr)[12], ((u8_t *)addr)[13], ((u8_t *)addr)[14], ((u8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ",lladdr.u8[0], lladdr.u8[1], lladdr.u8[2], lladdr.u8[3],lladdr.u8[4], lladdr.u8[5], lladdr.u8[6], lladdr.u8[7])

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

sem_t *sem_go, *sem_done;

PROCINIT(&etimer_process, &tcpip_process);
SENSORS(&socket_sensor);

int done_val = 0;

u8_t convert(const char c, uint8_t lower) {
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
void assign_rimeaddr(const char * addr) {
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

void log_message(char *m1, char *m2) {
	printf("%s%s\n", m1, m2);
}

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
//
//
//
//}

/*---------------------------------------------------------------------------*/
int ContikiMain(char *node_id, int mode, char *addr, char *app) {

	char sem_go_name[128] = "/ns_contiki_sem_go_";
	char sem_done_name[128] = "/ns_contiki_sem_done_";

	if ((sem_go = sem_open(sem_go_name, 0)) == SEM_FAILED )
			perror("contiki sem_open(sem_go) failed");

	if((sem_done = sem_open(sem_done_name, 0)) == SEM_FAILED)
			perror("contiki sem_open(sem_done) failed");



	printf("Contiki %d is on\n", getpid());
	ipc_init(node_id);

	/* Prepare process list */
	process_init();
	/* Prepares a list of timers; starts the ctimer process */
	ctimer_init();
	/* Run configured Radio, RDC, MAC and Network init functions */
	queuebuf_init();
	netstack_init();
	/* Start all processes listed in PROCINIT macro */
	procinit_init();
	/* Run Autostart processes (Application Layer) */
	autostart_start(autostart_processes);
	dlloader_load(app, NULL);
	while (1) {

		sem_post(sem_done);
		sem_wait(sem_go);
		puts("contiki new cycle");

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
		puts("missing arguments!\n");
		exit(0);
	}
	ContikiMain(argv[1], atoi(argv[2]), argv[3]);

	return 0;

}

