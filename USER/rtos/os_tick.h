/***********************
 *@file: os_tick.h
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/

#ifndef	_OS_TICK_H_
#define _OS_TICK_H_

#include "os_list.h"
#include "os_config.h"
#include "os_core.h"
#include "os_sched.h"

void __task_state_update(struct task_control_block* _task_tcb, 
						 os_task_state_t _clr,
						 os_task_state_t _new);
bool os_task_state_is_ready(struct task_control_block* _task_tcb);
bool os_task_state_is_sleep(struct task_control_block* _task_tcb);
void os_task_state_set_ready(struct task_control_block* _task_tcb);
os_handle_state_t os_add_tick_task(struct task_control_block* _task_tcb, unsigned int _tick);
void os_task_tick_poll(void);
void os_tick_del_task(struct task_control_block* _task_tcb);
void os_task_state_clr_sleep(struct task_control_block* _task_tcb);
void os_task_delay_ms(unsigned int _tick_ms);
#endif