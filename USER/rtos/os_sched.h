/***********************
 *@file: os_sched.h
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/


#ifndef _OS_SCHED_H_
#define _OS_SCHED_H_
#include "os_list.h"
#include "os_config.h"
#include "os_core.h"
#include "os_tick.h"

#define OS_SCHED_TIMESLICE_NULL 0xFFFFFFFF

struct os_ready_queue
{
	unsigned int _highest_priority;
	struct list_head _queue[OS_READY_LIST_SIZE];
};

struct os_sched_timeslice_pos
{
	unsigned int _last_priority;
	struct list_head* _last_task_node;
};

void __insert_task_priority(unsigned char _priority);
void __del_task_priority(unsigned char _priority);
__os_prv_ unsigned int __ffb(unsigned int _word);
void update_ready_queue_priority(void);
unsigned char __get_highest_ready_priority(void);
void os_rq_add_task(struct task_control_block* _task);
void os_rq_del_task(struct task_control_block* _task);
struct task_control_block* os_rq_get_highest_prio_task(void);
void os_ready_queue_init(void);
os_handle_state_t os_sched_lock(void);
bool os_sched_is_lock(void);
os_handle_state_t os_sched_unlock(void);
void os_sched_timeslice_init(void);
void os_sched_timeslice_set(unsigned int _prio, unsigned int _new_timeslice);
unsigned int os_sched_timeslice_get(unsigned int _prio);
void os_sched_timeslice_reload(struct task_control_block* _task_tcb);
void os_sched_timeslice_poll(void);
__os_prv_ void __os_sched_timeslice_task_rq_del(struct task_control_block* _task);
void os_diable_sched(void);
void os_enable_sched(void);

#endif