/***********************
 *@file: os_cofig.h
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/

#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_

// 可支持的最高优先级
#define OS_TASK_MAX_PRIORITY	(32)
// 可支持的最大id值
#define OS_TASK_MAX_ID			(64)
// 时间片
#define OS_TIMESLICE_STD		(10)

#define OS_READY_LIST_SIZE		((OS_TASK_MAX_PRIORITY) + 1)
#define OS_TASK_MAX_ID_SIZE		((OS_TASK_MAX_ID) + 1)

#if (OS_TASK_MAX_PRIORITY > 255 || OS_TASK_MAX_PRIORITY < 0)
	#error "OS_TASK_MAX_PRIORITY should be maintained between 0 and 255."
#endif

#ifndef bool
	#define bool unsigned char
#endif

#ifndef NULL
	#define NULL 0
#endif

#ifndef true
	#define true 1
#endif
	
#ifndef false
	#define false 0
#endif
	
#define __os_inline_ inline
#define __os_prv_	static

typedef enum os_handle_state
{
	OS_HANDLE_FAIL = 0,
	OS_HANDLE_SUCCESS = 1
}os_handle_state_t;

#endif