/***********************
 *@file: os_sys.h
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/

#ifndef _OS_SYS_H_
#define _OS_SYS_H_
#include "os_core.h"
#include "os_sched.h"
extern unsigned int os_cpu_running_flag;

struct task_control_block* os_get_idle_tcb(void);
void os_sys_init(void);
void os_sys_start(void);
bool os_sys_is_in_irq(void);
void os_sys_enter_irq(void);
void os_sys_exit_irq(void);
os_handle_state_t os_sched_lock(void);
void systick_handler(void);

#endif
