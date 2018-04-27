#include <stdio.h>
#include <semaphore.h>
#include "thread_watchdog.h"

sem_t sem0,sem1,sem2;


void * test_thread_0(void * arg){
    new_watchdog("thread-0", 10);
    for(;;){
        sem_wait(&sem0);
        watchdog_feed();
        watchdog_dump();
    }
    return NULL;
}



void * test_thread_1(void * arg){
    new_watchdog("thread-1", 15);
    for(;;){
        sem_wait(&sem1);
        watchdog_feed();
        watchdog_dump();
    }
    return NULL;
}


void * test_thread_2(void * arg){
    new_watchdog("thread-2", 20);
    for(;;){
        sem_wait(&sem2);
        watchdog_feed();
        watchdog_dump();
    }
    return NULL;
}

int main(int argc, const char**argv){
    pthread_t tid0,tid1,tid2;

    printf("usage: type 0/1/2 + ENTER "
           "or combination(for exam, \"012\") to feed "
           "thread-0/1/2\r\n");
    sem_init(&sem0,0,0);
    sem_init(&sem1,0,0);
    sem_init(&sem2,0,0);
    
    if(EXIT_SUCCESS != watchdog_initialize("/work/picocom/dog/dog-log")){
        return 1;
    }

    pthread_create(&tid0,NULL,test_thread_0,NULL);
    pthread_create(&tid1,NULL,test_thread_1,NULL);
    pthread_create(&tid2,NULL,test_thread_2,NULL);
    new_watchdog("main", 10);
    for(;;){
        char line[1024];
        fgets(line, sizeof line, stdin);
        watchdog_feed();

        if(strstr(line, "0"))
            sem_post(&sem0);
        if(strstr(line, "1"))
            sem_post(&sem1);
        if(strstr(line, "2"))
            sem_post(&sem2);
    }
    return 0;
}

