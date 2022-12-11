/*
 * [For SEU smart car race 2022]
 * @File  : fifo.c
 * @Author: Feijie Luo
 * @Date  : 2022
 */

#include "fifo.h"

FIFO_State FIFO_Init(FIFO_Struct *_fifo_struct, uint8_t *_buffer, uint32_t _size)
{
    _fifo_struct->_buffer = _buffer;
    _fifo_struct->_head = 0;
    _fifo_struct->_end = 0;
    _fifo_struct->_size = _size;
    _fifo_struct->_max = _size;
    return FIFO_SUCCESS_STATE;
}

void FIFO_HeadOffset(FIFO_Struct *_fifo_struct, uint32_t _offset)
{
    _fifo_struct->_head += _offset;
    while (_fifo_struct->_head >= _fifo_struct->_max)
    {
        _fifo_struct->_head -= _fifo_struct->_max;
    }
}

void FIFO_EndOffset(FIFO_Struct *_fifo_struct, uint32_t _offset)
{
    _fifo_struct->_end += _offset;
    while (_fifo_struct->_end >= _fifo_struct->_max)
    {
        _fifo_struct->_end -= _fifo_struct->_max;
    }
}

void FIFO_Clean(FIFO_Struct *_fifo_struct)
{
    _fifo_struct->_head = 0;
    _fifo_struct->_end = 0;
    _fifo_struct->_size = _fifo_struct->_max;
}

uint32_t FIFO_Used(FIFO_Struct *_fifo_struct)
{
    return _fifo_struct->_max - _fifo_struct->_size;
}

uint32_t FIFO_Free(FIFO_Struct *_fifo_struct)
{
    return _fifo_struct->_size;
}

FIFO_State FIFO_WriteBuffer(FIFO_Struct *_fifo_struct, uint8_t *_to_write_buffer, uint32_t _length)
{
    if (_fifo_struct->_size >= _length)
    {
        uint32_t _tmp_len = _fifo_struct->_max - _fifo_struct->_head;

        if (_length > _tmp_len)
        {
            memcpy(&_fifo_struct->_buffer[_fifo_struct->_head], _to_write_buffer, _tmp_len);
            FIFO_HeadOffset(_fifo_struct, _tmp_len);
            _to_write_buffer += _tmp_len;
            memcpy(&_fifo_struct->_buffer[_fifo_struct->_head], _to_write_buffer, _length - _tmp_len);
            FIFO_HeadOffset(_fifo_struct, _length - _tmp_len);
        }
        else
        {
            memcpy(&_fifo_struct->_buffer[_fifo_struct->_head], _to_write_buffer, _length);
            FIFO_HeadOffset(_fifo_struct, _length);
        }

        _fifo_struct->_size -= _length;
    }
    else
    {
        return FIFO_NO_ENOUGH_SPACE;
    }
    return FIFO_SUCCESS_STATE;
}

/*
 * @Note: 传入的指针(_dat[uint8_t *])需指向一块已存在并具备充足内存的内存块
 */
FIFO_State FIFO_ReadBuffer(FIFO_Struct *_fifo_struct, uint8_t *_dat, uint32_t *_length, FIFO_Operate _operate)
{
    uint32_t _temp_length;
    FIFO_State _ret_status = FIFO_SUCCESS_STATE;
    if (*_length > FIFO_Used(_fifo_struct))
    {
        *_length = (_fifo_struct->_max - _fifo_struct->_size);
        _ret_status = FIFO_NO_ENOUGH_DATA;
    }

    _temp_length = _fifo_struct->_max - _fifo_struct->_end;
    if (*_length <= _temp_length)
    {
        if (NULL != _dat)
            memcpy(_dat, &_fifo_struct->_buffer[_fifo_struct->_end], *_length);
    }
    else
    {
        if (NULL != _dat)
        {
            memcpy(_dat, &_fifo_struct->_buffer[_fifo_struct->_end], _temp_length);
            memcpy(&_dat[_temp_length], &_fifo_struct->_buffer[0], *_length - _temp_length);
        }
    }

    if (_operate == FIFO_READ_AND_POP)
    {
        FIFO_EndOffset(_fifo_struct, *_length);
        _fifo_struct->_size += *_length;
    }

    return _ret_status;
}

/*
 * @Note: 传入的指针(_dat[uint8_t *])需指向一块已存在并具备充足内存的内存块
 */
FIFO_State FIFO_ReadCommand(FIFO_Struct *_fifo_struct, uint8_t *_dat, uint8_t _command_stork, FIFO_CommandSep _command_sep, FIFO_Operate _operate)
{
    FIFO_State _ret_state = FIFO_SUCCESS_STATE;
    uint32_t _index = _fifo_struct->_end;
    uint32_t _data_m = FIFO_Used(_fifo_struct);
    uint32_t _dat_index = 0;
    while (_data_m-- > 0)
    {
        if (_index >= _fifo_struct->_max)
            _index = 0;
        if (_fifo_struct->_buffer[_index] == _command_stork)
        {
            switch (_command_sep)
            {
            case FIFO_COMMAND_INC_ZERO:
            {
                _dat[_dat_index++] = '\0';
                break;
            }
            case FIFO_COMMAND_INC_SEP:
            {
                _dat[_dat_index++] = _fifo_struct->_buffer[_index];
                break;
            }
            default:
                break;
            }
            if (_operate == FIFO_READ_AND_POP)
            {
                FIFO_EndOffset(_fifo_struct, _dat_index);
                _fifo_struct->_size += _dat_index;
            }
            return _ret_state;
        }
        _dat[_dat_index++] = _fifo_struct->_buffer[_index];
        _index++;
    }
    _ret_state = FIFO_NO_ENOUGH_DATA;
    return _ret_state;
}
