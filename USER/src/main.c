#include "debug.h"
#include "gui.h"
#include "headfile.h"
#include "img_process.h"
#include "soft_timer.h"

// 定义ADC通道与引脚，其定义在zf_adc.h文件中
#define POWER_ADC1_MOD ADC_1         // 定义通道1 ADC模块号
#define POWER_ADC1_PIN ADC1_CH2_B13  // 定义通道1 ADC引脚

#define POWER_ADC2_MOD ADC_1         // 定义通道2 ADC模块号
#define POWER_ADC2_PIN ADC1_CH1_B12  // 定义通道2 ADC引脚

#define POWER_ADC3_MOD ADC_1         // 定义通道3 ADC模块号
#define POWER_ADC3_PIN ADC1_CH5_B16  // 定义通道3 ADC引脚

#define POWER_ADC4_MOD ADC_1         // 定义通道4 ADC模块号
#define POWER_ADC4_PIN ADC1_CH6_B17  // 定义通道4 ADC引脚

#define POWER_ADC5_MOD ADC_1         // 定义通道5 ADC模块号
#define POWER_ADC5_PIN ADC1_CH4_B15  // 定义通道5 ADC引脚

#define POWER_ADC6_MOD ADC_1         // 定义通道6 ADC模块号
#define POWER_ADC6_PIN ADC1_CH3_B14  // 定义通道6 ADC引脚

#define CONTINUE_PIX 4

// uint8_t image[MT9V03X_H][MT9V03X_W];
//  凌瞳彩色摄像头图像数组
// uint16_t image_scc8660[SCC8660_CSI_PIC_H][SCC8660_CSI_PIC_W];
extern uint8 mt9v03x_csi_finish_flag;

void Blink(unsigned int _times) {
    static int _count = 0;
    if (_count++ == _times) {
        _count = 0;
        gpio_toggle(B9);
    }
}

void Init() {
    pit_init();
    pit_interrupt_ms(PIT_CH0, 1);
    //  中断优先级设置，CSI最高
    NVIC_SetPriority(PIT_IRQn, 0);
    NVIC_SetPriority(CSI_IRQn, 2);

    // 初始化flash, 储存参数. 一个扇区有16页, 一页可以储存256字节,
    // 一个参数占4个字节, 因此一页最多只能存64个参数
    flash_init();

    gpio_init(C27, GPO, 0, GPIO_PIN_CONFIG);

    // 初始化 QTIMER_1 A相使用QTIMER1_TIMER0_C0 B相使用QTIMER1_TIMER1_C1
    //qtimer_quad_init(QTIMER_1, QTIMER1_TIMER0_C0, QTIMER1_TIMER1_C1);
    qtimer_quad_init(QTIMER_1, QTIMER1_TIMER2_C2, QTIMER1_TIMER3_C24);

    // 电机使能
    gpio_init(B13, GPO, 1, GPIO_PIN_CONFIG);
    pwm_init(PWM1_MODULE3_CHB_D1, 800, 10000);  // 面向车头右轮前进
    pwm_init(PWM1_MODULE3_CHA_D0, 800, 10000);
    pwm_init(PWM2_MODULE3_CHB_D3, 800, 10000);
    pwm_init(PWM2_MODULE3_CHA_D2, 800, 10000);  // 左轮前进

    // 凌瞳彩色摄像头初始化
    scc8660_csi_init();

    // 红外信号引脚中断
    gpio_interrupt_init(D26, FALLING, GPIO_INT_CONFIG);
    NVIC_SetPriority(GPIO3_Combined_16_31_IRQn, 1);

    // 五向开关引脚中断配置
    gpio_interrupt_init(C5, RISING,
                        SPEED_100MHZ | DSE_R0 | PULLDOWN_100K | PULL_EN);
    gpio_interrupt_init(C6, RISING,
                        SPEED_100MHZ | DSE_R0 | PULLDOWN_100K | PULL_EN);
    gpio_interrupt_init(C7, RISING,
                        SPEED_100MHZ | DSE_R0 | PULLDOWN_100K | PULL_EN);
    gpio_interrupt_init(C25, RISING,
                        SPEED_100MHZ | DSE_R0 | PULLDOWN_100K | PULL_EN);
    gpio_interrupt_init(C26, RISING,
                        SPEED_100MHZ | DSE_R0 | PULLDOWN_100K | PULL_EN);
    NVIC_SetPriority(GPIO2_Combined_0_15_IRQn, 15);
    NVIC_SetPriority(GPIO2_Combined_16_31_IRQn, 15);

    // ADC初始化
    // 同一个ADC模块分辨率应该设置为一样的，如果设置不一样，则最后一个初始化时设置的分辨率生效
    adc_init(POWER_ADC1_MOD, POWER_ADC1_PIN, ADC_8BIT);
    adc_init(POWER_ADC2_MOD, POWER_ADC2_PIN, ADC_8BIT);
    adc_init(POWER_ADC3_MOD, POWER_ADC3_PIN, ADC_8BIT);
    adc_init(POWER_ADC4_MOD, POWER_ADC4_PIN, ADC_8BIT);
    adc_init(POWER_ADC5_MOD, POWER_ADC5_PIN, ADC_8BIT);
    adc_init(POWER_ADC6_MOD, POWER_ADC6_PIN, ADC_8BIT);

    // ICM20602 初始化，不使用请注释！！否则程序将卡死在这
    // icm20602_init_spi();
    // gui初始化
    gui_init();
    sys_tick_start(MS_TO_CLOCK_COUNT(1, CLOCK_GetFreq(kCLOCK_CpuClk)));
    //lcd_init();
}

