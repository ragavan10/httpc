#include "httpc_thread_pool.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_THREAD      10
#define NUM_WORK        50


static void *httpc_thread_worker(void *p)
{
    /* srand(time(NULL)); */
    int thread_id = rand() % 20;
    printf("Thread worker id %d\n", thread_id);
    httpc_thread_pool_t *pp = (httpc_thread_pool_t *)p;
    while (true) {
        pthread_mutex_lock(pp->mutex);
        while (NULL == pp->work)
           pthread_cond_wait(pp->work_cond, pp->mutex); 
        printf("Thread worker id %d is running\n", thread_id);
        // Pop the work
        httpc_work_t *w = pp->work;
        pp->work = pp->work->next;
        pthread_mutex_unlock(pp->mutex);
        // Perform the function
        w->fn(w->param);
        free(w);
    }
    return NULL; 
}

static httpc_err_e httpc_thread_work_create(httpc_work_t **w, httpc_thread_fn fn, 
        void *p)
{
    if (NULL == fn)
        return HTTPC_ERR_OTHER;
    httpc_work_t *nw = malloc(sizeof(*nw));
    if (NULL == nw)
        return HTTPC_ERR_MEM_ALLOC;

    nw->fn = fn;
    nw->param = p;
    nw->next = *w;
    *w = nw;

    return HTTPC_ERR_NONE;
}

static void httpc_thread_work_destroy(httpc_work_t *w)
{
    while (w) {
        httpc_work_t *tmp = w;
        w = w->next;
        free(tmp);
    }
}

httpc_err_e httpc_thread_add_work(httpc_thread_pool_t *tp, 
        httpc_thread_fn fn, void *p)
{
    httpc_work_t *w = NULL;
    pthread_mutex_lock(tp->mutex);
    httpc_err_e ret = httpc_thread_work_create(&w, fn, p);
    if (ret != HTTPC_ERR_NONE)
        return ret;
    tp->work = w;
    pthread_cond_broadcast(tp->work_cond);
    pthread_mutex_unlock(tp->mutex);
    return HTTPC_ERR_NONE;
}

httpc_err_e httpc_thread_pool_create(httpc_thread_pool_t **tp, size_t tcount)
{
    *tp = malloc(sizeof(**tp));
    if (NULL == *tp)
        return HTTPC_ERR_MEM_ALLOC;
    (*tp)->work = NULL;
    (*tp)->threads = (pthread_t *)malloc(tcount * sizeof(pthread_t));
    if ( NULL == (*tp)->threads)
        return HTTPC_ERR_MEM_ALLOC;
    (*tp)->thread_attrs = (pthread_attr_t *)malloc(
            tcount * sizeof(pthread_attr_t));
    if (NULL == (*tp)->thread_attrs)
        return HTTPC_ERR_MEM_ALLOC;
    (*tp)->thread_count = tcount;
    (*tp)->mutex = malloc(sizeof(pthread_mutex_t));
    if (NULL == (*tp)->mutex)
        return HTTPC_ERR_MEM_ALLOC;
    pthread_mutex_init((*tp)->mutex, NULL);
    (*tp)->work_cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    if (NULL == (*tp)->work_cond)
        return HTTPC_ERR_MEM_ALLOC;
    pthread_cond_init((*tp)->work_cond, NULL);

    // Create threads
    for (size_t i = 0; i < tcount; i ++) {;
        pthread_attr_init((*tp)->thread_attrs + i);
        pthread_attr_setdetachstate((*tp)->thread_attrs + i, PTHREAD_CREATE_DETACHED);
        pthread_create((*tp)->threads + i, (*tp)->thread_attrs + i, httpc_thread_worker, (void *)(*tp));
    }
    return HTTPC_ERR_NONE;
}

void httpc_thread_pool_destroy(httpc_thread_pool_t *tp)
{
    free(tp->threads);
    free(tp->mutex);
    httpc_thread_work_destroy(tp->work);
    for (size_t i = 0; i < tp->thread_count; i++)
        pthread_attr_destroy(tp->thread_attrs + i);
    free(tp->thread_attrs);
    free(tp);
}
