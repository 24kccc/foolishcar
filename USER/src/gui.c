/*
 *  [For SEU smart car race 2022]
 *  @File: gui.c
 *  @Author: Feijie Luo
 *  @Last edit time: 2022-9-9
 *
 */

/*
 * 调参使用步骤：第一步，根据所需修改参数个数变量 VAR_DATA_NUM
 *               第二步，在外设初始化阶段调用gui_init();
 *               第三步，根据自己所需修改var_page_init()函数中的参数名称
 *               第四步，根据自己所需在var_data_to_usr_data()函数中获取参数的值
 */

#include "gui.h"

// 为使 gui 显示更为顺畅，这里引入缓存机制
// 画幅.
GUI_COLOR_TYPE gui_paint[GUI_HEIGHT][GUI_WIDTH];

// 引导页
static char page_guide[1][GUI_PAGE_LINE][GUI_PAGE_CAP];
static uint16_t page_guide_cap = 0;
// 存储每一页的字符显示信息
static char page_var_str[INT_ROUND(VAR_DATA_NUM, GUI_PAGE_LINE - 1)][GUI_PAGE_LINE][GUI_PAGE_CAP];
// uint16_t page_var_cap[INT_ROUND(VAR_DATA_NUM, GUI_PAGE_LINE - 1)] = {0};
// 光标所在位置
cursor_pos_t cursor_position = {._parent = 1, ._sub = CURSOR_NO_SELECT_FLAG, ._atomic = CURSOR_NO_SELECT_FLAG};
// 参数变量
float var_data[VAR_DATA_NUM];//VAR_DATA_NUM在gui.h里定义的是50

int Kp;//
int Ki;//
int Kd;//
int BasicSpeed;//
int SpeedLimmit;
float Decrease;//
float LookAt;//
int camera;
int Rout;
int Lout;
int Rin;
int Lin;
int Tout;
int Tin;
int Correct;
int BrakePower;
int BrakeTime;
int TurnSlow;
int go_1_T;
int turn_T ;
int go_2_T ;
int go_1_l;
int go_1_r;
int turn_l ;
int turn_r ;
int go_2_l;
int go_2_r;

void var_page_init()
{
    // 包含从 flash 到全局var
    read_data_from_flash();
    var_data_to_usr_data();

    /* 参数名修改区 */
    // 第一页
    memcpy(page_var_str[0][1], "Kp    :", 8);
    memcpy(page_var_str[0][2], "Ki    :", 8);
    memcpy(page_var_str[0][3], "Kd    :", 8);
    memcpy(page_var_str[0][4], "BasSp  :", 8);
    memcpy(page_var_str[0][5], "Decrease  :", 8);
    memcpy(page_var_str[0][6], "LookAt  :", 8);
    memcpy(page_var_str[0][7], "SpLim  :", 8);
    memcpy(page_var_str[0][8], "Correct  :", 8);
    memcpy(page_var_str[0][9], "TurnSlow  :", 8);

    // 第二页
    memcpy(page_var_str[1][1], "camera :", 8);
    memcpy(page_var_str[1][2], "Rout :", 8);
    memcpy(page_var_str[1][3], "Lout :", 8);
    memcpy(page_var_str[1][4], "Tout :", 8);
    memcpy(page_var_str[1][5], "Rin :", 8);
    memcpy(page_var_str[1][6], "Lin :", 8);
    memcpy(page_var_str[1][7], "Tin :", 8);
    memcpy(page_var_str[1][8], "BrakeP :", 8);
    memcpy(page_var_str[1][9], "BrakeT :", 8);
    
  // 第三页
    memcpy(page_var_str[2][1], "go1 :", 8);
    memcpy(page_var_str[2][2], "turn :", 8);
    memcpy(page_var_str[2][3], "go2 :", 8);
    memcpy(page_var_str[2][4], " go_1_l:", 8);
    memcpy(page_var_str[2][5], " go_1_r:", 8);
    memcpy(page_var_str[2][6], " turn_l:", 8);
    memcpy(page_var_str[2][7], " turn_r:", 8);
    memcpy(page_var_str[2][8], " go_2_l:", 8);
    memcpy(page_var_str[2][9], " go_2_r:", 8);

}

