#ifndef C8_IMAGE_H_
#define C8_IMAGE_H_

#include "common.h"

// Monochrome image format.

class Image {
  public:
    // Allocs and de-allocs in ctor and dtor.
    Image(int cols, int rows);
    ~Image();

    uint8_t* Row(int r);

    // Returns a pixel that can be changed.
    uint8_t& operator()(int c, int r);

    void SetAll(uint8_t value);

    // XOR render sprite to image starting at top-left corner c,r.
    // Renders 1-byte of width and height rows. Returns whether or
    // not any pixels were set to 0 by this operation.
    bool XORSprite(int c, int r, int height, uint8_t* sprite);

    void Print();
    void DrawToStdout();

  private:
    int cols_;
    int rows_;

    uint8_t* data_;
};

#endif 