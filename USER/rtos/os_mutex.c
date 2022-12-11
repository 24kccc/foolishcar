/***********************
 *@file: os_mutex.c
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/

#include "os_block.h"
#include "os_mutex.h"


/*
 *@func: 将锁的拥有者(进程)的优先级提高，防止出现优先级反转
 */
__os_prv_ __os_inline_ void __mutex_owner_prio_up(struct os_mutex* _mutex, unsigned int _prio)
{
	_mutex->_mutex_owner->_task_priority = _prio;
}

/*
 *@func: 更改锁的拥有者(进程)
 */
__os_prv_ void __mutex_owner_change(struct os_mutex* _mutex, struct task_control_block* _task_tcb)
{
	_mutex->_lock_nesting = 1;
	_mutex->_owner_prio = _task_tcb->_task_priority;
	_mutex->_mutex_owner = _task_tcb;
}

/*
 *@func: 检测锁是否存在拥有者(进程) 
 */
__os_inline_ bool os_mutex_is_owned(struct os_mutex* _mutex)
{
	return (NULL != _mutex->_mutex_owner);
}

/*
 *@func: 检测锁的拥有者是否是当前进程
 */
__os_inline_ bool os_mutex_is_self(struct os_mutex* _mutex)
{
	return (_mutex->_mutex_owner == os_task_current);
}

/*
 *@func: 检测锁是否允许递归
 */
__os_inline_ bool os_mutex_is_recursive(struct os_mutex* _mutex)
{
	return (_mutex->_mutex_type == OS_MUTEX_RECURSIVE);
}

/*
 *@func: 释放锁拥有者
 */
__os_prv_ __os_inline_ void __os_mutex_owner_release(struct os_mutex* _mutex)
{
	_mutex->_mutex_owner->_task_priority = _mutex->_owner_prio;
	_mutex->_mutex_owner = NULL;
}

/*
 *@func: 锁链表是否存在进程
 */
__os_inline_ bool os_mutex_block_is_empty(struct os_mutex* _mutex)
{
	return (list_empty(&_mutex->_block_obj._list));
}


/*
*@func: 尝试将当前进程加锁, 如果该锁被其他进程所拥有则返回 OS_MUTEX_HANDLE_OTHER_OWNER
*/
os_mutex_handle_state_t os_mutex_try_lock(struct os_mutex* _mutex)
{
	if (NULL == _mutex)
		return OS_MUTEX_HANDLE_LOCK_FAIL;
	unsigned int _critical_state = os_port_enter_critical();
	// 如果锁不存在拥有者，则将当前运行的进程置为该锁的拥有者
	if (!os_mutex_is_owned(_mutex))
	{
		__mutex_owner_change(_mutex, os_task_current);
		os_port_exit_critical(_critical_state);
		return OS_MUTEX_HANDLE_GET_OWNER;
	}
	if (os_mutex_is_self(_mutex) && 
		os_mutex_is_recursive(_mutex))
	{
		if (_mutex->_lock_nesting < OS_MUTEX_MAX_RECURSIVE)
		{
			_mutex->_lock_nesting++;
			os_port_exit_critical(_critical_state);
			return OS_MUTEX_HANDLE_RECURSIVE_SUECCESS;
		}
		os_port_exit_critical(_critical_state);
		return OS_MUTEX_HANDLE_RECURSIVE_FAIL;
	}
	
	os_port_exit_critical(_critical_state);
	return OS_MUTEX_HANDLE_OTHER_OWNER;
}

/*
 *@func: 将当前进程上锁
 */
