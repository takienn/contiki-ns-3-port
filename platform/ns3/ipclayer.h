/* 
 * File:   ipclayer.h
 * Author: kentux
 *
 * Created on March 27, 2013, 6:24 PM
 */

#ifndef IPCLAYER_H
#define	IPCLAYER_H

#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "contiki.h"

typedef struct semaphores_t {
	int id;

	sem_t sem_time;
	sem_t sem_timer;

	sem_t sem_timer_go;
	sem_t sem_timer_done;

	sem_t sem_in;
	sem_t sem_out;

	sem_t sem_traffic_go;
	sem_t sem_traffic_done;

	sem_t sem_go;
	sem_t sem_done;

	/**
	 * \internal
	 * The pointer to a shared memory address where to receive traffic from.
	 */
	unsigned char *traffic_in;
	int traffic_in_id;
	/**
	 * \internal
	 * The pointer to a shared memory address where to send traffic to.
	 */
	unsigned char *traffic_out;
	int traffic_out_id;
	/**
	 * \internal
	 * The pointer to a shared memory address where to store the timers.
	 */
	unsigned char *traffic_timer;
	int traffic_timer_id;


	/**
	 * \internal
	 * The pointer to a shared memory address where to store the time.
	 */
	int *shm_time;
	int shm_time_id;
//	unsigned char *traffic_time;
//	int traffic_time_id;
//	/**
//	 * \internal
//	 * Shared Memory Object for input traffic
//	 */
//	int *shm_in;
//	int shm_in_id;
//	/**
//	 * \internal
//	 * Shared Memory Object for output traffic
//	 */
//	int *shm_out;
//	int shm_out_id;

} semaphores_t;

#ifdef	__cplusplus
extern "C" {
#endif

    void ipc_init(char *id, semaphores_t *sharedSemaphoresTmp);
    size_t ipc_read(void *buf);
    void ipc_write(uint8_t *buf, size_t len);
    uint64_t ipc_time(void);
    void ipc_settimer(uint64_t interval, uint8_t type);

#ifdef	__cplusplus
}
#endif



#endif	/* IPCLAYER_H */

