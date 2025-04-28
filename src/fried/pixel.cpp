// This file is distributed under a BSD license. See LICENSE.txt for details.

// FRIED
// pixel processing innerloops.
// (color conversion and such)
#include "fried.hpp"
#include "fried_internal.hpp"

namespace FRIED
{
  // forward conversions
  void gray_alpha_convert_dir(int32_t cols,int32_t colsPad,const uint8_t *src,int32_t *dst)
  {
    int32_t *outY = dst;
    int32_t *outA = dst + colsPad;

    for(int32_t i=0;i<cols;i++)
    {
      *outY++ = (*src++ - 128) << 2;
      *outA++ = (*src++ - 128) << 2;
    }

    for(int32_t i=cols;i<colsPad;i++)
    {
      *outY++ = 0;
      *outA++ = 0;
    }
  }

  void gray_x_convert_dir(int32_t cols,int32_t colsPad,const uint8_t *src,int32_t *dst)
  {
    int32_t *outY = dst;

    for(int32_t i=0;i<cols;i++)
    {
      *outY++ = (*src++ - 128) << 2;
      src++; // skip unused alpha byte
    }

    for(int32_t i=cols;i<colsPad;i++)
      *outY++ = 0;
  }

  void color_alpha_convert_dir(int32_t cols,int32_t colsPad,const uint8_t *src,int32_t *dst)
  {
    int32_t *outY = dst;
    int32_t *outCo = dst + colsPad;
    int32_t *outCg = outCo + colsPad;
    int32_t *outA = outCg + colsPad;

    for(int32_t i=0;i<cols;i++)
    {
      int32_t b = (*src++ - 128) << 2;
      int32_t g = (*src++ - 128) << 2;
      int32_t r = (*src++ - 128) << 2;

      g <<= 1;
 
      *outY++ = (r + g + b + 2) >> 2;
      *outCo++ = (r - b) >> 1;
      *outCg++ = (g - r - b + 2) >> 2; 
      *outA++ = (*src++ - 128) << 2;
    }

    for(int32_t i=cols;i<colsPad;i++)
    {
      *outY++ = 0;
      *outCo++ = 0;
      *outCg++ = 0;
      *outA++ = 0;
    }
  }

  void color_x_convert_dir(int32_t cols,int32_t colsPad,const uint8_t *src,int32_t *dst)
  {
    int32_t *outY = dst;
    int32_t *outCo = dst + colsPad;
    int32_t *outCg = outCo + colsPad;

    for(int32_t i=0;i<cols;i++)
    {
      int32_t b = (*src++ - 128) << 2;
      int32_t g = (*src++ - 128) << 2;
      int32_t r = (*src++ - 128) << 2;
      src++; // ignore alpha

      g <<= 1;
 
      *outY++ = (r + g + b + 2) >> 2;
      *outCo++ = (r - b) >> 1;
      *outCg++ = (g - r - b + 2) >> 2; 
    }

    for(int32_t i=cols;i<colsPad;i++)
    {
      *outY++ = 0;
      *outCo++ = 0;
      *outCg++ = 0;
    }
  }

  // inverse conversions (optimize me!)
  static int clampPixel(int32_t a)
  {
    return (a < 0) ? 0 : (a > 255) ? 255 : a;
  }

  void gray_alpha_convert_inv(int32_t cols,int32_t colsPad,const int16_t *src,uint8_t *dst)
  {
    const int16_t *inY = src;
    const int16_t *inA = src + colsPad;

    for(int32_t i=0;i<cols;i++)
    {
      *dst++ = clampPixel((*inY++ >> 4) + 128);
      *dst++ = clampPixel((*inA++ >> 4) + 128);
    }
  }

  void gray_x_convert_inv(int32_t cols, int32_t,const int16_t *src,uint8_t *dst)
  {
    const int16_t *inY = src;

    for(int32_t i=0;i<cols;i++)
    {
      *dst++ = clampPixel((*inY++ >> 4) + 128);
      *dst++ = 255;
    }
  }

  void color_alpha_convert_inv(int32_t cols,int32_t colsPad,const int16_t *src,uint8_t *dst)
  {
    const int16_t *inY = src;
    const int16_t *inCo = src + colsPad;
    const int16_t *inCg = inCo + colsPad;
    const int16_t *inA = inCg + colsPad;

    for(int32_t i=0;i<cols;i++)
    {
      int32_t y = *inY++;
      int32_t co = *inCo++;
      int32_t cg = *inCg++;
      int32_t r,g,b;

      g = y + cg;
      b = y - cg;
      r = b + co;
      b = b - co;

      *dst++ = clampPixel((b >> 4) + 128);
      *dst++ = clampPixel((g >> 4) + 128);
      *dst++ = clampPixel((r >> 4) + 128);
      *dst++ = clampPixel((*inA++ >> 4) + 128);
    }
  }

  void color_x_convert_inv(int32_t cols,int32_t colsPad,const int16_t *src,uint8_t *dst)
  {
    const int16_t *inY = src;
    const int16_t *inCo = src + colsPad;
    const int16_t *inCg = inCo + colsPad;

    for(int32_t i=0;i<cols;i++)
    {
      int32_t y = *inY++;
      int32_t co = *inCo++;
      int32_t cg = *inCg++;
      int32_t r,g,b;

      g = y + cg;
      b = y - cg;
      r = b + co;
      b = b - co;

      *dst++ = clampPixel((b >> 4) + 128);
      *dst++ = clampPixel((g >> 4) + 128);
      *dst++ = clampPixel((r >> 4) + 128);
      *dst++ = 255;
    }
  }
}
