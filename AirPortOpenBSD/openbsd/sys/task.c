//
//  task.c
//  AirPortOpenBSD
//
//  Created by Mac-PC on 2020/3/18.
//  Copyright © 2020 Zhong-Mac. All rights reserved.
//

#include <sys/kern_compat.h>
#include <sys/_malloc.h>
#include <sys/mutex.h>
#include "task.h"

struct pthread_cond {
    volatile unsigned int seq;
    pthread_mutex_t          mtx;
};

struct taskq {
    thread_t            thread;
    struct task_list    list;
    pthread_mutex_t     mtx;
    pthread_cond_t      cv;
};


struct taskq *const systq = taskq_create("systq", 1, IPL_NET, 0);

static void taskq_run(void* tqarg, wait_result_t waitResult);

int
pthread_mutex_init(pthread_mutex_t *mutex,
                   void *attr)
{
    mutex->lock = IORecursiveLockAlloc();
    return (0);
}

int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
    IORecursiveLockLock(mutex->lock);
    return (0);
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    IORecursiveLockUnlock(mutex->lock);
    return (0);
}

int
pthread_cond_init(pthread_cond_t *condp, const void *attr)
{
    *condp = (pthread_cond_t)malloc(sizeof(struct pthread_cond), M_DEVBUF, M_NOWAIT | M_ZERO);
    if (*condp == NULL)
        return 1;
    
    return 0;
}

struct taskq *
taskq_create(const char *name, unsigned int nthreads, int ipl,
             unsigned int flags)
{
    struct taskq *tq;
    int error;
    
    tq = (struct taskq *)malloc(sizeof(*tq), M_DEVBUF, M_NOWAIT | M_ZERO);
    if (tq == NULL)
        return (NULL);

    TAILQ_INIT(&tq->list);

    error = pthread_mutex_init(&tq->mtx, NULL);

    error = pthread_cond_init(&tq->cv, NULL);
    
    kernel_thread_start(taskq_run, tq, &tq->thread);
    thread_deallocate(tq->thread);

    return (tq);

}

void taskq_barrier(struct taskq *)
{
    
}

void taskq_destroy(struct taskq *)
{
    
}

int
pthread_cond_wait(pthread_cond_t condp, pthread_mutex_t *mutexp)
{
    int ret;
    mtx_enter(mutexp);
    ret = IORecursiveLockSleep(mutexp->lock, condp, THREAD_INTERRUPTIBLE);
    mtx_leave(mutexp);
    
    /* return value is always >= 0 */
    //    if (ret != THREAD_AWAKENED)
    //        return -ETIMEDOUT;
    return 0;
}


int
pthread_cond_signal(pthread_cond_t condp)
{
    mtx_enter(&condp->mtx);
    IORecursiveLockWakeup(condp->mtx.lock, condp, true);
    mtx_leave(&condp->mtx);
    return 1;
}

void task_run(void* tqarg, wait_result_t waitResult)
{
    struct task *t = (struct task *)tqarg;
    (*t->t_func)(t->t_arg);
    free(t, M_DEVBUF, sizeof(struct task));
}

void task_set_and_run(void (*t_func)(void *), void *arg)
{
    struct task *t = (struct task *)malloc(sizeof(struct task), M_DEVBUF, M_NOWAIT | M_ZERO);
    if (t == NULL)
        return;
    t->t_func = t_func;
    t->t_arg = arg;
    thread_t new_thread = NULL;
    kernel_thread_start(task_run, t, &new_thread);
    thread_deallocate(new_thread);
}


void
taskq_run(void* tqarg, wait_result_t waitResult)
{
    struct taskq *tq = (struct taskq *)tqarg;
    struct task *t;
    
    void (*t_func)(void *);
    void *t_arg;
    
    for (;;) {
        pthread_mutex_lock(&tq->mtx);
        while ((t = TAILQ_FIRST(&tq->list)) == NULL)
            pthread_cond_wait(tq->cv, &tq->mtx);
        
        TAILQ_REMOVE(&tq->list, t, t_entry);
        CLR(t->t_flags, TASK_ONQUEUE);
        
        t_func = t->t_func;
        t_arg = t->t_arg;
        
        pthread_mutex_unlock(&tq->mtx);
        (*t_func)(t_arg);
    }
    
}

void
task_set(struct task *t, void (*fn)(void *), void *arg)
{
    t->t_func = fn;
    t->t_arg = arg;
    t->t_flags = 0;
}

int
task_add(struct taskq *tq, struct task *t)
{
    int rv = 1;
    
    if (ISSET(t->t_flags, TASK_ONQUEUE))
        return (0);
    
    pthread_mutex_lock(&tq->mtx);
    if (ISSET(t->t_flags, TASK_ONQUEUE))
        rv = 0;
    else {
        SET(t->t_flags, TASK_ONQUEUE);
        TAILQ_INSERT_TAIL(&tq->list, t, t_entry);
        tq->cv->mtx = tq->mtx;
        pthread_cond_signal(tq->cv);
    }
    pthread_mutex_unlock(&tq->mtx);
    
    return (rv);
}


int
task_del(struct taskq *tq, struct task *t)
{
    int rv = 1;
    
    if (!ISSET(t->t_flags, TASK_ONQUEUE))
        return (0);
    
    pthread_mutex_lock(&tq->mtx);
    if (!ISSET(t->t_flags, TASK_ONQUEUE))
        rv = 0;
    else {
        TAILQ_REMOVE(&tq->list, t, t_entry);
        CLR(t->t_flags, TASK_ONQUEUE);
    }
    pthread_mutex_unlock(&tq->mtx);
    
    return (rv);
}
