#include <stdio.h>
#include <semaphore.h>
#include "thread_watchdog.h"

sem_t sem0,sem1,sem2;
int del0,del1,del2;

void * test_thread_0(void * arg){
    new_watchdog("thread-0", 10);
    for(;;){
        sem_wait(&sem0);
        if(del0){
            delete_watchdog();
        } else {
            watchdog_feed();
        }
        watchdog_dump();
    }
    return NULL;
}



void * test_thread_1(void * arg){
    new_watchdog("thread-1", 15);
    for(;;){
        sem_wait(&sem1);
        if(del1){
            delete_watchdog();
        } else {
            watchdog_feed();
        }
        watchdog_dump();
    }
    return NULL;
}


void * test_thread_2(void * arg){
    new_watchdog("thread-2", 20);
    for(;;){
        sem_wait(&sem2);
        if(del2){
            delete_watchdog();
        } else {
            watchdog_feed();
        }
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
    
    if(EXIT_SUCCESS != watchdog_initialize("./dog-log")){
        return 1;
    }

    pthread_create(&tid0,NULL,test_thread_0,NULL);
    pthread_create(&tid1,NULL,test_thread_1,NULL);
    pthread_create(&tid2,NULL,test_thread_2,NULL);
    for(;;){
        char line[1024];
        fgets(line, sizeof line, stdin);

        if(strstr(line, "d0")){
            del0 = 1;
            sem_post(&sem0);
            continue;
        }
        if(strstr(line, "d1")){
            del1 = 1;
            sem_post(&sem1);
            continue;
        }
        if(strstr(line, "d2")){
            del2 = 1;
            sem_post(&sem2);
            continue;
        }
        if(strstr(line, "0")){
            del0 = 0;
            sem_post(&sem0);
        }
        if(strstr(line, "1")){
            del1 = 0;
            sem_post(&sem1);
        }
        if(strstr(line, "2")){
            del2 = 0;
            sem_post(&sem2);
        }
    }
    return 0;
}

