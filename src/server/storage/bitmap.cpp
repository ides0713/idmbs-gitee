#include "bitmap.h"

int FindFirstZero(char byte, int start) {
    for (int i = start; i < 8; i++)
        if ((byte & (1 << i)) == 0)
            return i;
    return -1;
}

int FindFirstSet(char byte, int start) {
    for (int i = start; i < 8; i++)
        if ((byte & (1 << i)) != 0)
            return i;
    return -1;
}

void BitMap::Init(char *bitmap, int size) {
    bit_map_ = bitmap;
    size_ = size;
}

bool BitMap::GetBit(int index) {
    char bits = bit_map_[index / 8];
    return (bits & (1 << (index % 8))) != 0;
}

void BitMap::SetBit(int index) {
    char &bits = bit_map_[index / 8];
    bits |= (1 << (index % 8));
}

void BitMap::ClearBit(int index) {
    char &bits = bit_map_[index / 8];
    bits &= ~(1 << (index % 8));
}

int BitMap::NextUnsetBit(int start) {
    int ret = -1, start_in_byte = start % 8;
    for (int iter = start / 8, end = (size_ % 8 == 0 ? size_ / 8 : size_ / 8 + 1); iter <= end; iter++) {
        char byte = bit_map_[iter];
        if (byte != -1) {
            int index_in_byte = FindFirstZero(byte, start_in_byte);
            if (index_in_byte >= 0) {
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

int BitMap::NextSetBit(int start) {
    int ret = -1, start_in_byte = start % 8;
    for (int iter = start / 8, end = (size_ % 8 == 0 ? size_ / 8 : size_ / 8 + 1); iter <= end; iter++) {
        char byte = bit_map_[iter];
        if (byte != 0x00) {
            int index_in_byte = FindFirstSet(byte, start_in_byte);
            if (index_in_byte >= 0) {
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