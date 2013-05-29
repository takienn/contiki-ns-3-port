#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ipclayer.h>
#include <stdio.h>

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef UIP_BUFSIZE
#define UIP_BUFSIZE 1300
#endif

uint64_t now;
uint64_t old = 0;

size_t traffic_size = UIP_BUFSIZE, size_time = 8;
size_t sizeof_size_t = sizeof(size_t); // size of size_t

// Shared memory fds
int shm_in, shm_out, shm_time, shm_timer;

// Pointers to shared memories
uint8_t *addr_in, *addr_out, *addr_time, *addr_timer;

// Shared memory names
char m_traffic_in[128] = "/ns_contiki_traffic_out_";
char m_traffic_out[128] = "/ns_contiki_traffic_in_";
char m_traffic_time[128] = "/ns_contiki_traffic_time_";
char m_traffic_timer[128] = "/ns_contiki_traffic_timer_";

//Semaphore objects
sem_t *sem_in, *sem_out, *sem_time, *sem_timer;
sem_t *sem_timer_go, *sem_traffic_go;
sem_t *sem_timer_done, *sem_traffic_done;

//Semaphores names
char m_sem_in[128] = "/ns_contiki_sem_out_";
char m_sem_out[128] = "/ns_contiki_sem_in_";
char m_sem_time[128] = "/ns_contiki_sem_time_"; // time variables are shared among processes i.e same names for all
char m_sem_timer[128] = "/ns_contiki_sem_timer_"; // timer variables are shared among processes i.e same names for all
char sem_traffic_go_name[128] = "/ns_contiki_sem_traffic_go_";
char sem_traffic_done_name[128] = "/ns_contiki_sem_traffic_done_";
char sem_timer_go_name[128] = "/ns_contiki_sem_timer_go_";
char sem_timer_done_name[128] = "/ns_contiki_sem_timer_done_";

/* Intialization of Shared Memory Pointers and Semaphores
 */
void ipc_init(char *id) {

	/*XXX appending ns-3's node id to shm/sem names
	 time/timers are exceptions, one shm/sem for each
	 is shared among all nodes.
	 */

	//strcat(m_sem_time, id);
	//strcat(m_traffic_time, id);
	strcat(m_traffic_in, id);
	strcat(m_traffic_out, id);
	strcat(m_traffic_timer, id);
	strcat(m_sem_in, id);
	strcat(m_sem_out, id);
	strcat(m_sem_timer, id);
	strcat(sem_traffic_go_name, id);
	strcat(sem_traffic_done_name, id);
	strcat(sem_timer_go_name, id);
	strcat(sem_timer_done_name, id);

	/* opening Shared Memory*/

	if ((shm_in = shm_open(m_traffic_in, O_RDWR, 0)) == -1)
		perror("contiki shm_open(shm_in)");

	if ((shm_out = shm_open(m_traffic_out, O_RDWR, 0)) == -1)
		perror("contiki shm_open(shm_out)");

	if ((shm_time = shm_open(m_traffic_time, O_RDWR, 0)) == -1)
		perror("contiki shm_open(shm_time)");

	if ((shm_timer = shm_open(m_traffic_timer, O_RDWR, 0)) == -1)
		perror("contiki shm_open(shm_timer)");

	/* Resizing shared memory to defined size.
	 XXX I extend the size of allocated shm to include
	 information about actual data size being transfered.
	 */
	if (ftruncate(shm_in, sizeof_size_t + traffic_size) == -1)
		perror("contiki ftruncate(shm_in) error");

	if (ftruncate(shm_out, sizeof_size_t + traffic_size) == -1)
		perror("contiki ftruncate(shm_out) error");

	if (ftruncate(shm_time, size_time) == -1)
		perror("contiki ftruncate(shm_time) error");

	if (ftruncate(shm_timer, size_time + 1) == -1)
		perror("contiki ftruncate(shm_timer) error");

	/* Mapping shared memory fds
	 XXX I guess it is ok to assign RDWR permissions to all
	 maybe I need to meditate this ;)
	 */
	addr_in = mmap(NULL, traffic_size + sizeof_size_t, PROT_READ | PROT_WRITE, MAP_SHARED,
			shm_in, 0);
	addr_out = mmap(NULL, traffic_size + sizeof_size_t, PROT_READ | PROT_WRITE, MAP_SHARED,
			shm_out, 0);
	addr_time = mmap(NULL, size_time, PROT_READ | PROT_WRITE, MAP_SHARED,
			shm_time, 0);
	addr_timer = mmap(NULL, size_time + 1, PROT_READ | PROT_WRITE, MAP_SHARED,
			shm_timer, 0);

	/* opening Semaphores */

	if ((sem_in = sem_open(m_sem_in, 0)) == SEM_FAILED )
		perror("contiki sem_open(sem_in) failed");

	if ((sem_out = sem_open(m_sem_out, 0)) == SEM_FAILED )
		perror("contiki sem_open(sem_out) failed");

	if ((sem_time = sem_open(m_sem_time, 0)) == SEM_FAILED )
		perror("contiki sem_open(sem_time) failed");

	if ((sem_timer = sem_open(m_sem_timer, 0)) == SEM_FAILED )
		perror("contiki sem_open(sem_timer) failed");

/////// Semaphores to make sure timers are handled by ns3 ////////////

	if ((sem_timer_go = sem_open(sem_timer_go_name, 0)) == SEM_FAILED )
		perror("contiki sem_open(sem_timer_go) failed");
	if ((sem_timer_done = sem_open(sem_timer_done_name, 0)) == SEM_FAILED )
		perror("contiki sem_open(sem_timer_done) failed");

/////// Semaphores to make sure traffic is handled by ns3 ////////////

	if ((sem_traffic_go = sem_open(sem_traffic_go_name, 0)) == SEM_FAILED )
		perror("contiki sem_open(sem_timer_go) failed");
	if ((sem_traffic_done = sem_open(sem_traffic_done_name, 0)) == SEM_FAILED )
		perror("contiki sem_open(sem_timer_done) failed");

}

