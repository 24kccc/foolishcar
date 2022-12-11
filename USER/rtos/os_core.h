/***********************
 *@file: os_core.h
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/


#ifndef _OS_CORE_H_
#define _OS_CORE_H_

#include "os_list.h"
#include "os_config.h"
#include "os_port_c.h"


typedef void (*__task_fn_)(void* _arg);

typedef unsigned int os_task_stack_t;
typedef unsigned char os_task_priority_t;

typedef enum os_task_state
{
	OS_TASK_NONE = 0,
	OS_TASK_READY = 1,
	OS_TASK_RUNNING = (1 << 1L),
	OS_TASK_STOPPED = (1 << 2L),
	OS_TASK_SLEEP = (1 << 3L),
	OS_TASK_MUTEX = (1 << 4L)
}os_task_state_t;

#define OS_TASK_STATE_MASK (OS_TASK_READY | \
							OS_TASK_RUNNING | \
							OS_TASK_STOPPED | \
							OS_TASK_SLEEP | \
							OS_TASK_MUTEX)

typedef struct task_control_block
{
	// pointer of task stack top
	os_task_stack_t* _stack_top;
	// task priority
	os_task_priority_t _task_priority;

	// task state
	os_task_state_t _task_state;
	const char* _task_name;
	unsigned int _task_wait_time;
	unsigned int _task_id;
	unsigned int _task_timeslice;
	
	struct os_block_object * _block_obj;
	
	struct list_head _tick_list;
	struct list_head _slot_list;
}tcb_t;



os_handle_state_t os_task_create(struct task_control_block* _task_tcb,
							unsigned int* _stack_addr,
							unsigned int _stack_size,
							unsigned char _prio,
							__task_fn_ _entry_fn,
							void* _entry_fn_arg,
							const char* _task_name);
bool os_cpu_is_running(void);
void __os_sched(void);
__os_prv_ void __os_task_exit_handle(void);

#endif