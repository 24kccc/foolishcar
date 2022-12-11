/***********************
 *@file: os_block.h
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/
#ifndef _OS_BLOCK_H_
#define _OS_BLOCK_H_
#include "os_core.h"

#define OS_BLOCK_NEVER_TIME_OUT	0xFFFFFFFF

typedef enum os_block_type
{
	OS_BLOCK_NONE = 0,
	OS_BLOCK_MUTEX = (1 << 4L)
}os_block_type_t;

typedef struct os_block_object 
{
    os_block_type_t _type;
    struct list_head _list;
}os_block_object_t;

__os_prv_ void __os_block_list_add(struct os_block_object * _block_obj, struct task_control_block* _task_tcb);
__os_prv_ void __os_block_list_del(struct task_control_block* _task_tcb);
bool os_task_is_block(struct task_control_block* _task_tcb);
void os_block_init(struct os_block_object * _block_obj, os_block_type_t _block_type);
void os_block_deinit(struct os_block_object * _block_obj);
bool os_block_list_is_empty(struct os_block_object * _block_obj);
os_handle_state_t os_add_block_task(struct task_control_block* _task_tcb, 
									struct os_block_object* _block_obj,
									unsigned int _time_out);
os_handle_state_t os_block_wakeup_task(struct task_control_block* _task_tcb);
void os_block_wakeup_first_task(struct os_block_object* _block_obj);
void os_block_wakeup_all_task(struct os_block_object* _block_obj);

#endif