short hist_gram[256];
uint8_t _img[SCC8660_CSI_PIC_W * SCC8660_CSI_PIC_H];

void PID(int err) {  // 位置式PID算法控制
    // 误差累加
    static int ERR = 0;
    ERR = 0.9 * ERR + err;  // 防止溢出 此时求和收敛
    // 上一个误差 求微分项
    static int err_last = 0;
    // 求调整幅度
    int delta = Kp * err + Ki * ERR +
                Kd * (err - err_last);  // 实际上要求 delta < BasicSpeed,
                                        // 所以传入的err在BasicSpeed/100的量级
    delta /= 2;
    int slowdown = err > 0 ? err * TurnSlow : -err * TurnSlow;
    int lspeed = BasicSpeed - delta - slowdown;
    int rspeed = BasicSpeed + delta - slowdown;
    // 防出奇怪的问题, 限位
    if (lspeed < 0)
        lspeed = 0;
    else if (lspeed > 45000)
        lspeed = 45000;
    if (rspeed < 0)
        rspeed = 0;
    else if (rspeed > 45000)
        rspeed = 45000;

    //static int16 cou=0;
    int16 tempp = qtimer_quad_get(QTIMER_1, QTIMER1_TIMER2_C2);
    //if(tempp<cou) cou=tempp;
    //lcd_showint16(0, 0, cou);
    if( tempp < 0-SpeedLimmit ){
      rspeed=5000;
      lspeed=5000;
    }
    qtimer_quad_clear(QTIMER_1, QTIMER1_TIMER2_C2);
    

    pwm_duty(PWM1_MODULE3_CHB_D1, rspeed);
    pwm_duty(PWM2_MODULE3_CHA_D2, lspeed);

    err_last = err;
}