os_mutex_handle_state_t os_mutex_lock(struct os_mutex* _mutex)
{
	unsigned int _critical_state = os_port_enter_critical();
	
	os_mutex_handle_state_t _mutex_handle_state;
	_mutex_handle_state = os_mutex_try_lock(_mutex);
	if (_mutex_handle_state == OS_MUTEX_HANDLE_OTHER_OWNER)
	{
		// 锁已被其他进程锁占用
		// 判断锁的进程优先级是否低于当前进程的优先级，
		// 如果小于则改变锁的优先级，以防止优先级反转
		if (_mutex->_owner_prio > os_task_current->_task_priority)
			__mutex_owner_prio_up(_mutex, os_task_current->_task_priority);
		// 将当前进程挂起
		os_add_block_task(os_task_current, &_mutex->_block_obj, OS_BLOCK_NEVER_TIME_OUT);
		os_port_exit_critical(_critical_state);		
		// 展开调度
		__os_sched();
		
		/* 当程序被切回来的时候，将该进程置为锁的拥有者进程 */
		_critical_state = os_port_enter_critical();
		// 该处是为了给PendSV异常被触发前留足时间
		while(os_task_is_block(os_task_current)){};
		__mutex_owner_change(_mutex, os_task_current);
		os_port_exit_critical(_critical_state);
	}
	
	os_port_exit_critical(_critical_state);
	return _mutex_handle_state;
}

/*
 *@func: 将当前进程解锁
 */
os_mutex_handle_state_t os_mutex_unlock(struct os_mutex* _mutex)
{
	if (NULL == _mutex)
		return OS_MUTEX_HANDLE_UNLOCK_FAIL;
	unsigned int _critical_state = os_port_enter_critical();
	
	// 如果锁不为自己所有, 则返回
	if (!os_mutex_is_self(_mutex))
	{
		os_port_exit_critical(_critical_state);
		return OS_MUTEX_HANDLE_UNLOCK_FAIL;
	}
	
	if (_mutex->_mutex_type == OS_MUTEX_RECURSIVE && --_mutex->_lock_nesting > 0)
	{
		os_port_exit_critical(_critical_state);
		return OS_MUTEX_HANDLE_UNLOCK_SUCCESS;
	}
	
	__os_mutex_owner_release(_mutex);
	
	// 查看锁链表上是否存在进程
	if (os_mutex_block_is_empty(_mutex))
	{
		os_port_exit_critical(_critical_state);
		return OS_MUTEX_HANDLE_UNLOCK_SUCCESS;
	}
	// 唤醒锁阻塞队列中的第一个进程
	os_block_wakeup_first_task(&_mutex->_block_obj);
	os_port_exit_critical(_critical_state);
	// 调度
	__os_sched();
	return OS_MUTEX_HANDLE_UNLOCK_SUCCESS;
}

/*
 *@func: 销毁锁
 */
os_mutex_handle_state_t os_mutex_destory(struct os_mutex* _mutex)
{
	if (NULL == _mutex)
		return OS_MUTEX_HANDLE_DESTORY_FAIL;
	unsigned int _critical_state = os_port_enter_critical();
	
	// 如果该锁被进程所拥有则释放锁主
	if (os_mutex_is_owned(_mutex))
		__os_mutex_owner_release(_mutex);
	
	// 释放锁阻塞队列的所有进程
	if (!os_mutex_block_is_empty(_mutex))
		os_block_wakeup_all_task(&_mutex->_block_obj);
	os_block_deinit(&_mutex->_block_obj);
	os_port_exit_critical(_critical_state);
	// 调度
	__os_sched();
	return OS_MUTEX_HANDLE_DESTORY_SUCCESS;
}

/*
 *@func: 初始化锁
 */
os_mutex_handle_state_t os_mutex_init(struct os_mutex* _mutex, os_mutex_type_t _type)
{
	if (NULL == _mutex)
		return OS_MUTEX_HANDLE_INIT_FAIL;
	_mutex->_mutex_owner = NULL;
	_mutex->_mutex_type = _type;
	_mutex->_lock_nesting = 0;
	_mutex->_owner_prio = OS_MUTEX_PRIO_LOWEST;
	os_block_init(&_mutex->_block_obj, OS_BLOCK_MUTEX);
	return OS_MUTEX_HANDLE_INIT_SUCCESS;
}



