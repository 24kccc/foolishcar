/*
 *  [For SEU smart car race 2022]
 *  @File: soft_timer.h
 *  @Author: Feijie Luo
 *  @Last edit time: 2022-9-5
 *
 */

#ifndef _SOFT_TIMER_H_
#define _SOFT_TIMER_H_
#include "headfile.h"
#include "stdlib.h"
#include "stdint.h"
#include "core_cm7.h"

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

#define SOFT_TIMER_NUM 10

#define CLOCK_COUNT_TO_US(_count, _freq) ((uint64_t)((uint64_t)(_count)*1000000U / (_freq)))
#define CLOCK_COUNT_TO_MS(_count, _freq) ((uint64_t)((uint64_t)(_count)*1000U / (_freq)))

#define US_TO_CLOCK_COUNT(_us, _freq) ((uint64_t)((_freq) / ((uint64_t)(_us)*1000000U )))
#define MS_TO_CLOCK_COUNT(_ms, _freq) ((uint64_t)((_freq) / ((uint64_t)(_ms)*1000U )))

typedef void (*soft_timer_callback)(void *_argv);

typedef struct soft_timer_time_t
{
    uint32_t _ms;
    uint32_t _us;
} soft_timer_time_t;

typedef enum soft_timer_state
{
    SOFT_TIMER_STOPPED,
    SOFT_TIMER_RUNNING,
    SOFT_TIMER_TIMEOUT,
    SOFT_TIMER_NO_STATE
} soft_timer_state;

typedef enum soft_timer_mode
{
    SOFT_TIMER_MODE_ONE_SHOT,
    SOFT_TIMER_MODE_CYCLE
} soft_timer_mode;

typedef struct soft_timer_t
{
    soft_timer_state _soft_timer_state;
    soft_timer_mode _soft_timer_mode;
    soft_timer_time_t _due;
    uint32_t _period;
    soft_timer_callback _callback;
    void *_argv;
} soft_timer_t;
void soft_timer_systick_handle(void);
void soft_timer_init(void);
void soft_timer_systick_irq(void);
void sys_tick_start(const uint64_t _reload_tick);
bool soft_timer_start(const uint32_t _id, const soft_timer_mode _mode, const uint32_t _period, soft_timer_callback _callback, void *_argv);
void soft_timer_update(void);
bool soft_timer_stop(const uint32_t _id);
soft_timer_state soft_timer_get_state(const uint32_t _id);
soft_timer_time_t soft_timer_get_time(void);
unsigned int sys_tick_get_us(void);
soft_timer_time_t soft_timer_add_us(const soft_timer_time_t _time, const uint32_t _us);
int soft_timer_get_2_time_diff_us(const soft_timer_time_t _e, const soft_timer_time_t _s);
#endif