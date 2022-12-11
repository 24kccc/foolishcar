
#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdio.h>
/*
 * IAR重写的stdio中不包含__FILE结构体
 * @请各位参赛队伍人员在不使用本工程的条件下，
 *  检查Project -> Options -> General Options -> Library Configuration的Library是否为Full
 *  提醒：如果找不到该选项，请先将工程目录的[RT106x - nor_sdram_zf_dtcm]选中再进行上述操作
 */

int fputc(int ch, FILE *f);
void __uart_putchar(char _dat);
static void __monitor_var_update(void);
static void __monitor_send_var(void);
static void __monitor_send_begin();
void supermonitor();
#endif
