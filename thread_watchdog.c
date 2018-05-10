/**
 * @brief    A stupid watchdog for your working threads
 *
 * @author   lycon
 * @blog     https://lk361115629.github.io/
 * @github   https://github.com/lk361115629
 *
 */
#include <stdarg.h>
#include <sys/file.h>
#include "thread_watchdog.h"

/* a simple prompt */
#define WATCHDOG_NOT_FOUND_PROMPT  "this thread is no dog attached..."


static WatchdogList watchdog_list;


static void watchdog_log(const char *fmt, ...){
    char timestr[1024];
    time_t now = time(NULL);
    FILE *fp;
    va_list ap;
    va_start(ap, fmt);
    
    strftime(timestr, sizeof(timestr), 
             "%y-%m-%d,%H:%M:%S",
             localtime(&now));
    /* print on terminal */
    printf("[%s]Watchdog: ", timestr);
    vprintf(fmt, ap);
    if(watchdog_list.log_file){
        /* write into log file */
        fp = fopen(watchdog_list.log_file, "a");
        flock(fileno(fp), LOCK_SH);
        fseek(fp, 0, SEEK_END);
        
        fprintf(fp, "[%s]Watchdog: ", timestr);
        vfprintf(fp, fmt, ap);
        
        flock(fileno(fp), LOCK_UN);
        fclose(fp);
    }
    va_end(ap);
}


static void watchdog_someone_oops(Watchdog *dog){
    watchdog_log("Thread <%s> dead(tid = %lu)\r\n", dog->name, dog->id);
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

/**
 * @brief  call this first of all to initialize 
 *         watchdog thread and somewhat else
 */         
int watchdog_init(const char * log_file){
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

static void watchdog_list_push(WatchdogList *list, Watchdog *dog){
    if( !list->head ){
        list->head = list->tail = dog;
        dog->prev = dog->next = NULL;
        return;
    }
    list->tail->next = dog;
    dog->prev = list->tail;
    dog->next = NULL;
    list->tail = dog;
}

static void watchdog_list_pop(WatchdogList *list, Watchdog *dog){
    if( list->head == list->tail ){
        /* pop the unique node */
        list->head = list->tail = NULL;
        return;
    }
    if( dog == list->tail ){
        /* pop tail node */
        dog->prev->next = NULL;
        list->tail = dog->prev;
        return;
    }
    if(dog == list->head){
        /* pop head node */
        dog->next->prev = NULL;
        list->head = dog->next;
        return;
    }
    dog->prev->next = dog->next;
    dog->next->prev = dog->prev;
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
 * @brief  create a watchdog for one thread
 * @param  name:    name of the thread
 *         timeout: watchdog timeout(unit: second)
 */
Watchdog * new_watchdog(const char *name, int timeout){
    Watchdog * dog;

    pthread_mutex_lock(&watchdog_list.mtx);
    dog = watchdog_find_dog();
    if(dog){
        fprintf(stderr, "new_watchdog: can not create more than one dog for a thread!\r\n");
        dog = NULL;
    } else {
        dog = malloc(sizeof(Watchdog));
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

        /* LOG INTO FILE */
        watchdog_log("Dog for <%s> was created\r\n", dog->name);
    }
    pthread_mutex_unlock(&watchdog_list.mtx);
    return dog;
}

/**
 * @brief  delete the watchdog attached to the thread
 */
void delete_watchdog(void){
    Watchdog * dog;
    
    pthread_mutex_lock(&watchdog_list.mtx);
    dog = watchdog_find_dog();
    if(dog) {
        watchdog_list_pop(&watchdog_list, dog);
        free(dog);
    } else {
        fprintf(stderr, "delete_watchdog: "WATCHDOG_NOT_FOUND_PROMPT"\r\n");
    }
    pthread_mutex_unlock(&watchdog_list.mtx);
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
    } else {
        fprintf(stderr, "watchdog_feed: "WATCHDOG_NOT_FOUND_PROMPT"\r\n");
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
    } else {
        fprintf(stderr, "watchdog_dump: "WATCHDOG_NOT_FOUND_PROMPT"\r\n");
    }
    pthread_mutex_unlock(&watchdog_list.mtx);
}


