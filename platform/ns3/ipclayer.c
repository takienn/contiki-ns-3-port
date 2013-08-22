#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ipclayer.h>
#include <stdio.h>

#define DEBUG 1

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#ifndef UIP_BUFSIZE
#define UIP_BUFSIZE 1500
#endif

#ifndef DATA_TYPE
const uint8_t DATA_TYPE = 0;
const uint8_t TIMER_TYPE = 1;
//#define DATA_TYPE 0
//#define TIMER_TYPE 1
#endif

uint64_t now;
uint64_t old = 0;

size_t traffic_size = UIP_BUFSIZE, size_time = 8;
size_t sizeof_size_t = sizeof(size_t); // size of size_t

/* Intialization of Shared Memory Pointers and Semaphores
 */
semaphores_t *sharedSemaphores;
void ipc_init(char *id, semaphores_t *sharedSemaphoresTmp) {
	sharedSemaphores=sharedSemaphoresTmp;
}

size_t ipc_read(void *buf) {

	size_t input_size;

	//PRINTF("contiki %d trying to read using semaphore %s\n", getpid(), m_sem_in);

	PRINTF("-- Wait on  sem_out");// - %d is on\n", (int)sharedSemaphores->sem_out);
	if (sem_wait(&sharedSemaphores->sem_out) == -1)
		perror("contiki sem_wait() error");
	PRINTF("-- Wait on  sem_out");//  - %d is on\n", sharedSemaphores->sem_out);

	//PRINTF("contiki %d got sem_in\n", getpid());
	// reading input size
	memcpy(&input_size, sharedSemaphores->traffic_out, sizeof_size_t);
	PRINTF("-- Read from ns-3:%d ", input_size);//  - %d is on\n", sharedSemaphores->sem_out);

	// reading input
	memcpy(buf, sharedSemaphores->traffic_out + sizeof_size_t, input_size);

	// zeroing all
	memset(sharedSemaphores->traffic_out, 0, traffic_size + sizeof_size_t);

	PRINTF("-- Post  on  sem_out");//  - %d is on\n", sharedSemaphores->sem_out);
	if (sem_post(&sharedSemaphores->sem_out) == -1)
		perror("contiki sem_post() error");
	PRINTF("-- Post  on  sem_out");//  - %d is on\n", sharedSemaphores->sem_out);

	//PRINTF("contiki %d finished read using semaphore %s\n", getpid(), m_sem_in);

	if(input_size>0)
	{
		PRINTF("ipc_read\n");
#if DEBUG
		puts("\n");
		fwrite(buf,1,input_size,stdout);
		puts("\n");
#endif
		fflush(stdout);
	}

	return input_size;

}

void ipc_write(uint8_t *buf, size_t len) {
	uint8_t typeOfInformation= DATA_TYPE;

	// As reading, this function writes data with size information
	//XXX maybe checking that len <= traffic_size

	PRINTF("-- Wait  on  sharedSemaphores->sem_in");//  - %s is on\n", sharedSemaphores->sem_timer);
	if (sem_wait(&sharedSemaphores->sem_in) == -1)
		perror("contiki sem_wait() error");
	PRINTF("-- Wait  on  sharedSemaphores->sem_in");//  - %s is on\n", sharedSemaphores->sem_timer);

	// writing data type and size first
	memcpy(sharedSemaphores->traffic_in, &typeOfInformation, 1);
	memcpy(sharedSemaphores->traffic_in + 1, &len, sizeof_size_t);

	// now writing data of that size
	memcpy(sharedSemaphores->traffic_in + sizeof_size_t + 1, buf, len);

	//PRINTF("Contiki wrote packet of size %d\n", len);
#if DEBUG
	int i=0;
	puts("Full packet\n");
	for(i=0;i<len;i++){
		PRINTF("{%c}", buf[i]);
	}
	PRINTF("\n %d -> IPC \n", (int)len);
	fflush(stdout);
	for(i=0;i<len;i++){
		PRINTF("{%c}", (sharedSemaphores->traffic_in + sizeof_size_t + 1)[i]);
	}
	puts("\n");
	puts("\nipc_write\n");
	fwrite(sharedSemaphores->traffic_in + sizeof_size_t + 1, 1, len, stdout);
	puts("\n");
	fflush(stdout);
#endif

	PRINTF("-- Post  on  sem_in");//  - %s is on\n", sharedSemaphores->sem_in);
	if (sem_post(&sharedSemaphores->sem_traffic_done) == -1)
		perror("contiki sem_wait() error");
	PRINTF("-- Post  on  sem_in");//  - %s is on\n", sharedSemaphores->sem_in);

	// Remove this step could increase performance, but could happen that the contiki
	// machine would be released BEFORE the ns-3 thread have the time to
	// process the message, what could cause problems afterwards with missing
	// messages
	PRINTF("-- Wait  on  sem_in");//  - %s is on\n", sharedSemaphores->sem_in);
	if (sem_wait(&sharedSemaphores->sem_traffic_go) == -1)
		perror("contiki sem_wait() error");
	PRINTF("-- Wait  on  sem_in");// - %s is on\n", sharedSemaphores->sem_in);

}

