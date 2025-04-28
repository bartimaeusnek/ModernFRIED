// This file is distributed under a BSD license. See LICENSE.txt for details.

// FRIED
// quantization functions.
#include "fried_internal.hpp"

namespace FRIED
{
  // ---- the quantization tables themselves
  static bool tablesInitialized = false;
  static int32_t qdescale[8][16];
  static int32_t qrescale[8][16];

  // ---- macroblock AC scanning pattern

  static int32_t zigzag2[16] = { 0,4,1,2,5,8,12,9,6,3,7,10,13,14,11,15 };

  // ---- transform row norms (2-norm)

  static float_t xformn[4] = { 1.0000f, 1.3260f, 1.0000f, 1.5104f };
  // exact values: 1.3260 ^= sqrt(3601/2048), 1.5104 ^= sqrt(73/32)

  // ---- helper functions

  static int32_t imul14(int32_t a, int32_t b)
  {
      int64_t result = (int64_t)a * (int64_t)b;
      result += 8192;
      result >>= 14;
      return (int32_t)result;
  }


  static int32_t descale(int32_t x,int32_t bias,int32_t factor,int32_t shift)
  {
    int32_t p = imul14(x,factor);
    int32_t scale = 1 << shift;
    int32_t realbias = (scale >> 1) - (bias >> 14);

    if(p >= 0)
      p += realbias;
    else
      p -= realbias - scale + 1;

    return p >> shift;
  }

  static void initQuantTables()
  {
    if(tablesInitialized)
      return;

    for(int32_t level=0;level<8;level++)
    {
      double_t factor = pow(2.0,level / 8.0);
      
      for(int32_t y=0;y<4;y++)
      {
        for(int32_t x=0;x<4;x++)
        {
          int32_t rescale = int32_t(16 * factor * xformn[x] * xformn[y] + 0.5);
          int32_t descale = (524288 + rescale) / (2 * rescale);

          qrescale[level][y*4+x] = rescale;
          qdescale[level][y*4+x] = descale;
        }
      }
    }

    tablesInitialized = true;
  }

  // ---- actual quantization functions

  int32_t newQuantize(int32_t qs,int32_t *x,int32_t npts,int32_t cwidth)
  {
    int32_t shift,*qtab,bias,i,f,n,*p;

    // prepare quantizer tables
    initQuantTables();

    shift = qs >> 3;
    qtab = qdescale[qs & 7];
    bias = 1024 << shift;

    // quantization by groups
    p = x;

    for(i=0;i<16;i++)
    {
      f = qtab[zigzag2[i]];

      for(n=0;n<cwidth;n++)
        *p++ = descale(*p,bias,f,shift);
    }

    // now find number of zeroes
    for(n=npts-1;n>=0;n--)
      if(x[n])
        break;

    return n+1;
  }

  static void rescaleLoop(int16_t *x,int32_t count,int32_t f,int32_t shift)
  {
    if(shift < 4)
    {
      shift = 4 - shift;

      for(int32_t i=0;i<count;i++)
        x[i] = (x[i] * f) >> shift;
    }
    else
    {
      shift -= 4;

      for(int32_t i=0;i<count;i++)
        x[i] = (x[i] * f) << shift;
    }
  }

  void newDequantize(int32_t qs,int16_t *x,int32_t npts,int32_t cwidth)
  {
    int32_t shift,*qtab,i,f,count;

    // prepare quantizer tables
    initQuantTables();

    shift = qs >> 3;
    qtab = qrescale[qs & 7];

    // dequantization by subbands
    for(i=0;i<16;i++)
    {
      f = qtab[zigzag2[i]];
      count = sMin(npts,cwidth);

      if(count)
      {
        rescaleLoop(x,count,f,shift);
        x += count;
        npts -= count;
      }
    }
  }
}
