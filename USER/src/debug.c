
#include "debug.h"
#include "fsl_lpuart.h"

float variable[16]={0};

/*
 * 取一个数据的各个位
 */
#define BYTE0(dat)       (*(char*)(&dat))
#define BYTE1(dat)       (*((char*)(&dat) + 1))
#define BYTE2(dat)       (*((char*)(&dat) + 2))
#define BYTE3(dat)       (*((char*)(&dat) + 3))

/* 重定向printf */
int fputc(int ch, FILE *f)
{
	/* 堵塞判断串口是否发送完成 */
	while (!(LPUART3->STAT & LPUART_STAT_TDRE_MASK));
	LPUART3->DATA = ch;
	return ch;
}

void __uart_putchar(char _dat)
{
	while (!(LPUART3->STAT & LPUART_STAT_TDRE_MASK));
	LPUART3->DATA = _dat;
}

//设置需要发送的参数  可以设16个
static void __monitor_var_update(void)
{
	variable[0] = 0;
	variable[1] = 0;
	variable[2] = 0;
	variable[3] = 0;
	variable[4] = 0;
	variable[5] = 0;
	variable[6] = 12.4;
	variable[7] = 0;
	variable[8] = 0;
	variable[9] = 0;
	variable[10] = 0;
	variable[11] = 0;
	variable[12] = 0;
	variable[13] = 0;
	variable[14] = 0;
	variable[15] = 0;
}

static void __monitor_send_var(void)
{
	unsigned char ch = 0;
	float temp = 0;
	static unsigned char variable_num = 16;
	__uart_putchar((char)85);		// 协议头_1
	__uart_putchar((char)170);		// 协议头_2
	__uart_putchar((char)255);		// 协议头_3
	__uart_putchar((char)1);		// 协议头_4
	__uart_putchar(variable_num);	// 发送出的变量个数
	for(unsigned char i = 0; i < variable_num; ++i)
	{
		temp = variable[i];
		ch = BYTE0(temp);
		__uart_putchar(ch);
		ch = BYTE1(temp);
		__uart_putchar(ch);
		ch = BYTE2(temp);
		__uart_putchar(ch);
		ch = BYTE3(temp);
		__uart_putchar(ch);
	}
	__uart_putchar((char)1);	     // 协议尾
}

/* 用来通知上位机新的一组数据开始，要保存数据必须发送它 */
static void __monitor_send_begin()
{
	__uart_putchar(0x55);
	__uart_putchar(0xaa);
	__uart_putchar(0x11);
}

/* 名优科创上位机 */
void supermonitor()
{
	__monitor_var_update();
	__monitor_send_var();
	__monitor_send_begin();
}