void ipc_settimer(uint64_t interval, uint8_t type) {
	uint8_t typeOfInformation= TIMER_TYPE;

	// 0 for event timers, 1 for realtime timers
	if (type != 0 && type != 1) {
		puts("Wrong timer type!");
		exit(1);
	}

	PRINTF("-- Wait  on  sharedSemaphores->sem_in");//  - %s is on\n", sharedSemaphores->sem_timer);
	if (sem_wait(&sharedSemaphores->sem_in) == -1)
		perror("contiki sem_wait() error");
	PRINTF("-- Wait  on  sharedSemaphores->sem_in");//  - %s is on\n", sharedSemaphores->sem_timer);

	memcpy(sharedSemaphores->traffic_in, &typeOfInformation, 1);
//	char test[] = "test";
//	memcpy(sharedSemaphores->traffic_in+1, &test, 4);

	memcpy(sharedSemaphores->traffic_in + 1, &type, 1);
	memcpy(sharedSemaphores->traffic_in + 2, &interval, 8);

	PRINTF("-- Post  on  sem_in");//  - %s is on\n", sharedSemaphores->sem_in);
	if (sem_post(&sharedSemaphores->sem_traffic_done) == -1)
		perror("contiki sem_wait() error");
	PRINTF("-- Post  on  sem_in");//  - %s is on\n", sharedSemaphores->sem_in);

	PRINTF("-- Wait  on  sem_in");//  - %s is on\n", sharedSemaphores->sem_in);
	if (sem_wait(&sharedSemaphores->sem_traffic_go) == -1)
		perror("contiki sem_wait() error");
	PRINTF("-- Wait  on  sem_in");// - %s is on\n", sharedSemaphores->sem_in);

}


uint64_t ipc_time(void) {


	// They just read the time, they do not cContiki valor of sem timehange anything so the read
	// do not need to be exclusive
	PRINTF("-- Wait  on  sem_time");//  - %s is on\n",  sharedSemaphores->sem_time);
	if (sem_wait(&sharedSemaphores->sem_time) == -1)
		perror("contiki sem_wait() error");
	PRINTF("-- Wait  on  sem_time");//  - %s is on\n", sharedSemaphores->sem_time);

	memcpy((void *) &now, sharedSemaphores->shm_time, size_time);
	PRINTF("Time Now = %d \n", (int)now);

	PRINTF("-- Post  on  sem_time");//  - %s is on\n", sharedSemaphores->sem_time);
	if (sem_post(&sharedSemaphores->sem_time) == -1)
		perror("contiki sem_post() error");
	PRINTF("-- Post  on  sem_time");//  - %s is on\n", sharedSemaphores->sem_time);

	if (now > old) {
		old = now;
		//printf("contiki %d got time %llu\n", getpid(), now);
	}

	return now;
}


