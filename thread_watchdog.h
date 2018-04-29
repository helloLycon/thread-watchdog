/*
 * @brief    A stupid watchdog for your working threads
 *
 * @author   lycon
 * @blog     https://lk361115629.github.io/
 * @github   https://github.com/lk361115629
 *
 */

#ifndef  __THREAD_WATCHDOG_H
#define  __THREAD_WATCHDOG_H



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>



typedef struct Watchdog{
    const char * name;
    pthread_t    id;
    int timeout;
    int countdown;

    /* list */
    struct Watchdog *prev;
    struct Watchdog *next;
} Watchdog;


typedef struct {
    Watchdog * head;
    Watchdog * tail;
    const char * log_file;
    pthread_mutex_t mtx;
} WatchdogList;



int watchdog_initialize(const char * log_file);
Watchdog * new_watchdog(const char * name, int timeout);
int watchdog_feed(void);
void watchdog_dump(void);






#endif