void CameraPic() {  // 图像处理
    int k = 0;
    for (int i = 0; i < SCC8660_CSI_PIC_W; i++) {
        for (int j = 0; j < SCC8660_CSI_PIC_H; j++) {
            uint16 pixel = scc8660_csi_image[j][i];
            pixel = ((pixel & 0xff) << 8) | (pixel >> 8);
            uint8 r = pixel >> 11;
            uint8 g = (pixel >> 5) & 0x3f;
            uint8 b = pixel & 0x1f;
            _img[k++] = (r * 299 + g * 587 + b * 114 + 500) / 1000;
        }
    }
    get_hist_gram((uint8_t*)_img, SCC8660_CSI_PIC_H, SCC8660_CSI_PIC_W,
                  hist_gram);
    unsigned char threshold = get_threshold_otsu(hist_gram);
    binaryzation_process((uint8_t*)_img, SCC8660_CSI_PIC_H, SCC8660_CSI_PIC_W,
                         threshold + 3);
}
int getBias(double lookaaaat) {  // lookaaaat是0~1的小数，表示屏幕从上往下的比例
    int biggestwidth = 0;
    int rightedge = CONTINUE_PIX;  // 这两个edge比实际的大CONTINUE_PIX,
                                   // 所以后面求偏差需要减掉
    int lastedge = CONTINUE_PIX;
    uint8_t now = 0;
    int center = SCC8660_CSI_PIC_W / 2;
    for (; rightedge < SCC8660_CSI_PIC_W; rightedge++) {  // 遍历一行
        uint8_t temp = _img[(int)(SCC8660_CSI_PIC_H *
                                  (rightedge + lookaaaat))];  // 更新状态
        if (temp != now) {  // 出现了不同的像素
            for (int i = 0; ++rightedge < SCC8660_CSI_PIC_W && i < CONTINUE_PIX;
                 i++) {  // 到达边界默认后面都连续
                // 是否连续 CONTINUE_PIX 像素保持
                if (temp !=
                    _img[(int)(SCC8660_CSI_PIC_H * (rightedge + LookAt))]) {
                    temp = 2;
                    break;
                }
            }
            if (temp != 2) {
                // 确实 CONTINUE_PIX 像素连续, 或到边界
                now = temp;
                if (now)  // 0->255,黑转白
                    lastedge = rightedge;
                else {  // 255->0,白转黑，量区间大小
                    int width = rightedge - lastedge;
                    if (width > biggestwidth) {
                        biggestwidth = width;
                        center = (rightedge + lastedge) / 2 - CONTINUE_PIX;
                    }
                }
            }
        }
    }

    // 最边缘需要处理,防止最左边也是白色
    if (now) {
        int width = SCC8660_CSI_PIC_W - lastedge + CONTINUE_PIX;
        if (width > biggestwidth)
            center = (rightedge + lastedge - CONTINUE_PIX) / 2;
    }

    // 此时已经获取中心点
    center = center -
             SCC8660_CSI_PIC_W / 2;  // 正数表示轨道中心点在视野左侧,右加左减
    return center;
}
bool ifend() {  // 判断是否到终点

    uint8_t now = 0;
    int x = 0;
    for (int rightedge = 0; rightedge < SCC8660_CSI_PIC_W;
         rightedge++) {  // 遍历一行
        uint8_t temp =
            _img[(int)(SCC8660_CSI_PIC_H * (rightedge + 0.37))];  // 更新状态
        if (temp != now) {  // 出现了不同的像素
            for (int i = 0; rightedge < SCC8660_CSI_PIC_W && i < 3;
                 rightedge++, i++) {
                // 是否连续 3 像素保持
                if (temp !=
                    _img[(int)(SCC8660_CSI_PIC_H * (rightedge + 0.37))]) {
                    temp = 2;
                    break;
                }
            }
            if (temp != 2) {
                // 确实 CONTINUE_PIX 像素连续
                now = temp;
                x++;
            }
        }
    }
    return x > 6;
}
float centerWhite() {  // 中心线白色比例
    float sum = 0;
    for (int i = 0; i < SCC8660_CSI_PIC_H; i++) {
        if (_img[SCC8660_CSI_PIC_H * (SCC8660_CSI_PIC_W / 2 - Correct) + i]) {
            sum++;
        }
    }
    return sum / SCC8660_CSI_PIC_H;
}
int LR_diff() {  // 左右30%部分的白色像素差
    int sum = 0;
    for (int i = 0; i < SCC8660_CSI_PIC_H * SCC8660_CSI_PIC_H * 0.3; i++)
        if (_img[i])
            sum++;
    for (int i = SCC8660_CSI_PIC_H * SCC8660_CSI_PIC_H * 0.92;
         i < SCC8660_CSI_PIC_H * SCC8660_CSI_PIC_H * 0.3; i++)
        if (_img[i])
            sum--;
    return sum;
}
/*-----------环岛↓------------*/
// 补黑
void island_drawBlack(float x1, float y1, float x2, float y2) {
    if (x1 < x2) {
        float temp = x1;
        x1 = x2;
        x2 = temp;
        temp = y1;
        y1 = y2;
        y2 = temp;
    }
    float k = (y1 - y2) / (x1 - x2);
    int y = 0;
    for (int x = x2; x <= x1; x++) {
        y = (k * (x - x1)) + y1;
        for (int i = 0; i <= y; i++) {
            _img[(x * SCC8660_CSI_PIC_H) + i] = 0;
        }
    }
}
// 环岛判断：不对称
// 从上往下第53行 检测左边全白右边有黑：找黑白交界点 需要连续性判断
// 图像是镜像的！！！所以环岛在左侧（图片起点在右上角）
int first_wb_border() {
    for (int i = SCC8660_CSI_PIC_W - 1; i >= 0;
         i--) {  // 图片从左向右遍历第53行
        if (!_img[SCC8660_CSI_PIC_H * i + 52]) {  // 如果看到了黑色
            // 连续型判断
            bool ifcontinue = true;
            for (int j = 0; --i >= 0 && j < CONTINUE_PIX; j++) {
                if (_img[SCC8660_CSI_PIC_H * i + 54]) {  // 看到了白色，不连续
                    ifcontinue = false;
                    break;
                }
            }
            if (ifcontinue) {  // 如果黑色连续(或到头)就认为找到了第一个黑->白交界点，返回交界点列坐标
                return i + CONTINUE_PIX;
            }
        }
    }
    return SCC8660_CSI_PIC_W;
}

