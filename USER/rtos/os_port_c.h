/***********************
 *@file: os_port_c.h
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/

#ifndef _OS_PORT_H_
#define _OS_PORT_H_
#include "os_core.h"
#include "os_config.h"

typedef unsigned int STACK_TYPE;

extern struct task_control_block* os_task_current;
extern struct task_control_block* os_task_ready;
extern unsigned int enter_critical_num;

void sf_os_start(void);
void sf_os_ctx_sw(void);

unsigned int* os_process_stack_init(void* _fn_entry, 
									void* _arg, 
									void* _exit, 
									STACK_TYPE* _stack_addr, 
									unsigned int _stack_size);
unsigned int os_port_enter_critical(void);
void os_port_exit_critical(unsigned int _state);
void os_port_cpu_int_disable(void);
void os_port_cpu_int_enable(void);

#endif