size_t ipc_read(void *buf) {

	size_t input_size;

	//PRINTF("contiki %d trying to read using semaphore %s\n", getpid(), m_sem_in);

	if (sem_wait(sem_in) == -1)
		perror("contiki sem_wait() error");

	//PRINTF("contiki %d got sem_in\n", getpid());
	// reading input size
	memcpy(&input_size, addr_in, sizeof_size_t);

	// reading input
	memcpy(buf, addr_in + sizeof_size_t, input_size);

	// zeroing all
	memset(addr_in, 0, traffic_size + sizeof_size_t);

	if (sem_post(sem_in) == -1)
		perror("contiki sem_post() error");

	//PRINTF("contiki %d finished read using semaphore %s\n", getpid(), m_sem_in);

	if(input_size>0)
	{
		PRINTF("ipc_read\n");
#if DEBUG
		fwrite(buf,1,input_size,stdout);
		puts("\n");
#endif
		fflush(stdout);
	}

	return input_size;

}

void ipc_write(uint8_t *buf, size_t len) {

	// As reading, this function writes data with size information
	//XXX maybe checking that len <= traffic_size

	if (sem_wait(sem_out) == -1)
		perror("contiki sem_wait() error");

	// writing data size first
	memcpy(addr_out, &len, sizeof_size_t);

	// now writing data of that size
	memcpy(addr_out + sizeof_size_t, buf, len);

	//PRINTF("Contiki wrote packet of size %d\n", len);
#if DEBUG
	puts("ipc_write\n");
	fwrite(addr_out + sizeof_size_t, 1, len, stdout);
	puts("\n");
	fflush(stdout);
#endif

	if (sem_post(sem_out) == -1)
		perror("contiki sem_wait() error");

	//PRINTF("Contiki wrote a packet\n");
	sem_post(sem_traffic_done);
	//PRINTF("Contiki waiting to proceed after writing a packet\n");
	sem_wait(sem_traffic_go);
	//PRINTF("Contiki proceeds after writing a packet\n");
}

uint64_t ipc_time(void) {

	if (sem_wait(sem_time) == -1)
		perror("contiki sem_wait() error");

	memcpy((void *) &now, addr_time, size_time);

	if (sem_post(sem_time) == -1)
		perror("contiki sem_post() error");

	if (now > old) {
		old = now;
		//printf("contiki %d got time %llu\n", getpid(), now);
	}

	return now;
}

void ipc_settimer(uint64_t interval, uint8_t type) {

	// 0 for event timers, 1 for realtime timers
	if (type != 0 && type != 1) {
		puts("Wrong timer type!");
		exit(1);
	}

	if (sem_wait(sem_timer) == -1)
		perror("contiki sem_wait() error");

	memcpy(addr_timer, &type, 1);
	memcpy(addr_timer + 1, &interval, 8);

	if (sem_post(sem_timer) == -1)
		perror("contiki sem_wait() error");

	/* XXX
	 * The idea is to not leave this function until
	 * we are sure ns-3 handled this timer.
	 */

	//PRINTF("Contiki set a timer\n");
	sem_post(sem_timer_done);
	//PRINTF("Contiki waiting to proceed after setting a timer\n");
	sem_wait(sem_timer_go);
	//PRINTF("Contiki proceeds after setting a timer\n");

}