void var_data_to_usr_data(void)
{
  
  Kp = var_data[0];
  Ki = var_data[1];
  Kd = var_data[2];
  BasicSpeed = var_data[3];
  Decrease = var_data[4];
  LookAt = var_data[5];
  SpeedLimmit = var_data[6];
  Correct = var_data[7];
  TurnSlow = var_data[8];
  

  camera = var_data[9];
  Rout = var_data[10];
  Lout = var_data[11];
  Tout = var_data[12];
  Rin = var_data[13];
  Lin = var_data[14];
  Tin = var_data[15];
  BrakePower = var_data[16];
  BrakeTime = var_data[17];
  
  
  go_1_T = var_data[18];
  turn_T = var_data[19];
  go_2_T = var_data[20];
  go_1_l = var_data[21];
  go_1_r = var_data[22];
  turn_l = var_data[23] ;
  turn_r = var_data[24] ;
  go_2_l = var_data[25];
  go_2_r = var_data[26];
    
    
    /*
     *   [参数自取区]
     * @example: usr_var_0 = var_data[0];
     *           usr_var_1 = var_data[1]; ...
     * @note: 该函数在初始化阶段会被调用一次，此外每键入一个参数也会被调用一次
     */
  
}

/*
 * Control Mode
 */
void gui_motor_control(char _nec_data)
{
	static uint32_t right_pwm = 0;
	static uint32_t left_pwm = 0;
	static uint32_t servo_pwm = 5750;
	switch(_nec_data)
	{
		// 向上方位键
		case 'u':
			right_pwm += 200;
			left_pwm += 200;
			break;
		// 向左方位键
		case 'l':
			servo_pwm = servo_pwm < 6350 ? (servo_pwm + 100) : 6450;
			break;
		// 向右方位键
		case 'r':
			servo_pwm = servo_pwm > 5100 ? (servo_pwm - 100) : 5000;
			break;
		// 向下方位键
		case 'b':
			right_pwm = right_pwm > 200 ? (right_pwm - 200) : 0;
			left_pwm = left_pwm > 200 ? (left_pwm - 200) : 0;
			break;
		// ok键暂停
		case 'o':
			right_pwm = 0;
			left_pwm = 0;
			break;
		default:
			break;
	}
	pwm_init(PWM1_MODULE3_CHB_D1, 800, right_pwm); // 面向车头右轮前进
	pwm_init(PWM2_MODULE3_CHA_D2, 800, left_pwm); // 左轮前进
	pwm_duty(PWM4_MODULE3_CHA_C31, servo_pwm);
}

// 初始化屏幕
void gui_init(void)
{
    lcd_init();
    const unsigned int _tmp_num_page_var = INT_ROUND(VAR_DATA_NUM, GUI_PAGE_LINE - 1);
    memset((char *)page_guide, ' ', sizeof(char) * 1 * GUI_PAGE_LINE * GUI_PAGE_CAP);
    memset((char *)page_var_str, ' ', sizeof(char) * _tmp_num_page_var * GUI_PAGE_LINE * GUI_PAGE_CAP);
    memcpy((char *)page_guide, "|SEU  SMART CAR|", 16);
    memcpy((char *)((unsigned int)page_guide + GUI_PAGE_CAP * 1), "   Set Param    ", 16);
    memcpy((char *)((unsigned int)page_guide + GUI_PAGE_CAP * 2), "  Control Mode  ", 16);
    memcpy((char *)((unsigned int)page_guide + GUI_PAGE_CAP * 3), "   Image View   ", 16);
    memcpy((char *)((unsigned int)page_guide + GUI_PAGE_CAP * 4), "   test   ", 16);
    page_guide_cap = 4;

    char _tmp_title_str[] = "|    PAGE      |";
    for (unsigned int _i = 0; _i < _tmp_num_page_var; ++_i)
    {
        char _tmp_page_num[16];
        sprintf(_tmp_page_num, "%u", _i);
        assert(strlen(_tmp_page_num) <= 4);
        memcpy((char *)((unsigned int)_tmp_title_str + 10), _tmp_page_num, strlen(_tmp_page_num));
        memcpy((char *)((unsigned int)page_var_str + GUI_PAGE_LINE * GUI_PAGE_CAP * _i), _tmp_title_str, strlen(_tmp_title_str));
        // if (_i < _tmp_num_page_var - 1)
        //     page_var_cap[_i] = 9;
        // else
        //     page_var_cap[_i] = VAR_DATA_NUM % GUI_PAGE_LINE;
    }
    var_page_init();
    gui_refresh(0);
    // 开启中断
    gpio_interrupt_init(D26, FALLING, GPIO_INT_CONFIG);
    NVIC_SetPriority(GPIO3_Combined_16_31_IRQn, 15);
}

