
#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdio.h>
/*
 * IAR��д��stdio�в�����__FILE�ṹ��
 * @���λ����������Ա�ڲ�ʹ�ñ����̵������£�
 *  ���Project -> Options -> General Options -> Library Configuration��Library�Ƿ�ΪFull
 *  ���ѣ�����Ҳ�����ѡ����Ƚ�����Ŀ¼��[RT106x - nor_sdram_zf_dtcm]ѡ���ٽ�����������
 */

int fputc(int ch, FILE *f);
void __uart_putchar(char _dat);
static void __monitor_var_update(void);
static void __monitor_send_var(void);
static void __monitor_send_begin();
void supermonitor();
#endif
