/***********************
 *@file: os_tick.c
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/

#include "os_tick.h"
#include "os_sys.h"

LIST_HEAD(_os_tick_list_head);

__os_prv_ void __os_tick_add_list(struct task_control_block* _task_tcb, unsigned int _tick)
{
	// 采取阶梯增量式tick计算方法
	_task_tcb->_task_wait_time = _tick;
	
	struct list_head* _current_node = NULL;
	struct task_control_block* _current_node_tcb = NULL;
	unsigned int _prev_tick = 0;
	unsigned int _current_tick = 0;
	
	list_for_each(_current_node, &_os_tick_list_head)
	{
		_current_node_tcb = os_list_entry(_current_node, struct task_control_block, _tick_list);
		_current_tick = _prev_tick + _current_node_tcb->_task_wait_time;
		if (_current_tick > _task_tcb->_task_wait_time)
			break;
		if (_current_tick == _task_tcb->_task_wait_time && 
			_current_node_tcb->_task_priority > _task_tcb->_task_priority)
			break;
		_prev_tick = _current_tick;
	}
	
	_task_tcb->_task_wait_time -= _prev_tick;
	
	if (NULL != _current_node && _current_node != &_os_tick_list_head)
		_current_node_tcb->_task_wait_time -= _task_tcb->_task_wait_time;
	list_add_tail(_current_node, &_task_tcb->_tick_list);
}

__os_inline_ void __task_state_update(struct task_control_block* _task_tcb, 
									  os_task_state_t _clr,
									  os_task_state_t _new)
{
	_task_tcb->_task_state &= (~(_clr & OS_TASK_STATE_MASK));
	_task_tcb->_task_state |= (_new & OS_TASK_STATE_MASK);
}

__os_inline_ bool os_task_state_is_ready(struct task_control_block* _task_tcb)
{
	return (_task_tcb->_task_state & OS_TASK_READY);
}

__os_inline_ bool os_task_state_is_sleep(struct task_control_block* _task_tcb)
{
	return (_task_tcb->_task_state & OS_TASK_SLEEP);
}

__os_inline_ void os_task_state_set_ready(struct task_control_block* _task_tcb)
{
	_task_tcb->_task_state |= (OS_TASK_READY & OS_TASK_STATE_MASK);
}

__os_inline_ void os_task_state_clr_sleep(struct task_control_block* _task_tcb)
{
	_task_tcb->_task_state &= ~OS_TASK_SLEEP;
}

os_handle_state_t os_add_tick_task(struct task_control_block* _task_tcb, unsigned int _tick)
{
	if (NULL == _task_tcb || os_task_state_is_sleep(_task_tcb))
		return OS_HANDLE_FAIL;
	__os_tick_add_list(_task_tcb, _tick);
	__task_state_update(_task_tcb, OS_TASK_READY, OS_TASK_SLEEP);
	return OS_HANDLE_SUCCESS;
}

void os_tick_del_task(struct task_control_block* _task_tcb)
{
	list_del_init(&_task_tcb->_tick_list);
	os_task_state_clr_sleep(_task_tcb);
}
extern int _ccccc1;
extern int _ccccc2;
/* 进程 tick 轮询 */
void os_task_tick_poll(void)
{
	unsigned int _critical_state = os_port_enter_critical();
	os_sys_enter_irq();
	if (list_empty(&_os_tick_list_head))
	{
		os_sys_exit_irq();
		os_port_exit_critical(_critical_state);
		return;
	}
	struct task_control_block* _current_node_tcb;
	_current_node_tcb = os_list_first_entry(&_os_tick_list_head, struct task_control_block, _tick_list);
	--(_current_node_tcb->_task_wait_time);
	if (_current_node_tcb->_task_wait_time > 0)
	{
		os_sys_exit_irq();
		os_port_exit_critical(_critical_state);
		return;
	}
	
	struct list_head* _current_node;
	struct list_head* _next_node;
	/* 由于需要将_tick_wait_time == 0的节点移除，
	   这里需要使用list_for_each_safe以免指针丢失 */
	list_for_each_safe(_current_node, _next_node, &_os_tick_list_head)
	{
		_current_node_tcb = os_list_entry(_current_node, struct task_control_block, _tick_list);
		if (_current_node_tcb->_task_wait_time > 0)
			break;
		
		// 从 _os_tick_list_head 队列中移除
		os_tick_del_task(_current_node_tcb);
		// 加入就绪队列
		os_rq_add_task(_current_node_tcb);
	}
	os_sys_exit_irq();
	os_port_exit_critical(_critical_state);
	// 调度
	__os_sched();
}


/* RTOS延时函数 */
void os_task_delay_ms(unsigned int _tick_ms)
{
	if (_tick_ms == 0)
		return;
	unsigned int _critical_state = os_port_enter_critical();
	// 先从就绪队列中剔除
	os_rq_del_task(os_task_current);
	// 加入延时队列
	os_add_tick_task(os_task_current, _tick_ms);
	os_port_exit_critical(_critical_state);
	// 调度开启
	__os_sched();
}
