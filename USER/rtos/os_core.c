/***********************
 *@file: os_core.c
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/

#include "os_core.h"
#include "os_sched.h"
#include "os_sys.h"
#include "os_tick.h"

static struct task_control_block* _os_id_tcb_tab[OS_TASK_MAX_ID];

unsigned int os_cpu_running_flag = 0;
struct task_control_block* os_task_current = NULL;
struct task_control_block* os_task_ready = NULL;

inline bool os_cpu_is_running(void)
{
	return (os_cpu_running_flag == 1);
}

os_handle_state_t os_task_create(struct task_control_block* _task_tcb,
								 unsigned int* _stack_addr,
								 unsigned int _stack_size,
								 unsigned char _prio,
								 __task_fn_ _entry_fn,
								 void* _entry_fn_arg,
								 const char* _task_name)
{
	if (NULL == _task_tcb ||
		NULL == _stack_addr ||
		NULL == _entry_fn ||
		_prio > OS_TASK_MAX_PRIORITY)
		return OS_HANDLE_FAIL;
	
	unsigned int _last_critical_state = os_port_enter_critical();
	unsigned int _tmp_id = OS_TASK_MAX_ID + 1;
	
	for (unsigned int _i = 0; _i <= OS_TASK_MAX_ID; ++_i)
	{
		if (NULL == _os_id_tcb_tab[_i])
		{
			_tmp_id = _i;
			break;
		}
	}
	if (_tmp_id > OS_TASK_MAX_ID)
	{
		os_port_exit_critical(_last_critical_state);
		return OS_HANDLE_FAIL;
	}
	_os_id_tcb_tab[_tmp_id] = _task_tcb;
	_task_tcb->_task_id = _tmp_id;
	
	// 初始化进程堆栈
	_task_tcb->_stack_top = os_process_stack_init((void*)_entry_fn, _entry_fn_arg, (void*)__os_task_exit_handle, _stack_addr, _stack_size);
	
	_task_tcb->_task_priority = _prio;
	_task_tcb->_task_name = _task_name;
	
	/* note: 在加入就绪队列中会将进程状态置为ready状态 */
	_task_tcb->_task_state = OS_TASK_NONE;
	// __task_state_update(_task_tcb, OS_TASK_STATE_MASK, OS_TASK_READY);
	_task_tcb->_task_wait_time = 0;
	_task_tcb->_task_timeslice = 0;
	_task_tcb->_block_obj = NULL;
	
	// 链表初始化
	list_head_init(&_task_tcb->_slot_list);
	list_head_init(&_task_tcb->_tick_list);
	
	// 加入优先级队列
	os_rq_add_task(_task_tcb);
	os_port_exit_critical(_last_critical_state);
	
	// 	调度
	__os_sched();
	
	return OS_HANDLE_SUCCESS;
}

/* 实时系统调度函数 */
void __os_sched(void)
{
	unsigned int _state = os_port_enter_critical();

	/*
	* 执行调度前提条件: 内核已启动; 不处于中断当中
	 */
	if (!os_cpu_is_running() || 
		os_sys_is_in_irq())
	{
		os_port_exit_critical(_state);
		return;
	}
	
	// 当进程处于调度锁并且存在延时时,将进程切换到系统空闲进程
	if (os_sched_is_lock())
	{
		static struct task_control_block* _lock_tcb;
		if (os_task_current != os_get_idle_tcb())
			_lock_tcb = os_task_current;
		if (os_task_state_is_sleep(_lock_tcb))
			os_task_ready = os_get_idle_tcb();
		else
			os_task_ready = _lock_tcb;
	}
	else
		os_task_ready = os_rq_get_highest_prio_task();
	if (NULL == os_task_ready ||
		os_task_current == os_task_ready)
	{
		os_port_exit_critical(_state);
		return;
	}
	os_port_exit_critical(_state);
	// 触发PendSV异常，以进行上下文切换
	sf_os_ctx_sw();
}

/* 进程退出 */
__os_prv_ void __os_task_exit_handle(void)
{
	unsigned int _state = os_port_enter_critical();
	
	// 清除id
	_os_id_tcb_tab[os_task_current->_task_id] = NULL;
	// 将进程从就绪队列中清除
	os_rq_del_task(os_task_current);
	os_task_current = NULL;
	os_port_exit_critical(_state);
	// 执行调度
	__os_sched();
}