extern const uint8_t tft_ascii[95][16];
void gui_insert_char(uint16_t _x, uint16_t _y, const unsigned char _dat, const GUI_COLOR_TYPE _font_color, const GUI_COLOR_TYPE _bkg_color, bool _is_clr_font_bgd)
{
    // 这里使用逐飞提供的字库 tft_ascii, 逐行扫描
    // 字库字高16bit，字宽8bit
    uint8_t _font_tmp;
    for (uint16_t _i = 0; _i < 16; ++_i)
    {
        _font_tmp = tft_ascii[_dat - 32][_i];
        for (uint16_t _j = 0; _j < 8; ++_j)
        {
            if (_font_tmp & 0x01)
                *((GUI_COLOR_TYPE *)gui_paint + (_y + _i) * GUI_WIDTH + _x + _j) = _font_color;
            else
            {
                if (_is_clr_font_bgd)
                    *((GUI_COLOR_TYPE *)gui_paint + (_y + _i) * GUI_WIDTH + _x + _j) = _bkg_color;
            }
            _font_tmp >>= 1;
        }
    }
}

void gui_insert_point(uint16_t _x, uint16_t _y, const GUI_COLOR_TYPE _color)
{
    *((GUI_COLOR_TYPE *)gui_paint + _y * GUI_WIDTH + _x) = _color;
}

void gui_insert_string(uint16_t _x, uint16_t _y, const char *_dat, const uint8_t _size, const GUI_COLOR_TYPE _font_color, const GUI_COLOR_TYPE _bkg_color, bool _is_clr_font_bgd)
{
    uint16_t _j = 0;
    while (_j < _size)
    {
        gui_insert_char(_x + 8 * _j, _y * 16, _dat[_j], GUI_PEN_COLOR, GUI_BGD_COLOR, true);
        ++_j;
    }
}

// static void lcd_show_string(uint16_t _x, uint16_t _y, const char *_dat, uint8_t _size)
// {
//     uint16_t _j = 0;
//     while (_j < _size)
//     {
//         lcd_showchar(_x + 8 * _j, _y * 16, _dat[_j]);
//         ++_j;
//     }
// }

