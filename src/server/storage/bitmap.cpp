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

void BitMap::initialize(char *bitmap, int size)
{
    bit_map_ = bitmap;
    size_ = size;
}

bool BitMap::getBit(int index)
{
    char bits = bit_map_[index / 8];
    return (bits & (1 << (index % 8))) != 0;
}

void BitMap::setBit(int index)
{
    char &bits = bit_map_[index / 8];
    bits |= (1 << (index % 8));
}

void BitMap::clearBit(int index)
{
    char &bits = bit_map_[index / 8];
    bits &= ~(1 << (index % 8));
}

int BitMap::nextUnsettedBit(int start)
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

int BitMap::nextSettedBit(int start)
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