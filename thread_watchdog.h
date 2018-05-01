/**
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


#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE  1
#endif

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


/* call it first of all */
int watchdog_initialize(const char * log_file);


/** 
 * @brief   call it in your work-thread to create a new dog 
 * @param   name: name of thread
 *          timeout: timeout of watchdog
 */
Watchdog * new_watchdog(const char * name, int timeout);


/* feed your dog periodically */
int watchdog_feed(void);

/* dump your dog for debug */
void watchdog_dump(void);






#endif

