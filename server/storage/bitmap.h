#pragma once
class BitMap {
public:
  BitMap(): bit_map_(nullptr), size_(0){}
  BitMap(char *bitmap, int size) : bit_map_(bitmap), size_(size){}

  void init(char *bitmap, int size);
  bool get_bit(int index);
  void set_bit(int index);
  void clear_bit(int index);

  /**
   * @param start 从哪个位开始查找，start是包含在内的
   */
  int next_unsetted_bit(int start);
  int next_setted_bit(int start);

private:
  char *bit_map_;
  int size_;
};