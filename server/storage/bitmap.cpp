#include "bitmap.h"
int find_first_zero(char byte, int start)
{
    for (int i = start; i < 8; i++)
        if ((byte & (1 << i)) == 0)
            return i;
    return -1;
}

int find_first_setted(char byte, int start)
{
    for (int i = start; i < 8; i++)
        if ((byte & (1 << i)) != 0)
            return i;
    return -1;
}

void BitMap::init(char *bitmap, int size)
{
    bit_map_ = bitmap;
    size_ = size;
}

bool BitMap::get_bit(int index)
{
    char bits = bit_map_[index / 8];
    return (bits & (1 << (index % 8))) != 0;
}

void BitMap::set_bit(int index)
{
    char &bits = bit_map_[index / 8];
    bits |= (1 << (index % 8));
}

void BitMap::clear_bit(int index)
{
    char &bits = bit_map_[index / 8];
    bits &= ~(1 << (index % 8));
}

int BitMap::next_unsetted_bit(int start)
{
    int ret = -1;
    int start_in_byte = start % 8;
    for (int iter = start / 8, end = (size_ % 8 == 0 ? size_ / 8 : size_ / 8 + 1); iter <= end; iter++)
    {
        char byte = bit_map_[iter];
        if (byte != -1)
        {
            int index_in_byte = find_first_zero(byte, start_in_byte);
            if (index_in_byte >= 0)
            {
                ret = iter * 8 + index_in_byte;
                break;
            }
            start_in_byte = 0;
        }
    }
    if (ret >= size_)
        ret = -1;
    return ret;
}

int BitMap::next_setted_bit(int start)
{
    int ret = -1;
    int start_in_byte = start % 8;
    for (int iter = start / 8, end = (size_ % 8 == 0 ? size_ / 8 : size_ / 8 + 1); iter <= end; iter++)
    {
        char byte = bit_map_[iter];
        if (byte != 0x00)
        {
            int index_in_byte = find_first_setted(byte, start_in_byte);
            if (index_in_byte >= 0)
            {
                ret = iter * 8 + index_in_byte;
                break;
            }
            start_in_byte = 0;
        }
    }
    if (ret >= size_)
        ret = -1;
    return ret;
}