// 刷新屏幕
void gui_refresh(char _nec_data)
{
	// 主页
    if (CURSOR_NO_SELECT_FLAG == cursor_position._sub)
    {
        for (uint8_t _i = 0; _i < GUI_PAGE_LINE; ++_i)
        {
            gui_insert_string(0, _i, (char *)page_guide[0][_i], 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
        }
		refresh_sel_disp();
    }
	// 参数页: Set Param
    else if (1 == cursor_position._parent && CURSOR_NO_SELECT_FLAG != cursor_position._sub)
    {
        for (uint8_t _i = 0; _i < GUI_PAGE_LINE; ++_i)
        {
            gui_insert_string(0, _i, (char *)page_var_str[cursor_position._sub / GUI_PAGE_LINE][_i], 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
        }
		refresh_sel_disp();
    }
	// Control Mode
	if (2 == cursor_position._parent && CURSOR_NO_SELECT_FLAG != cursor_position._sub)
	{
		gui_insert_string(0, 0, "|REMOTE CONTROL|", 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
		gui_insert_string(0, 1, "                ", 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
		gui_insert_string(0, 2, "                ", 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
		gui_insert_string(0, 3, "       ^        ", 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
		gui_insert_string(0, 4, "       |        ", 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
		gui_insert_string(0, 5, "    <-   ->     ", 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
		gui_insert_string(0, 6, "       |        ", 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
		gui_insert_string(0, 7, "       v        ", 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
		gui_insert_string(0, 8, "                ", 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
		gui_insert_string(0, 9, "                ", 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
		gui_motor_control(_nec_data);
	}
	// Image View
	if (3 == cursor_position._parent && CURSOR_NO_SELECT_FLAG != cursor_position._sub)
	{
	}
	
    //test
    if (4 == cursor_position._parent && CURSOR_NO_SELECT_FLAG != cursor_position._sub)
	{
          //gui_insert_string(0, 1, (char)qtimer_quad_get(QTIMER_1,QTIMER1_TIMER0_C0), 16, GUI_PEN_COLOR, GUI_BGD_COLOR, true);
          
	}
    gui_redraw();
}

void gui_redraw(void)
{
    for (uint16_t _i = 0; _i < GUI_HEIGHT; ++_i)
    {
        lcd_set_region(0, _i, GUI_WIDTH - 1, _i);
        for (uint16_t _j = 0; _j < GUI_WIDTH; ++_j)
        {
            lcd_writedata_16bit(*((GUI_COLOR_TYPE *)gui_paint + _i * GUI_WIDTH + _j));
        }
    }
}

void draw_rect(const uint16_t _x, const uint16_t _y, const uint16_t _width, const uint16_t _height, const GUI_COLOR_TYPE _color)
{
    for (uint16 _i = 0; _i < _width; ++_i)
    {
        gui_insert_point(_x + _i, _y + 2, _color);
        gui_insert_point(_x + _i, _y + _height - 2, _color);
    }
    for (uint16_t _i = 2; _i < _height - 2; ++_i)
    {
        gui_insert_point(_x, _y + _i, _color);
        gui_insert_point(_x + _width, _y + _i, _color);
    }
}

void refresh_sel_disp(void)
{
    if (CURSOR_NO_SELECT_FLAG == cursor_position._atomic)
    {
        if (CURSOR_NO_SELECT_FLAG == cursor_position._sub)
        {
            draw_rect(1, cursor_position._parent * 16, TFT_X_MAX - 4, 16, BLUE);
        }
        else
        {
            draw_rect(1, (cursor_position._sub % 10) * 16, TFT_X_MAX - 4, 16, BLUE);
        }
    }
    else
    {
        if (1 == cursor_position._parent)
        {
            draw_rect(cursor_position._atomic * 8, (cursor_position._sub % 10) * 16, 7, 16, RED);
        }
    }
}

void eru_triggered(void)
{
    // 引脚中断禁用
    GPIO_DisableInterrupts(GPIO3, 26 << 1);
    //disable_glo_iqr();
    soft_timer_time_t _start_time = soft_timer_get_time();
    while (!gpio_get(D26))
    {
    }
    unsigned int _tmp_time = soft_timer_get_2_time_diff_us(soft_timer_get_time(), _start_time);
    // 引导码的低电平持续时间需9ms
    if (_tmp_time < 8000 || _tmp_time > 10000)
    {
        // 引脚中断使能
        GPIO_EnableInterrupts(GPIO3, 26 << 1);
        return;
    }
    while (gpio_get(D26))
    {
    }
    _tmp_time = soft_timer_get_2_time_diff_us(soft_timer_get_time(), _start_time);
    // 引导码的高电平持续时间需4.5ms
    if (_tmp_time > 12000 && _tmp_time < 15000)
    {
        unsigned char bytes[4] = {0};
        unsigned int _tmp_time_low = 0;
        soft_timer_time_t _tmp_low_time;
        unsigned int _tmp_time_high = 0;
        soft_timer_time_t _tmp_time_std;
        int _count = 0;
        for (uint8_t _i = 0; _i < 4; ++_i)
        {
            for (uint8_t _j = 0; _j < 8; ++_j)
            {
                _tmp_time_std = soft_timer_get_time();
                while (!gpio_get(D26))
                {
                }
                _tmp_low_time = soft_timer_get_time();
                _tmp_time_low = soft_timer_get_2_time_diff_us(_tmp_low_time, _tmp_time_std);
                while (gpio_get(D26))
                {
                }
                _tmp_time_high = soft_timer_get_2_time_diff_us(soft_timer_get_time(), _tmp_low_time);
                if (_tmp_time_low < 300 || _tmp_time_low > 700)
                {
                    // 引脚中断使能
                    GPIO_EnableInterrupts(GPIO3, 26 << 1);
                    return;
                }
                if (_tmp_time_high > 1300 && _tmp_time_high < 1900)
                {
                    bytes[_i] |= 1 << (_j);
                    _count++;
                }
            }
        }
        // 引脚中断使能
        GPIO_EnableInterrupts(GPIO3, 26 << 1);
        trigger_rec_process(bytes);
    }
    // 引脚中断使能
    GPIO_EnableInterrupts(GPIO3, 26 << 1);
}

void trigger_rec_process(const unsigned char *_rec_bytes)
{
    // lcd_showuint16(0, 0, _rec_bytes[2]);
    // lcd_showuint16(0, 1, _rec_bytes[3]);
    char _nec_data_decode_dat = nec_data_decode(_rec_bytes[2], _rec_bytes[3]);
    if (0 == _nec_data_decode_dat)
        return;
    if (CURSOR_NO_SELECT_FLAG == cursor_position._atomic)
    {
        if (CURSOR_NO_SELECT_FLAG == cursor_position._sub)
        {
            if ('u' == _nec_data_decode_dat)
            {
                if (cursor_position._parent > 1)
                    cursor_position._parent--;
            }
            else if ('b' == _nec_data_decode_dat)
            {
                if (cursor_position._parent < page_guide_cap)
                    cursor_position._parent++;
            }
            else if ('o' == _nec_data_decode_dat)
                cursor_position._sub = 1;
        }
        else
        {
            if ('u' == _nec_data_decode_dat)
            {
                if (cursor_position._sub > 0)
                    cursor_position._sub--;
            }
            else if ('b' == _nec_data_decode_dat)
            {
                // if (cursor_position._sub < page_var_cap[cursor_position._sub / GUI_PAGE_LINE])
                //     cursor_position._sub++;
                if (cursor_position._sub < INT_ROUND(VAR_DATA_NUM, GUI_PAGE_LINE - 1) - 1 + VAR_DATA_NUM)
                    cursor_position._sub++;
                else
                    cursor_position._sub = 1;
            }
            else if ('l' == _nec_data_decode_dat)
            {
                if (cursor_position._sub < GUI_PAGE_LINE)
                    cursor_position._sub = (INT_ROUND(VAR_DATA_NUM, GUI_PAGE_LINE - 1) - 1) * GUI_PAGE_LINE + 1;
                else
                    cursor_position._sub -= GUI_PAGE_LINE;
            }
            else if ('r' == _nec_data_decode_dat)
            {
                if (cursor_position._sub >= (INT_ROUND(VAR_DATA_NUM, GUI_PAGE_LINE - 1) - 1) * GUI_PAGE_LINE)
                    cursor_position._sub %= GUI_PAGE_LINE;
                else
                    cursor_position._sub += GUI_PAGE_LINE;
            }
            else if ('o' == _nec_data_decode_dat)
            {
				// 参数页
				if (cursor_position._parent == 1)
				{
					if (cursor_position._sub % GUI_PAGE_LINE != 0)
					{
						cursor_position._atomic = 8;
					}
					else
					{
						uint32_t _get_page_index = cursor_position._sub / 10;
						static char _tmp_title_str[] = "@GO TO PAGE:    ";
						memcpy((char *)((unsigned int)page_var_str + GUI_PAGE_LINE * GUI_PAGE_CAP * _get_page_index), _tmp_title_str, strlen(_tmp_title_str));
						cursor_position._atomic = 12;
					}
				}
            }
            else if ('#' == _nec_data_decode_dat)
            {
                cursor_position._atomic = CURSOR_NO_SELECT_FLAG;
                cursor_position._sub = CURSOR_NO_SELECT_FLAG;
            }
        }
    }
    else
    {
        if (1 == cursor_position._parent)
        {
            if ((_nec_data_decode_dat >= '0' && _nec_data_decode_dat <= '9') || _nec_data_decode_dat == '.')
            {
                *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) = _nec_data_decode_dat;
                if (cursor_position._atomic < 15)
                    cursor_position._atomic++;
            }
            else if ('r' == _nec_data_decode_dat)
            {
                if (cursor_position._atomic < 15)
                    cursor_position._atomic++;
            }
            else if ('l' == _nec_data_decode_dat)
            {
                if (cursor_position._atomic > 8)
                    cursor_position._atomic--;
            }
            else if ('o' == _nec_data_decode_dat)
            {
                // 检测是否含两个及以上的小数点
                if (cursor_position._sub % GUI_PAGE_LINE != 0)
                {
                    uint16_t _point_num = 0;
                    for (uint16_t _i = 8; _i < 16; ++_i)
                    {
                        if ('.' == *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + _i))
                            _point_num++;
                    }
                    if (_point_num <= 1)
                    {
                        cursor_position._atomic = CURSOR_NO_SELECT_FLAG;
                        var_str_to_var_data(cursor_position._sub);
                        var_data_to_usr_data();
                        write_data_to_flash();
                    }
                }
                else
                {
                    // 检测是否为整数
                    bool _is_int = true;
                    for (uint16_t _i = 12; _i < 16; ++_i)
                    {
                        if (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + _i) != ' ' &&
                            (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + _i) > '9' ||
                             *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + _i) < '0'))
                        {
                            _is_int = false;
                        }
                    }
                    if (_is_int)
                    {
                        // 检测page是否存在
                        bool _exit = true;
                        uint32_t _tmp_page_index = 0;
                        int16_t _s = 0;
                        for (int16_t _i = 1; _i < 5; ++_i)
                        {
                            if (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + GUI_PAGE_CAP - _i) != ' ')
                            {
                                _tmp_page_index += (uint32_t)(((*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + GUI_PAGE_CAP - _i)) - '0') * pow(10, _s));
                                _s++;
                            }
                            else
                            {
                                if (_s != 0)
                                {
                                    _exit = false;
                                }
                            }
                        }
                        if (_tmp_page_index >= INT_ROUND(VAR_DATA_NUM, GUI_PAGE_LINE - 1))
                        {
                            _exit = false;
                        }
                        if (_exit)
                        {
                            uint32_t _get_page_index = cursor_position._sub / 10;
                            static char _tmp_page_num[16];
                            static char _tmp_title_str[] = "|    PAGE      |";
                            sprintf(_tmp_page_num, "%u", _get_page_index);
                            assert(strlen(_tmp_page_num) <= 4);
                            memcpy((char *)((unsigned int)_tmp_title_str + 10), _tmp_page_num, strlen(_tmp_page_num));
                            memcpy((char *)((unsigned int)page_var_str + GUI_PAGE_LINE * GUI_PAGE_CAP * _get_page_index), _tmp_title_str, strlen(_tmp_title_str));
                            cursor_position._sub = _tmp_page_index * GUI_PAGE_LINE + 1;
                            cursor_position._atomic = CURSOR_NO_SELECT_FLAG;
                        }
                    }
                }
            }
            else if ('u' == _nec_data_decode_dat)
            {
                if (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) == '9')
                {
                    *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) = '.';
                }
                else if (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) == '.')
                {
                    *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) = '0';
                }
                else
                {
                    *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) += 1;
                    if (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) > '9')
                        *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) = '9';
                    if (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) < '.')
                        *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) = '.';
                }
            }
            else if ('b' == _nec_data_decode_dat)
            {
                if (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) == '0')
                {
                    *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) = '.';
                }
                else if (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) == '.')
                {
                    *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) = '9';
                }
                else
                {
                    *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) -= 1;
                    if (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) > '9')
                        *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) = '9';
                    if (*((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) < '.')
                        *((char *)page_var_str + cursor_position._sub * GUI_PAGE_CAP + cursor_position._atomic) = '.';
                }
            }
        }
    }
    gui_refresh(_nec_data_decode_dat);
}