// 识别到环岛后 补线 此为找到顶点
// 先在从右向左69%的列找到黑白分界点，然后向右找到黑白分界点，返回坐标
void island_Apex(int* x, int* y) {
    int horizonLine = 0.69 * SCC8660_CSI_PIC_W;
    int basic = SCC8660_CSI_PIC_H * horizonLine;
    for (int i = 0; i < SCC8660_CSI_PIC_H / 2; i++) {  // 只检测上一半
        if (_img[basic + i]) {                         // 看到白色
            bool ifcontinue = true;
            for (int j = 0; ++i < SCC8660_CSI_PIC_H / 2 && j < CONTINUE_PIX;
                 j++) {
                if (!_img[basic + i]) {  // 看到黑色
                    ifcontinue = false;
                    break;
                }
                if (ifcontinue) {  // 第一个从黑到白的点的列坐标
                    i -= CONTINUE_PIX;
                    (*y) = i;
                    // 找行第一个从黑到白的点
                    for (; --horizonLine > 0;) {  // 防止h=0，所以用>
                        if (_img[horizonLine * SCC8660_CSI_PIC_H +
                                 i]) {  // 如果看到了白色
                            // 连续性检验
                            for (j = 0; --horizonLine > 0 && j < CONTINUE_PIX;
                                 j++) {
                                if (!_img[horizonLine * SCC8660_CSI_PIC_H +
                                          i]) {
                                    ifcontinue = false;
                                    break;
                                }
                            }
                            if (ifcontinue) {
                                horizonLine += CONTINUE_PIX;
                                // (horizonLine,i)为坐标
                                (*x) = horizonLine;
                                return;
                            }
                        }
                    }
                    (*x) = 1;  // 都是黑的
                    return;
                }
            }
        }
    }
    (*x) = -1;  // 没找到
    (*y) = -1;
}

// 补线（全）
void island_addLine() {
    int x, y;
    island_Apex(&x, &y);
    if (x > 0 && y > 0)
        island_drawBlack(x, y + 2, 0, SCC8660_CSI_PIC_H - 3);
}
/*-----------环岛↑------------*/

int main(void) {
    DisableGlobalIRQ();
    board_init();
    Init();
    EnableGlobalIRQ(0);
    char IslandMode = 0;
    // 出库
	pwm_duty(PWM1_MODULE3_CHB_D1, Rout);
	pwm_duty(PWM2_MODULE3_CHA_D2, Lout);
	systick_delay_ms(Tout);
    while (1) {
        static int center = 0;  // 可能需要保留之前的结果，所以用static
        if (scc8660_csi_finish_flag) {  // 凌瞳彩色摄像头
            CameraPic();
            float centerWhites = centerWhite();
            if (ifend()) break;
           // lcd_showstr(0,0,(int8 const)&centerWhites);
            if (IslandMode) {               // 环岛模式
                if (centerWhites > 0.90) {  // 直道路
                    IslandMode = 0;
                    gpio_set(C27, 1);
					pwm_duty(PWM1_MODULE3_CHB_D1, go_2_l);
					pwm_duty(PWM2_MODULE3_CHA_D2, go_2_r);
					systick_delay_ms(go_2_T);
                    gpio_set(C27, 0);
					continue;
                }
            } else {
              if (centerWhites > 0.88 && first_wb_border() < SCC8660_CSI_PIC_W / 2) {
                  //
					IslandMode = 1;
                                         gpio_set(C27, 1);
					pwm_duty(PWM1_MODULE3_CHB_D1, go_1_l);
					pwm_duty(PWM2_MODULE3_CHA_D2, go_1_r);
					systick_delay_ms(go_1_T);
					pwm_duty(PWM1_MODULE3_CHB_D1, turn_l);
					pwm_duty(PWM2_MODULE3_CHA_D2, turn_r);
					systick_delay_ms(turn_T);
                                        gpio_set(C27, 0);
                    continue;
                }
            }
            if (camera)
                lcd_displayimage032((uint8_t*)_img, SCC8660_CSI_PIC_H,
                                    SCC8660_CSI_PIC_W);  // 显示图像
            scc8660_csi_finish_flag = 0;

            center = getBias(LookAt);
            PID((center + Correct+Decrease*IslandMode) * 6);
        }  // if
    }      // while
    // 入库
        gpio_set(C27,1);
	pwm_duty(PWM1_MODULE3_CHB_D1, Rin);
	pwm_duty(PWM2_MODULE3_CHA_D2, Lin);
	systick_delay_ms(Tin);
        //刹车  BrakePower小于10000，越小力度越大
        pwm_duty(PWM1_MODULE3_CHB_D1, BrakePower);
        pwm_duty(PWM2_MODULE3_CHA_D2, BrakePower);
        systick_delay_ms(BrakeTime);
	// // 停车
	pwm_duty(PWM1_MODULE3_CHB_D1, 10000);
	pwm_duty(PWM2_MODULE3_CHA_D2, 10000);
       
        gpio_set(C27,0);
}
