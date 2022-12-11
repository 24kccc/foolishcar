/*
 *  [For SEU smart car race 2022]
 *  @File: gui.h
 *  @Author: Feijie Luo
 *  @Last edit time: 2022-9-5
 *
 */
#ifndef _GUI_H_
#define _GUI_H_
#include "stdio.h"
#include "string.h"
#include "headfile.h"
#include <stdlib.h>
#include "soft_timer.h"


#ifndef NULL
#define NULL 0
#endif

#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/*
 * @Note: 一个参数存储需要 8 个字节的flash内存，尽量避免参数过多。建议 < 200
 */
#define VAR_DATA_NUM 50

#define GUI_WIDTH TFT_X_MAX
#define GUI_HEIGHT TFT_Y_MAX

/*
 * @Note: 针对driver ic为ST7735的TFT屏：关于颜色定义，逐飞的库使用的是
 *        [8-bit data bus for 16-bit/pixel (RGB 5-6-5-bit input), 65K-Colors]
 *        即16位前5位为R,中间6位为G，后5位为B。详细查看 ST7735手册
 */
typedef unsigned short GUI_COLOR_TYPE;
#define GUI_PEN_COLOR 0xFFFF
#define GUI_BGD_COLOR 0x0000

/***************** 以下内部调用 *****************/
#if (VAR_DATA_NUM > 512)
#error VAR_DATA_NUM cannot be greater than 512.
#endif

#define GUI_PAGE_LINE 10
#define GUI_PAGE_CAP 16

#define INT_ROUND(_var_mol, _var_den) (((_var_mol) + (_var_den)-1) / (_var_den))

#define CURSOR_NO_SELECT_FLAG (0xFFFFFFFFU)

typedef struct cursor_pos
{
    uint32_t _parent;
    uint32_t _sub;
    uint32_t _atomic;
} cursor_pos_t;

extern cursor_pos_t cursor_position;

//void disable_glo_iqr(void);
//void enable_glo_iqr(void);
void gui_init(void);
void gui_refresh(char _nec_data);
void gui_motor_control(char _nec_data);
// void lcd_show_string(uint16_t _x, uint16_t _y, const char *_dat, uint8_t _size);
void refresh_sel_disp(void);
void eru_triggered(void);
void trigger_rec_process(const unsigned char *_rec_bytes);
char nec_data_decode(const uint8_t _dat1, const uint8_t _dat2);
void write_data_to_flash();
void read_data_from_flash();
void var_page_init();
void var_data_to_usr_data(void);
double page_var_to_float(const char *_str);
void img_view_control(void *_argv);
void var_str_to_var_data(const uint32_t _page_var_index);
void write_data_to_page_str();
void page_str_to_uint32(uint32_t *_uint32_arr, const uint32_t _len);
void flash_data_to_var_str(const uint32_t *_uint32_arr, const uint32_t _len);

void draw_rect(const uint16_t _x, const uint16_t _y, const uint16_t _width, const uint16_t _height, const GUI_COLOR_TYPE _color);
void gui_redraw(void);
void gui_insert_char(uint16_t _x, uint16_t _y, const unsigned char _dat, const GUI_COLOR_TYPE _font_color, const GUI_COLOR_TYPE _bkg_color, bool _is_clr_font_bgd);
void gui_insert_point(uint16_t _x, uint16_t _y, const GUI_COLOR_TYPE _color);
void gui_insert_string(uint16_t _x, uint16_t _y, const char *_dat, const uint8_t _size, const GUI_COLOR_TYPE _font_color, const GUI_COLOR_TYPE _bkg_color, bool _is_clr_font_bgd);
extern int Kp ;//
extern int Ki;//
extern int Kd;//
extern int BasicSpeed;  //基础速度
extern int SpeedLimmit; //最大速度
extern float Decrease;  //积分项衰减
extern float LookAt;    //看的位置
extern int camera;     //是否要摄像头
extern int Rout;
extern int Lout;
extern int Rin;
extern int Lin;
extern int Tout;
extern int Tin;
extern int Correct;     //摄像头水平矫正
extern int BrakePower;  //刹车力度
extern int BrakeTime;   //刹车时间
extern int TurnSlow;
extern int go_1_T;      //环岛前
extern int go_2_T;      //环岛后
extern int turn_T;      //环岛中
extern int go_1_l;
extern int go_1_r;
extern int turn_l ;
extern int turn_r ;
extern int go_2_l;
extern int go_2_r;


#endif
