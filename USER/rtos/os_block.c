/***********************
 *@file: os_block.c
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/
 
#include "os_sched.h"
#include "os_tick.h"
#include "os_block.h"


/* 将链节挂载到阻塞对象 */
__os_prv_ void __os_block_list_add(struct os_block_object * _block_obj, struct task_control_block* _task_tcb)
{
	struct list_head* _block_node = &_block_obj->_list;
	struct list_head* _current_node = NULL;
	struct task_control_block* _current_tcb = NULL;
	
	// 根据优先级先后挂载, 优先级高在表头
	list_for_each(_current_node, _block_node)
	{
		_current_tcb = os_list_entry(_current_node, struct task_control_block, _slot_list);
		if (_current_tcb->_task_priority > _task_tcb->_task_priority)
			break;
	}
	list_add_tail(_current_node, &_task_tcb->_slot_list);
	// 向进程tcb中加入阻塞类对象信息
	_task_tcb->_block_obj = _block_obj;
}

/* 将进程tcb从阻塞类对象链表中移除 */
__os_prv_ void __os_block_list_del(struct task_control_block* _task_tcb)
{
	list_del_init(&_task_tcb->_slot_list);
	_task_tcb->_block_obj = NULL;
}

/* 检测进程tcb是否已被挂载在某一个阻塞类对象中 */
__os_inline_ bool os_task_is_block(struct task_control_block* _task_tcb)
{
	return (_task_tcb->_block_obj != NULL);
}

/* 初始化阻塞类对象 */
void os_block_init(struct os_block_object * _block_obj, os_block_type_t _block_type)
{
	_block_obj->_type = _block_type;
	list_head_init(&_block_obj->_list);
}

/* 阻塞类对象去初始化 */
void os_block_deinit(struct os_block_object * _block_obj)
{
	_block_obj->_type = OS_BLOCK_NONE;
	list_head_init(&_block_obj->_list);
}

/* 检测阻塞类对象链表是否为空 */
__os_inline_ bool os_block_list_is_empty(struct os_block_object * _block_obj)
{
	return (list_empty(&_block_obj->_list));
}

/* 
 *@func: 将进程挂载进阻塞队列 
 *@note: 包含进程的状态更新
 */
os_handle_state_t os_add_block_task(struct task_control_block* _task_tcb, 
									struct os_block_object* _block_obj,
									unsigned int _time_out)
{
	if (NULL == _task_tcb ||
		NULL == _block_obj ||
		0 == _time_out)
		return OS_HANDLE_FAIL;
	// 先将进程从优先队列中移除
	os_rq_del_task(_task_tcb);
	__os_block_list_add(_block_obj, _task_tcb);
	// 状态更新
	__task_state_update(_task_tcb, OS_TASK_READY, (os_task_stack_t)_block_obj->_type);
	if (OS_BLOCK_NEVER_TIME_OUT != _time_out)
	{
		// 如果存在阻塞timeout，则将进程顺带挂载进tick队列
		os_add_tick_task(_task_tcb, _time_out);
	}
	return OS_HANDLE_SUCCESS;
}

/*
 *@func: 将进程从阻塞类对象队列中唤醒
 */
os_handle_state_t os_block_wakeup_task(struct task_control_block* _task_tcb)
{
	if (NULL == _task_tcb)
		return OS_HANDLE_FAIL;
	// 如果在tick队列上存在该进程
	if(os_task_state_is_sleep(_task_tcb))
		os_tick_del_task(_task_tcb);
	if (os_task_is_block(_task_tcb))
		__os_block_list_del(_task_tcb);
	// 将进程加入就绪队列
	os_rq_add_task(_task_tcb);
	return OS_HANDLE_SUCCESS;
}

/*
 *@func: 将阻塞类对象链表中的第一个进程唤醒
 */
__os_inline_ void os_block_wakeup_first_task(struct os_block_object* _block_obj)
{
	if (_block_obj->_list.next != &_block_obj->_list)
	{
		struct task_control_block* _tcb = os_list_entry(_block_obj->_list.next, struct task_control_block, _slot_list);
		os_block_wakeup_task(_tcb);
	}
}

/*
 *@func: 将阻塞类对象链表中的所有进程唤醒
 */
void os_block_wakeup_all_task(struct os_block_object* _block_obj)
{
	struct task_control_block* _task_tcb = NULL;
	struct list_head* _current_node = NULL;
	struct list_head* _next_ndoe = NULL;
	list_for_each_safe(_current_node, _next_ndoe, &_block_obj->_list)
	{
		_task_tcb = os_list_entry(_current_node, struct task_control_block, _slot_list);
		os_block_wakeup_task(_task_tcb);
	}
}
