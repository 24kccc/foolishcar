/***********************
 *@file: os_sys.c
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/

#include "os_sched.h"
#include "os_sys.h"

#define IDLE_TASK_PRIO	OS_TASK_MAX_PRIORITY
#define IDLE_TASK_STACK_SIZE 128
static struct task_control_block _idle_tcb;
static unsigned int idle_task_stack[IDLE_TASK_STACK_SIZE];

static unsigned char _os_iqr_nesting;

void os_idle_task(void* _arg)
{
	while(1)
	{
		int i = 0;
	}
}

static void __idle_task_create(void)
{
	os_task_create(&_idle_tcb, idle_task_stack, IDLE_TASK_STACK_SIZE, IDLE_TASK_PRIO, os_idle_task, NULL, "IDLE TASK");
}

__os_inline_ struct task_control_block* os_get_idle_tcb(void)
{
	return (&_idle_tcb);
}

/* 实时系统初始化 */
void os_sys_init(void)
{
	// 时间片初始化
	os_sched_timeslice_init();
	// 优先级队列初始化
	os_ready_queue_init();
	os_cpu_running_flag = 0;
	__idle_task_create();
	_os_iqr_nesting = 0;
}

/* 实时系统内核启动 */
void os_sys_start(void)
{
	os_task_ready = os_rq_get_highest_prio_task();
	sf_os_start();
}

/* 系统是否处在中断当中, 如果返回true也意味着实时系统此时不会发生调度 */
bool os_sys_is_in_irq(void)
{
	return (_os_iqr_nesting > 0);
}

/* 进入中断, 此函数用于当系统处在中断函数中时禁止实时系统调度 */
void os_sys_enter_irq(void)
{
	unsigned int _critical_state = os_port_enter_critical();
	if (os_cpu_is_running() && 
		_os_iqr_nesting < 255)
		_os_iqr_nesting++;
	os_port_exit_critical(_critical_state);
}

/* 退出中断 */
void os_sys_exit_irq(void)
{
	if (os_cpu_is_running() && 
		os_sys_is_in_irq())
	{
		unsigned int _critical_state = os_port_enter_critical();
		_os_iqr_nesting--;
		os_port_exit_critical(_critical_state);
		// 调度
		if (0 == _os_iqr_nesting)
			__os_sched();
	}
}


__os_inline_ void systick_handler(void)
{
	if (os_cpu_is_running())
	{
		os_sched_timeslice_poll();
		os_task_tick_poll();
	}
}

void soft_timer_systick_handle(void);
void SysTick_Handler(void)
{
	systick_handler();
	soft_timer_systick_handle();
}