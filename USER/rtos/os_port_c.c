/***********************
 *@file: os_port_c.c
 *@author: Feijie Luo
 *@data: 2022-9
 *@note: 
 ***********************/


#include <stdio.h>
#include "os_port_c.h"
#include "os_core.h"

unsigned int enter_critical_num;


unsigned int* os_process_stack_init(void* _fn_entry, 
									void* _arg, 
									void* _exit, 
									STACK_TYPE* _stack_addr, 
									unsigned int _stack_size)
{
	// locate end of _stack_addr
	unsigned int* _tmp_stack_addr = _stack_addr + _stack_size - 1;
	// note: 8-byte align		AAPCS  We must ensure that 8-byte alignment is maintained in C
	_tmp_stack_addr = (STACK_TYPE*)((STACK_TYPE)(_tmp_stack_addr) & 0xFFFFFFF8UL);
	
    *(--_tmp_stack_addr) = (unsigned int)0x01000000UL;                          //xPSR
    *(--_tmp_stack_addr) = (unsigned int)_fn_entry;                             // Entry Point(PC)
    *(--_tmp_stack_addr) = (unsigned int)_exit;                                 // R14 (LR)
    *(--_tmp_stack_addr) = (unsigned int)0x12121212UL;                          // R12
    *(--_tmp_stack_addr) = (unsigned int)0x03030303UL;                          // R3
    *(--_tmp_stack_addr) = (unsigned int)0x02020202UL;                          // R2
    *(--_tmp_stack_addr) = (unsigned int)0x01010101UL;                          // R1
    *(--_tmp_stack_addr) = (unsigned int)_arg;                                  // R0
	// the upper reg will be poped by systemp automatically
	
	// the following reg will be poped by switch_process.
    *(--_tmp_stack_addr) = (unsigned int)0x11111111UL;                          // R11
    *(--_tmp_stack_addr) = (unsigned int)0x10101010UL;                          // R10
    *(--_tmp_stack_addr) = (unsigned int)0x09090909UL;                          // R9
    *(--_tmp_stack_addr) = (unsigned int)0x08080808UL;                          // R8
    *(--_tmp_stack_addr) = (unsigned int)0x07070707UL;                          // R7
    *(--_tmp_stack_addr) = (unsigned int)0x06060606UL;                          // R6
    *(--_tmp_stack_addr) = (unsigned int)0x05050505UL;                          // R5
    *(--_tmp_stack_addr) = (unsigned int)0x04040404UL;                          // R4
	return _tmp_stack_addr;
}

void os_port_cpu_int_disable(void)
{
    __asm volatile ("CPSID I" : : : "memory");
}


void os_port_cpu_int_enable(void)
{
    __asm volatile ("CPSIE I" : : : "memory");
}

unsigned int os_port_cpu_primask_get(void)
{
	unsigned int _ret;

	__asm volatile ("MRS %0, PRIMASK" : "=r" (_ret));

	return(_ret);
}

void os_port_cpu_primask_set(unsigned int _primask)
{
	__asm volatile ("MSR PRIMASK, %0" : : "r" (_primask) : "memory");
}

/* 进入临界区 */
unsigned int os_port_enter_critical(void)
{
	unsigned int _ret = os_port_cpu_primask_get();
	
	os_port_cpu_int_disable();
	
	return _ret;
} 

/* 退出临界区 */
void os_port_exit_critical(unsigned int _state)
{
	os_port_cpu_primask_set(_state);
}

