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

uint8_t& Image::At(int c, int r) {
  if (c < 0 || c >= cols_) {
    throw std::runtime_error("Column out of bounds.");
  }
  return Row(r)[c];
}

void Image::SetAll(uint8_t value) {
  std::memset(data_, value, rows_ * cols_);
}


bool Image::XORSprite(int c, int r, int height, uint8_t* sprite) {
  // TODO: If the sprite is positioned so part of it is outside the
  // coordinates of the display, it wraps around to the opposite side of the screen
  // So perhaps bound it to ensure it starts within the screen, then from there
  // we wrap as we draw.
  c = std::min(c, cols_ - 1);
  r = std::min(r, rows_ - 1);

  bool pixel_was_disabled = false;
  int dots = 0;
  for (int y = 0; y < height; y++) {
    int current_r = r + y;
    if (current_r >= rows_) break;
    uint8_t sprite_byte = sprite[y];
    DBG("%X ", sprite_byte);
    for (int x = 0; x < 8; x++) {
      int current_c = c + x;
      if (current_c >= cols_) break;
      // Note: We scan from MSbit to LSbit
      uint8_t sprite_val = (sprite_byte & (0x80 >> x)) >> (7-x);
      if (sprite_val > 0) {
        dots++;
      }
      pixel_was_disabled |= XOR(current_c, current_r, sprite_val);
    }
  }
  DBG(" %d dots", dots);
  return pixel_was_disabled;
}

bool Image::XOR(int c, int r, uint8_t val) {
  uint8_t& current_val = At(c, r);
  uint8_t prev_val = current_val;
  current_val ^= val;
  return current_val == 0 && prev_val >= 0;
}

 void Image::CopyToRGB24(uint8_t* dst, int red_scale, int green_scale, int blue_scale) {
  int cols = Cols();
  for (int row = 0; row < Rows(); row++) {
    for (int col = 0; col < cols; col++) {
      dst[(row * cols + col) * 3] = At(col, row) * red_scale;
      dst[(row * cols + col) * 3 + 1] = At(col, row) * green_scale;
      dst[(row * cols + col) * 3 + 2] = At(col, row) * blue_scale;
    }
  }
 }

void Image::Print() {
  for (int r = 0; r < rows_; r++) {
    for (int c = 0; c < cols_; c++) {
      std::cout << std::setfill('0') << std::setw(3) << static_cast<int>(At(c,r)) << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void Image::DrawToStdout() {
  for (int r = 0; r < rows_; r++) {
    for (int c = 0; c < cols_; c++) {
      if (At(c,r) > 0) {
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