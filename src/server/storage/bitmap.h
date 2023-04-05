#pragma once

///@brief a bitmap@n(NOTE:char's size is 1 byte and use it to stands for 1 bit)
class BitMap {
public:
    BitMap() : bit_map_(nullptr), size_(0) {}

    BitMap(char *bitmap, int size) : bit_map_(bitmap), size_(size) {}

    void init(char *bitmap, int size);

    bool getBit(int index);

    void setBit(int index);

    void clearBit(int index);

    ///@brief find next unset bit from pos:start@n(note:start is included)
    int nextUnsetBit(int start);

    int nextSetBit(int start);

private:
    char *bit_map_;
    int size_;
};