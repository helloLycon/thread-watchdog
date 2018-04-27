#include "thread_watchdog.h"


static WatchdogList watchdog_list;


static void watchdog_list_push(WatchdogList *list, Watchdog *dog){
    pthread_mutex_lock(&list->mtx);
    if( !list->head ){
        list->head = list->tail = dog;
        dog->prev = dog->next = NULL;
        goto push_end;
    }
    list->tail->next = dog;
    dog->prev = list->tail;
    dog->next = NULL;
    list->tail = dog;
    
push_end:
    pthread_mutex_unlock(&list->mtx);
}

static void watchdog_list_pop(WatchdogList *list, Watchdog *dog){
    Watchdog *it;

    pthread_mutex_lock(&list->mtx);
    for(it = list->head;it;it = it->next){
        if( it == dog ){
            if( dog == list->head && dog == list->tail ){
                /* pop the unique node */
                list->head = list->tail = NULL;
                goto pop_end;
            }
            if( dog == list->tail ){
                /* pop tail node */
                it->prev->next = it->next;
                list->tail = it->prev;
                goto pop_end;
            }
            if(dog == list->head){
                /* pop head node */
                it->next->prev = it->prev;
                list->head = it->next;
                goto pop_end;
            }
            it->prev->next = it->next;
            it->next->prev = it->prev;
            goto pop_end;
        }
    }
    /* not found */
pop_end:
    pthread_mutex_unlock(&list->mtx);
}

static void watchdog_someone_oops(Watchdog *dog){
    char log[1024];
    char timestr[1024];
    time_t now = time(NULL);
    FILE *fp;

    strftime(timestr, sizeof(timestr), 
             "%Y-%m-%d %H:%M:%S",
             localtime(&now));
    sprintf(log, "[%s]Watchdog: thread<%s> dead(tid = %lu)\r\n",
            timestr,
            dog->name,
            dog->id);
    /* print on the terminal */
    fputs(log, stderr);
    
    /* write into log file */
    fp = fopen(watchdog_list.log_file, "a");
    fputs(log, fp);
    fclose(fp);

    /* exit the process */
    exit(EXIT_FAILURE);
}

static void * watchdog_watch_routine(void * arg){
    WatchdogList * list = arg;
    Watchdog *it;

    for(;;){
        pthread_mutex_lock(&list->mtx);
        for(it = list->head ; it ; it = it->next){
            if( it->countdown <= 0){
                /* this thread dead */
                watchdog_someone_oops(it);
            }
            it->countdown--;
        }
        pthread_mutex_unlock(&list->mtx);
        sleep(1);
    }
    return NULL;
}

int watchdog_initialize(const char * log_file){
    static pthread_t tid = 0;
    pthread_attr_t attr;
    if( !tid ){
        watchdog_list.head = watchdog_list.tail = NULL;
        watchdog_list.log_file = log_file;
        if(pthread_mutex_init(&watchdog_list.mtx, NULL)){
            perror("pthread_mutex_init");
            return EXIT_FAILURE;
        }

        /* boot the thread */
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 64*1024);
        if(pthread_create(&tid, &attr, watchdog_watch_routine, &watchdog_list)){
            perror("pthread_create");
            return EXIT_FAILURE;
        }
        pthread_detach(tid);
    }
    return EXIT_SUCCESS;
}

/**
 * @brief  create a watchdog for one thread
 * @param  name:    name of the thread
 *         timeout: watchdog timeout(unit: second)
 */
Watchdog * new_watchdog(const char *name, int timeout){
    Watchdog * dog = malloc(sizeof(Watchdog));
    
    if(!dog){
        fprintf(stderr, "malloc: failed\r\n");
        exit(EXIT_FAILURE);
    }
    dog->name = name;
    dog->id   = pthread_self();
    dog->timeout = timeout;
    dog->countdown = timeout;

    /* add it into the list */
    watchdog_list_push(&watchdog_list, dog);
    
    return dog;
}

/**
 * @brief  do NOT use it for the moment
 */
void delete_watchdog(Watchdog *dog){
    watchdog_list_pop(&watchdog_list, dog);
    free(dog);
}

/**
 * @ATTENTION  mutex is not locked here,
 *             lock the mutex before calling plz.
 */
static Watchdog * watchdog_find_dog(void){
    pthread_t tid = pthread_self();
    Watchdog * it;

    for(it = watchdog_list.head ; it ; it = it->next){
        if( it->id == tid ){
            return it;
        }
    }
    return NULL;
}


/**
 * @brief feed the dog attached to the thread of caller
 */
int watchdog_feed(void){
    Watchdog * it;

    pthread_mutex_lock(&watchdog_list.mtx);
    it = watchdog_find_dog();
    if(it){
        it->countdown = it->timeout;
        pthread_mutex_unlock(&watchdog_list.mtx);
        return EXIT_SUCCESS;
    }
    
    /* the dog not found */
    pthread_mutex_unlock(&watchdog_list.mtx);
    return EXIT_FAILURE;
}


/**
 * @brief dump the dog attached to the callder thread
 *        for debug
 */
void watchdog_dump(void){
    Watchdog * it;
    pthread_mutex_lock(&watchdog_list.mtx);
    it = watchdog_find_dog();
    if(it){
        printf("-------- watch-dog --------\r\n"
               "name:      %s\r\n"
               "tid:       %lu\r\n"
               "timeout:   %d\r\n"
               "countdown: %d\r\n",
               it->name,
               it->id,
               it->timeout,
               it->countdown);
    }else{
        printf("this thread is no dog attached...\r\n");
    }
    pthread_mutex_unlock(&watchdog_list.mtx);
}


