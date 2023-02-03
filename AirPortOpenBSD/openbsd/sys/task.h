//
//  task.h
//  AirPortOpenBSD
//
//  Created by Zhong-Mac on 2019/7/29.
//  Copyright © 2019 Zhong-Mac. All rights reserved.
//

#ifndef task_h
#define task_h

#include <sys/queue.h>

struct taskq;

struct task {
    TAILQ_ENTRY(task) t_entry;
    void        (*t_func)(void *);
    char *      t_func_name;
    void        *t_arg;
    unsigned int    t_flags;
};

#define TASK_ONQUEUE        1
#define TASK_BARRIER        2

TAILQ_HEAD(task_list, task);

#define TASKQ_MPSAFE        (1 << 0)
#define TASKQ_CANTSLEEP        (1 << 1)

#define TASK_INITIALIZER(_f, _a)  {{ NULL, NULL }, (_f), (_a), 0 }

extern struct taskq *const systq;
//extern struct taskq *const systqmp;

struct taskq *taskq_create(const char *, unsigned int, int, unsigned int);
void taskq_barrier(struct taskq *);
void taskq_destroy(struct taskq *);

void task_set(struct task *, void (*)(void *), void *);
int task_add(struct taskq *, struct task *);
int task_del(struct taskq *, struct task *);

void task_set_and_run(void (*)(void *), void *);

#define task_pending(_t)    ((_t)->t_flags & TASK_ONQUEUE)

#endif /* task_h */