/*
 * @note: 关于解码：若出现按相关按键不反应，请自行检查 nec 解码是否有问题。
 *        检查方法：注释 函数 trigger_rec_process() 最后一行的 gui_refresh();
 *        将该函数的参数 _rec_bytes[2] 与 _rec_bytes[3] 的值显示到屏幕上，
 *        对照 nec_data_decode 函数的 _dat1 与 _dat2 参数
 *        注意：_dat1 与 _dat2 互为反码关系，若不是，请检查软件定时器。
 */
char nec_data_decode(const uint8_t _dat1, const uint8_t _dat2)
{
    switch (_dat1)
    {
    case 69:
        if (0 == (_dat1 & _dat2))
            return '1';
        break;
    case 70:
        if (0 == (_dat1 & _dat2))
            return '2';
        break;
    case 71:
        if (0 == (_dat1 & _dat2))
            return '3';
        break;
    case 68:
        if (0 == (_dat1 & _dat2))
            return '4';
        break;
    case 64:
        if (0 == (_dat1 & _dat2))
            return '5';
        break;
    case 67:
        if (0 == (_dat1 & _dat2))
            return '6';
        break;
    case 7:
        if (0 == (_dat1 & _dat2))
            return '7';
        break;
    case 21:
        if (0 == (_dat1 & _dat2))
            return '8';
        break;
    case 9:
        if (0 == (_dat1 & _dat2))
            return '9';
        break;
    case 25:
        if (0 == (_dat1 & _dat2))
            return '0';
        break;
    case 8:
        if (0 == (_dat1 & _dat2))
            return 'l'; // left
        break;
    case 24:
        if (0 == (_dat1 & _dat2))
            return 'u'; // up
        break;
    case 22:
        if (0 == (_dat1 & _dat2))
            return '.';
        break;
    case 13:
        if (0 == (_dat1 & _dat2))
            return '#';
        break;
    case 90:
        if (0 == (_dat1 & _dat2))
            return 'r'; // right
        break;
    case 82:
        if (0 == (_dat1 & _dat2))
            return 'b'; // buttom
        break;
    case 28:
        if (0 == (_dat1 & _dat2))
            return 'o'; // ok
        break;
    default:
        break;
    }
    return 0;
}

