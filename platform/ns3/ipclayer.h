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

#ifdef	__cplusplus
extern "C" {
#endif

    void ipc_init(char *id);
    size_t ipc_read(uint8_t *buf);
    void ipc_write(void *buf, size_t len);
    uint64_t ipc_time(void);
    void ipc_settimer(uint64_t interval, uint8_t type);

#ifdef	__cplusplus
}
#endif

#endif	/* IPCLAYER_H */

