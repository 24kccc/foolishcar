; ***********************
; *@file: os_port_s.s
; *@author: Feijie Luo
; *@data: 2022-9
; *@compiling envir.: armcc 
; ***********************


	
SCB_SHPR3_PRI13		EQU	0xE000ED20		; focus on priority14 reg.
SCB_PENDSV_PRIORITY	EQU 0x00FF0000		; pendsv priority
SCB_ICSR			EQU 0xE000ED04		; interrupt control and state reg adr
SCB_ICSR_PENDSV_SET	EQU	0x10000000 		; trigger pendsv interrupt
	PRESERVE8
	AREA |.text1|, CODE, READONLY
	THUMB
	;REQUIRE8
	;
	
	IMPORT enter_critical_num
	IMPORT os_task_current
	IMPORT os_task_ready
	
	EXPORT sf_os_start
	EXPORT sf_os_ctx_sw
	EXPORT PendSV_Handler
		
	IMPORT os_cpu_running_flag;
	
sf_os_start
	CPSID I								; disable global interrupt
	PUSH {R4, R5}						; note: arm:FD(full decrease), R5 in higher address
	LDR R4, =SCB_SHPR3_PRI13
	LDR R5, =SCB_PENDSV_PRIORITY
	STR R5, [R4]						; set pendsv priority
	
	MOVS R4, #0
	MSR	PSP, R4							; reset psp
	
	LDR R4, =os_cpu_running_flag;
	MOV R5, #1
	STR R5, [R4];
	
	LDR R4, =SCB_ICSR
	LDR R5, =SCB_ICSR_PENDSV_SET
	STR R5, [R4]						; trigger pendsv exception
	POP {R4, R5}						; load R4 from lower address
	CPSIE I								; enable global interrupt
	;;; note: os has its own main loop
sf_os_while
	B sf_os_while
	
sf_os_enter_critical
	CPSID I								; diable global interrupt
	;;; push R4 and R5 before using
	PUSH {R4, R5}
	LDR R4, =enter_critical_num
	LDR R5, [R4]
	ADD R5, R5, #1						; enter_critical_num++
	STR	R5, [R4]
	POP {R4, R5}
	BX LR
	
sf_os_exit_critical
	PUSH {R4, R5}
	LDR R4, =enter_critical_num
	LDR R5, [R4]
	SUB R5, R5, #1
	STR R5, [R4]
	;;; now, if R5 == 0, enable global interrupt
	MOV R4, #0
	CMP R5, #0
	MSREQ PRIMASK, R4
	POP {R4, R5}
	BX LR

sf_os_ctx_sw
	PUSH {R4, R5}
	LDR R4, =SCB_ICSR
	LDR R5, =SCB_ICSR_PENDSV_SET
	STR R5, [R4]						; trigger pendsv exception
	POP {R4, R5}
	BX LR
	
PendSV_Handler
	CPSID I
	MRS R0, PSP
    ; CBZ R0, switch_process				; regard psp = 0x000 as a flag of switching process
	CMP R0, #0
	BEQ switch_process
	
	;;; why R0 -= 0x20: when system handles PendSV_Handler,
	;;; R0-R3, R12, R14, R15(PC) and xPSP will be pushed by system automatically.
	;;; At this point, the PSP points to [R11] of stack, See os_process_stack_init function in os_port_c.c
	SUBS R0, R0, #0x20
	STM R0, {R4-R11}						; PUSH R4-R11, now R0 points to [R4] of stack
	
	LDR R1, =os_task_current
	LDR R1, [R1]
	STR R0, [R1]							; update _stack_top of os_task_current
											; and then execute switch_process
	
switch_process
	LDR R0, =os_task_current
	LDR R1, =os_task_ready
	LDR R2, [R1]						; R2 = os_task_ready
	STR R2, [R0]						; os_task_current = os_task_ready
	
	LDR R0, [R2]						; see [struct task_control_block], R0 = _stack_top
	
	LDMIA R0!, {R4-R11}					; restore R4-R11 from new process stack, note: _stack_top must point to [full]
										; the rest reg value will be poped by system automatically
	
	MSR PSP, R0							; update PSP
	ORR LR, LR, #0x04					; Return to Thread mode, exception return uses non-floating-point state from
										; the PSP and execution uses PSP after return.
	CPSIE I
	BX LR
	
	END