void write_data_to_flash()
{
#define FLASH_SECTOR (FLASH_SECTOR_NUM - 1)
#define MAX_PAGE_PARAM 64
    uint8_t _status;
    if (flash_check(FLASH_SECTOR, 0))
    {
        _status = flash_erase_sector(FLASH_SECTOR);
        if (_status)
            while (1)
                ;
    }
    const uint32_t _flash_var_num = VAR_DATA_NUM * 2;
    uint16_t _num_page_needed = _flash_var_num / MAX_PAGE_PARAM + 1;
    static uint32_t _write_buf[VAR_DATA_NUM * 2];
    page_str_to_uint32(_write_buf, _flash_var_num);
    for (uint16_t _i = 0; _i < _num_page_needed; ++_i)
    {
        uint16_t _tmp_num_flash = MAX_PAGE_PARAM;
        if (_i == _num_page_needed - 1)
            _tmp_num_flash = _flash_var_num % MAX_PAGE_PARAM;
        _status = flash_page_program(FLASH_SECTOR, _i, (uint32 const *)(_write_buf + (MAX_PAGE_PARAM * _i)), _tmp_num_flash);
        if (_status)
            while (1)
                ;
    }
}

void read_data_from_flash()
{
#define FLASH_SECTOR (FLASH_SECTOR_NUM - 1)
#define MAX_PAGE_PARAM 64
    const uint32_t _flash_var_num = VAR_DATA_NUM * 2;
    static uint32_t _read_buf[VAR_DATA_NUM * 2];
    uint16_t _num_page_needed = _flash_var_num / MAX_PAGE_PARAM + 1;
    DCACHE_CleanInvalidateByRange(FLASH_BASE_ADDR + FLASH_SECTOR * FLASH_SECTOR_SIZE, _flash_var_num * 4);
    for (uint16_t _i = 0; _i < _num_page_needed; ++_i)
    {
        uint16_t _tmp_num_flash = MAX_PAGE_PARAM;
        if (_i == _num_page_needed - 1)
            _tmp_num_flash = _flash_var_num % MAX_PAGE_PARAM;
        flash_read_page(FLASH_SECTOR, _i, (uint32 *)(_read_buf + (MAX_PAGE_PARAM * _i)), _tmp_num_flash);
    }
    flash_data_to_var_str(_read_buf, _flash_var_num);
    uint32_t _tmp_var_str_len = VAR_DATA_NUM + INT_ROUND(VAR_DATA_NUM, GUI_PAGE_LINE - 1);
    for (uint32_t _i = 0; _i < _tmp_var_str_len; ++_i)
    {
        if (_i % 10 != 0)
        {
            var_str_to_var_data(_i);
        }
    }
}

