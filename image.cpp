#include "image.h"
#include "common.h"

#include <iostream>
#include <iomanip>

namespace { 
void DBG(const char* str, ...) {
  #ifdef DEBUG
  va_list arglist;
  va_start(arglist, str);
  vprintf(str, arglist);
  va_end(arglist);
  #endif
}
}

Image::Image(int cols, int rows) {
  data_ = static_cast<uint8_t*>(std::malloc(cols * rows * sizeof(uint8_t)));
  cols_ = cols;
  rows_ = rows;
}

uint8_t* Image::Row(int r) {
  if (r < 0 || r >= rows_) {
    throw std::runtime_error("Row out of bounds.");
  }
  return &data_[r * cols_];
}

uint8_t& Image::operator()(int c, int r) {
  if (c < 0 || c >= cols_) {
    throw std::runtime_error("Column out of bounds.");
  }
  return Row(r)[c];
}

void Image::SetAll(uint8_t value) {
  std::memset(data_, value, rows_ * cols_);
}


bool Image::XORSprite(int c, int r, int height, uint8_t* sprite) {
  // TODO: Should be able to draw near edges of screen, and just draw as much
  // of the sprite as we can. OOB only if we START OOB
  if (c + 8 > cols_ || r + height > rows_) {
    throw std::runtime_error("Sprite renders out of bounds.");
  }
  bool pixel_was_disabled = false;
  int dots = 0;
  for (int y = 0; y < height; y++) {
    int current_r = r + y;
    uint8_t sprite_byte = sprite[y];
    DBG("%X ", sprite_byte);
    for (int x = 0; x < 8; x++) {
      int current_c = c + x;
      // Note: We scan from MSbit to LSbit
      uint8_t sprite_val = (sprite_byte & (0x80 >> x)) >> (7-x);
      if (sprite_val > 0) {
        dots++;
      }
      uint8_t& val = operator()(current_c, current_r);
      val |= sprite_val;
      pixel_was_disabled |= val == 0;
    }
  }
  DBG(" %d dots", dots);
  return pixel_was_disabled;
}

void Image::Print() {
  for (int r = 0; r < rows_; r++) {
    for (int c = 0; c < cols_; c++) {
      std::cout << std::setfill('0') << std::setw(3) << static_cast<int>(operator()(c,r)) << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void Image::DrawToStdout() {
  for (int r = 0; r < rows_; r++) {
    for (int c = 0; c < cols_; c++) {
      if (operator()(c,r) > 0) {
        std::cout << "X";
      } else {
        std::cout << " ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

Image::~Image() {
  free(data_);
}