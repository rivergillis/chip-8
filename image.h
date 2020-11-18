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
    uint8_t& At(int c, int r);
    uint8_t& operator()(int c, int r) { return At(c, r); }

    void SetAll(uint8_t value);

    // XOR render sprite to image starting at top-left corner c,r.
    // c,r must be within the image. Up to 1-byte of width and up
    // to height rows will be rendered, depending on the image bounds.
    // Returns whether or not any pixels were set to 0 by this operation.
    bool XORSprite(int c, int r, int height, uint8_t* sprite);

    int Cols() { return cols_; }
    int Rows() { return rows_; }

    // The size of the allocated output buffer is exactly
    // Cols() * Rows() * 3.
    // Formatted interleaved RGBRGBRGB...
    void CopyToRGB24(uint8_t* dst, int red_scale, int green_scale, int blue_scale);

    void Print();
    void DrawToStdout();

  private:
    // XOR a pixel with val. Returns true if the pixel was set to 0.
    bool XOR(int c, int r, uint8_t val);

    int cols_;
    int rows_;

    uint8_t* data_;
};

#endif 