void page_str_to_uint32(uint32_t *_uint32_arr, const uint32_t _len)
{
    for (uint32_t _i = 0; _i < _len; ++_i)
    {
        if (_i % 2 == 0)
            memcpy((char *)(&_uint32_arr[_i]), (char *)page_var_str + (1 + (_i / 2) + (_i / 2) / 9) * GUI_PAGE_CAP + 8, 4);
        else
            memcpy((char *)(&_uint32_arr[_i]), (char *)page_var_str + (1 + (_i / 2) + (_i / 2) / 9) * GUI_PAGE_CAP + 12, 4);
    }
}

void flash_data_to_var_str(const uint32_t *_uint32_arr, const uint32_t _len)
{
    for (uint32_t _i = 0; _i < _len; ++_i)
    {
        if (_i % 2 == 0)
            memcpy((char *)page_var_str + (1 + (_i / 2) + (_i / 2) / 9) * GUI_PAGE_CAP + 8, (char *)(&_uint32_arr[_i]), 4);
        else
            memcpy((char *)page_var_str + (1 + (_i / 2) + (_i / 2) / 9) * GUI_PAGE_CAP + 12, (char *)(&_uint32_arr[_i]), 4);
    }
}

void var_str_to_var_data(const uint32_t _var_str_index)
{
    uint32_t _tmp_page_var_index = _var_str_index - 1 - _var_str_index / 10;
    var_data[_tmp_page_var_index] = page_var_to_float((char *)page_var_str + _var_str_index * GUI_PAGE_CAP);
}

double page_var_to_float(const char *_str)
{
    uint16_t _point_pos = 0;
    double _num = 0;
    for (uint16_t _i = 1; _i < 9; ++_i)
    {
        if (_str[GUI_PAGE_CAP - _i] == '.')
        {
            _point_pos = _i;
        }
        else
        {
            if (_point_pos == 0)
                _num += (_str[GUI_PAGE_CAP - _i] - '0') * pow(10, _i - 1);
            else
                _num += (_str[GUI_PAGE_CAP - _i] - '0') * pow(10, _i - 2);
        }
    }
    if (_point_pos != 0)
        return _num / pow(10, _point_pos - 1);
    else
        return _num;
}


extern uint8_t image[MT9V03X_H][MT9V03X_W];
void img_view_control(void *_argv)
{
	while (*((bool*)_argv))
		lcd_displayimage032((uint8_t *)image, MT9V03X_CSI_W, MT9V03X_CSI_H);
}

