// This file is distributed under a BSD license. See LICENSE.txt for details.
#pragma once
// FRIED
// internal interface (everything that shouldn't be visible to the public)

#ifndef __FRIED_INTERNAL_HPP__
#define __FRIED_INTERNAL_HPP__
#include <cstdint>
#include "types_updated.h"
namespace FRIED
{
  enum ChannelType : uint8_t
  {
    CHANNEL_NONE  = 0,
    CHANNEL_Y     = 1,
    CHANNEL_CO    = 2,
    CHANNEL_CG    = 3,
    CHANNEL_ALPHA = 4,
    // just allocate other channel types as required
  };

#pragma pack(push, 1)
  // channel header
  struct ChannelHeader
  {
    ChannelType Type;                  // channel type tag
    uint8_t Quantizer;                  // quantizer factor
    int32_t StripeOffset;              // offset in stripe data
    int32_t ChunkOffset;               // offset in chunk data
  };

// file header
  struct FileHeader
  {
    char Signature[8];             // FRIEDxxx (xxx=version number)
    int32_t XRes;                  // width of image
    int32_t YRes;                  // height of image
//    int32_t VirtualXRes;               // virtual width of image
    int32_t ChunkWidth;            // chunk width
    uint8_t Channels;              // # of channels used (max 16)
  };
#pragma pack(pop)

  // encode context
  struct EncodeContext
  {
    FileHeader FH;
    ChannelHeader Chans[16];
    int32_t XResPadded;
    int32_t YResPadded;
    int32_t *SB;                       // stripe buffer (32 lines)
    int32_t *QB;                       // quantized buffer (16 lines)
    int32_t *CK;                       // chunk (unquantized) buffer

    uint8_t *Bits;                      // packed buffer
    int32_t BitsLength;                // length of packed buffer

    const uint8_t *Image;               // source image pointer
    int32_t Flags;                     // encoding flags
  };

  // decode context
  struct DecodeContext
  {
    FileHeader FH;
    ChannelHeader Chans[16];
    int32_t XResPadded;
    int32_t YResPadded;
    int16_t *SB;                       // stripe buffer (32 lines)
    int32_t *QB;                       // quantized buffer (16 lines)
    int16_t *CK;                       // chunk (unquantized) buffer

      uint8_t *Image;                     // destination image pointer
    int32_t ChannelSetup;              // channel setup number
  };

  // entropy coding
  int32_t rlgrenc(uint8_t *bits,int32_t nbmax,int32_t *x,int32_t n,int32_t xminit);
  int32_t rlgrdec(const uint8_t *bits,int32_t nbmax,int16_t *y,int32_t n,int32_t xminit);

  // quantization
  int32_t newQuantize(int32_t qs,int32_t *x,int32_t npts,int32_t cwidth);
  void newDequantize(int32_t qs,int16_t *x,int32_t npts,int32_t cwidth);

  // transforms
  void ndct42D(int32_t *x0,int32_t *x1,int32_t *x2,int32_t *x3);
  void indct42D(int16_t *x0,int16_t *x1,int16_t *x2,int16_t *x3);
  
  void ndct42D_MB(int32_t *x0,int32_t *x,int32_t *x2,int32_t *x3);
  void indct42D_MB(int16_t *x0,int16_t *x1,int16_t *x2,int16_t *x3);

  void lbt4pre2x4(int32_t *x0,int32_t *x1);
  void lbt4post2x4(int16_t *x0,int16_t *x1);
  void lbt4pre4x2(int32_t *x0,int32_t *x1,int32_t *x2,int32_t *x3);
  void lbt4post4x2(int16_t *x0,int16_t *x1,int16_t *x2,int16_t *x3);
  void lbt4pre4x4(int32_t *x0,int32_t *x1,int32_t *x2,int32_t *x3);
  void lbt4post4x4(int16_t *x0,int16_t *x1,int16_t *x2,int16_t *x3);

  // pixel processing
  void gray_alpha_convert_dir(int32_t cols,int32_t colsPad,const uint8_t *src,int32_t *dst);
  void gray_alpha_convert_inv(int32_t cols,int32_t colsPad,const int16_t *src,uint8_t *dst);
  void gray_x_convert_dir(int32_t cols,int32_t colsPad,const uint8_t *src,int32_t *dst);
  void gray_x_convert_inv(int32_t cols,int32_t colsPad,const int16_t *src,uint8_t *dst);
  void color_alpha_convert_dir(int32_t cols,int32_t colsPad,const uint8_t *src,int32_t *dst);
  void color_alpha_convert_inv(int32_t cols,int32_t colsPad,const int16_t *src,uint8_t *dst);
  void color_x_convert_dir(int32_t cols,int32_t colsPad,const uint8_t *src,int32_t *dst);
  void color_x_convert_inv(int32_t cols,int32_t colsPad,const int16_t *src,uint8_t *dst);
}

#endif
