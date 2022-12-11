/*
 * [For SEU smart car race 2022]
 * @File  : fifo.h
 * @Author: Feijie Luo
 * @Date  : 2022
 */
#include "stdio.h"
#include "stdint.h"
#include "string.h"
typedef struct FIFO_Struct
{
    uint8_t *_buffer;
    uint32_t _head;
    uint32_t _end;
    uint32_t _size;
    uint32_t _max;
} FIFO_Struct;

typedef enum FIFO_State
{
    FIFO_SUCCESS_STATE,
    FIFO_NO_ENOUGH_SPACE,
    FIFO_NO_ENOUGH_DATA,
    FIFO_NO_COMMAND
} FIFO_State;

typedef enum FIFO_Operate
{
    FIFO_READ_AND_POP,
    FIFO_READ_ONLY,
} FIFO_Operate;

typedef enum FIFO_CommandSep
{
    FIFO_COMMAND_INC_ZERO,
    FIFO_COMMAND_INC_SEP
} FIFO_CommandSep;
