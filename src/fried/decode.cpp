// This file is distributed under a BSD license. See LICENSE.txt for details.

// FRIED
// decoding functions.
#include "fried.hpp"
#include "fried_internal.hpp"

//#include <crtdbg.h>

namespace FRIED
{
  static void writeBitmapRow(DecodeContext &ctx,int32_t row,int16_t *srp)
  {
    if(row < 0 || row >= ctx.FH.YRes)
      return;

    int32_t cols = ctx.FH.XRes;
    int32_t colsPad = ctx.XResPadded;
    uint8_t *dst = ctx.Image;

    if(ctx.ChannelSetup < 2) // grayscale
    {
      dst += row * 2 * cols;

      if(ctx.ChannelSetup == 0)
        gray_x_convert_inv(cols,colsPad,srp,dst);
      else
        gray_alpha_convert_inv(cols,colsPad,srp,dst);
    }
    else
    {
      dst += row * 4 * cols;

      if(ctx.ChannelSetup == 2)
        color_x_convert_inv(cols,colsPad,srp,dst);
      else if(ctx.ChannelSetup == 3)
        color_alpha_convert_inv(cols,colsPad,srp,dst);
    }

    // TODO: write the corresponding row using the correct innerloop
  }


// Simulate MMX registers (each holding 4 int16_t values)
    typedef struct {
        int16_t w[4];
    } MMXReg;

/**
 * Emulates the MMX load operations from the original macro
 * Loads 4 registers (64 bits each) from 4 source pointers with an offset
 */
    static void mm_load4(MMXReg *reg0, MMXReg *reg1, MMXReg *reg2, MMXReg *reg3,
                         int16_t *c0, int16_t *c1, int16_t *c2, int16_t *c3, int offset) {
        // Convert byte offset to 16-bit element offset
        int elemOffset = offset / sizeof(int16_t);

        // Load 4 consecutive int16_t values (64 bits) from each source
        memcpy(reg0->w, c0 + elemOffset, sizeof(MMXReg));
        memcpy(reg1->w, c1 + elemOffset, sizeof(MMXReg));
        memcpy(reg2->w, c2 + elemOffset, sizeof(MMXReg));
        memcpy(reg3->w, c3 + elemOffset, sizeof(MMXReg));
    }

/**
 * Emulates the MMX store operations from the original macro
 * Stores 4 registers to calculated destination addresses
 */
    static void mm_store4(MMXReg *reg0, MMXReg *reg1, MMXReg *reg2, MMXReg *reg3,
                          int16_t **dest, int32_t xOffs, uint8_t rc0, uint8_t rc1, uint8_t rc2, uint8_t rc3) {
        // Calculate destination pointers as in the original macro
        int16_t *dst0 = dest[(rc0 >> 4)] + xOffs + (rc0 & 0xf);
        int16_t *dst1 = dest[(rc1 >> 4)] + xOffs + (rc1 & 0xf);
        int16_t *dst2 = dest[(rc2 >> 4)] + xOffs + (rc2 & 0xf);
        int16_t *dst3 = dest[(rc3 >> 4)] + xOffs + (rc3 & 0xf);

        // Store 4 consecutive int16_t values (64 bits) to each destination
        memcpy(dst0, reg0->w, sizeof(MMXReg));
        memcpy(dst1, reg1->w, sizeof(MMXReg));
        memcpy(dst2, reg2->w, sizeof(MMXReg));
        memcpy(dst3, reg3->w, sizeof(MMXReg));
    }

/**
 * Emulates the MMX transpose operations from the original macro
 * Transposes a 4x4 matrix of 16-bit values stored in 4 MMX registers
 */
    static void mm_transpose(MMXReg *reg0, MMXReg *reg1, MMXReg *reg2, MMXReg *reg3,
                             MMXReg *reg4, MMXReg *) {
        // Temporary registers to hold intermediate values
        MMXReg tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;

        // Copy values to preserve originals as needed
        memcpy(&tmp4, reg0, sizeof(MMXReg));
        memcpy(&tmp5, reg2, sizeof(MMXReg));

        // punpcklwd - unpack and interleave low words
        tmp0.w[0] = reg0->w[0]; tmp0.w[1] = reg1->w[0];
        tmp0.w[2] = reg0->w[1]; tmp0.w[3] = reg1->w[1];

        // punpckhwd - unpack and interleave high words
        tmp4.w[0] = reg0->w[2]; tmp4.w[1] = reg1->w[2];
        tmp4.w[2] = reg0->w[3]; tmp4.w[3] = reg1->w[3];

        // punpcklwd for reg2/reg3
        tmp2.w[0] = reg2->w[0]; tmp2.w[1] = reg3->w[0];
        tmp2.w[2] = reg2->w[1]; tmp2.w[3] = reg3->w[1];

        // punpckhwd for reg2/reg3 (using tmp5 which was set to reg2)
        tmp5.w[0] = reg2->w[2]; tmp5.w[1] = reg3->w[2];
        tmp5.w[2] = reg2->w[3]; tmp5.w[3] = reg3->w[3];

        // Copy tmp0 and tmp4
        memcpy(&tmp1, &tmp0, sizeof(MMXReg));
        memcpy(&tmp3, &tmp4, sizeof(MMXReg));

        // punpckldq - unpack and interleave low double words
        reg0->w[0] = tmp0.w[0]; reg0->w[1] = tmp0.w[1];
        reg0->w[2] = tmp2.w[0]; reg0->w[3] = tmp2.w[1];

        // punpckhdq - unpack and interleave high double words
        reg1->w[0] = tmp1.w[2]; reg1->w[1] = tmp1.w[3];
        reg1->w[2] = tmp2.w[2]; reg1->w[3] = tmp2.w[3];

        // punpckldq for tmp4/tmp5
        reg4->w[0] = tmp4.w[0]; reg4->w[1] = tmp4.w[1];
        reg4->w[2] = tmp5.w[0]; reg4->w[3] = tmp5.w[1];

        // punpckhdq for tmp4/tmp5
        reg3->w[0] = tmp3.w[2]; reg3->w[1] = tmp3.w[3];
        reg3->w[2] = tmp5.w[2]; reg3->w[3] = tmp5.w[3];
    }

/**
 * Main function that shuffles 4 arrays of 16-bit integers
 */
    void shuffle4x16(int16_t **dest, int32_t xOffs, int16_t *c0, int16_t *c1, int16_t *c2, int16_t *c3) {
        // MMX registers
        MMXReg mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;

        // First set of operations
        mm_load4(&mm0, &mm1, &mm2, &mm3, c0, c1, c2, c3, 0);
        mm_transpose(&mm0, &mm1, &mm2, &mm3, &mm4, &mm5);

        mm_load4(&mm2, &mm5, &mm6, &mm7, c0, c1, c2, c3, 8);
        mm_store4(&mm0, &mm1, &mm4, &mm3, dest, xOffs, 0x00, 0x04, 0x44, 0x40);

        mm_transpose(&mm2, &mm5, &mm6, &mm7, &mm0, &mm1);
        mm_load4(&mm1, &mm3, &mm4, &mm6, c0, c1, c2, c3, 16);
        mm_store4(&mm2, &mm5, &mm0, &mm7, dest, xOffs, 0x80, 0xc0, 0xc4, 0x84);

        mm_transpose(&mm1, &mm3, &mm4, &mm6, &mm2, &mm5);
        mm_load4(&mm0, &mm4, &mm5, &mm7, c0, c1, c2, c3, 24);
        mm_store4(&mm1, &mm3, &mm2, &mm6, dest, xOffs, 0x88, 0xc8, 0xcc, 0x8c);

        mm_transpose(&mm0, &mm4, &mm5, &mm7, &mm1, &mm3);
        mm_store4(&mm0, &mm4, &mm1, &mm7, dest, xOffs, 0x4c, 0x48, 0x08, 0x0c);
    }

  static void inv_reorder(int16_t **dest,int32_t xOffs,int16_t *src,int32_t cwidth)
  {
    int32_t nmb = cwidth/16;
    int16_t *g0,*g1,*g2,*g3;
    int32_t mb;

    // first row of block AC coeffs+DC
    g0 = src;
    g1 = src +  2 * cwidth;
    g2 = src +  3 * cwidth;
    g3 = src +  9 * cwidth;
    for(mb=0;mb<cwidth;mb+=16)
    {
      int16_t dcs[16];
      int16_t *gp = g0;

      // get dc coeffs (with reordering)
      dcs[ 0] = *gp; gp += nmb;
      dcs[ 3] = *gp; gp += nmb;
      dcs[ 1] = *gp; gp += nmb;
      dcs[14] = *gp; gp += nmb;

      dcs[ 2] = *gp; gp += nmb;
      dcs[ 4] = *gp; gp += nmb;
      dcs[ 5] = *gp; gp += nmb;
      dcs[ 7] = *gp; gp += nmb;

      dcs[13] = *gp; gp += nmb;
      dcs[15] = *gp; gp += nmb;
      dcs[12] = *gp; gp += nmb;
      dcs[ 8] = *gp; gp += nmb;

      dcs[ 6] = *gp; gp += nmb;
      dcs[ 9] = *gp; gp += nmb;
      dcs[11] = *gp; gp += nmb;
      dcs[10] = *gp; g0++;

      // shuffle
      shuffle4x16(dest+0,xOffs+mb,dcs,g1+mb,g2+mb,g3+mb);
    }

    // second rows of block AC coeffs
    g0 = src +  1 * cwidth;
    g1 = src +  4 * cwidth;
    g2 = src +  8 * cwidth;
    g3 = src + 10 * cwidth;
    for(mb=0;mb<cwidth;mb+=16)
      shuffle4x16(dest+1,xOffs+mb,g0+mb,g1+mb,g2+mb,g3+mb);

    // third rows of block AC coeffs
    g0 = src +  5 * cwidth;
    g1 = src +  7 * cwidth;
    g2 = src + 11 * cwidth;
    g3 = src + 14 * cwidth;
    for(mb=0;mb<cwidth;mb+=16)
      shuffle4x16(dest+2,xOffs+mb,g0+mb,g1+mb,g2+mb,g3+mb);

    // fourth rows of block AC coeffs
    g0 = src +  6 * cwidth;
    g1 = src + 12 * cwidth;
    g2 = src + 13 * cwidth;
    g3 = src + 15 * cwidth;
    for(mb=0;mb<cwidth;mb+=16)
      shuffle4x16(dest+3,xOffs+mb,g0+mb,g1+mb,g2+mb,g3+mb);
  }

  static int32_t decodeStripe(DecodeContext &ctx,int32_t cols,int32_t,const uint8_t *byteStart,int32_t maxbytes,int16_t **srp)
  {
    int32_t cjs[16];
    int32_t cwidth = ctx.FH.ChunkWidth;
    int32_t nchunks = (cols + cwidth - 1) / cwidth;
    int16_t *g0;
    const uint8_t *bytes,*bytesEnd;

    bytes = byteStart;
    bytesEnd = bytes + maxbytes;

    // clear chunk positions
    for(int32_t ch=0;ch<ctx.FH.Channels;ch++)
      cjs[ch] = 0;

    // process this stripe chunk by chunk
    for(int32_t chunk=0,ncc=0;chunk<nchunks;chunk++,ncc+=cwidth)
    {
      // adjust width for last chunk
      if(chunk == nchunks-1)
        cwidth = cols - ncc;

      // read chunk length
      if(bytes + 2 > bytesEnd)
        return -1;

      const uint8_t *bytesChunkEnd = bytes + (bytes[0] + (bytes[1] << 8));
      bytes += 2;

      // process channels
      for(int32_t ch=0;ch<ctx.FH.Channels;ch++)
      {
        int32_t so = ctx.Chans[ch].StripeOffset;
        int32_t co = ctx.Chans[ch].ChunkOffset;
        int32_t qs = ctx.Chans[ch].Quantizer;
        int32_t cksize = cwidth * 16;

        // read number of encoded coeffs
        int32_t encsize;

        if(bytes >= bytesEnd)
          return -1;

        if(*bytes & 1) // long code
        {
          if(bytes + 1 >= bytesEnd)
            return -1;

          encsize = ((bytes[0] + (bytes[1] << 8)) & ~1) * 4;
          bytes += 2;
        }
        else // short code
          encsize = *bytes++ * 4;

        // decode coefficients
        g0 = ctx.CK + co;
        sSetMem(g0,0,cksize * sizeof(int16_t));

        if(encsize)
        {
          int32_t xminit,nbs;

          xminit = 625 >> (qs >> 3);
          nbs = rlgrdec(bytes,bytesEnd - bytes,g0,sMin(encsize,cwidth),xminit);
          if(nbs < 0)
            return -1;
          else
            bytes += nbs;

          if(encsize > cwidth)
          {
            xminit = 94 >> (qs >> 3);
            nbs = rlgrdec(bytes,bytesEnd - bytes,g0+cwidth,encsize-cwidth,xminit);
            if(nbs < 0)
              return -1;
            else
              bytes += nbs;
          }
        }

        // un-delta dc coefficients
        int32_t nmb = sMin(encsize,cwidth/16);

        int32_t n = 0;
        while(++n < nmb)
          g0[n] += g0[n-1];

        // dequantize, undo reordering
        newDequantize(qs,g0,encsize,cwidth);
        inv_reorder(srp+16,so + cjs[ch],g0,cwidth);

        // this channel is done
        cjs[ch] += cwidth;
      }

      if(bytes != bytesChunkEnd)
        return -1;
    }

    return bytes - byteStart;
  }

  static void ihlbt_group1(int32_t swidth,int32_t so,int16_t **srp)
  {
    int16_t *p0,*p1,*p2,*p3;
    int32_t col;
    
    // first row of macroblocks
    p0 = srp[16] + so;
    p1 = srp[20] + so;
    p2 = srp[24] + so;
    p3 = srp[28] + so;

    for(col=0;col<swidth;col+=16)
      indct42D_MB(p0+col,p1+col,p2+col,p3+col);

    // first row
    p0 = srp[16] + so;
    p1 = srp[17] + so;
    p2 = srp[18] + so;
    p3 = srp[19] + so;
    indct42D(p0,p1,p2,p3);

    // rescale top left 2x2 pixels
    p0[0] <<= 2;
    p0[1] <<= 2;
    p1[0] <<= 2;
    p1[1] <<= 2;

    for(int32_t col=4;col<swidth;col+=4)
    {
      indct42D(p0+col,p1+col,p2+col,p3+col);
      lbt4post2x4(p0+col-2,p1+col-2);
    }

    // rescale top right 2x2 pixels
    p0[swidth-2] <<= 2;
    p0[swidth-1] <<= 2;
    p1[swidth-2] <<= 2;
    p1[swidth-1] <<= 2;
  }

  static void ihlbt_group2(int32_t swidth,int32_t so,int16_t **srp,bool)
  {
    // macroblocks only
    int16_t *p0,*p1,*p2,*p3;
    int32_t col;

    p0 = srp[16] + so;
    p1 = srp[20] + so;
    p2 = srp[24] + so;
    p3 = srp[28] + so;

    for(col=0;col<swidth;col+=16)
      indct42D_MB(p0+col,p1+col,p2+col,p3+col);
  }

  static void ihlbt_group3(int32_t swidth,int32_t so,int32_t ib,int16_t **srp,bool fbot)
  {
    // normal rows only
    int16_t *pa,*pb,*p0,*p1,*p2,*p3;
    int32_t col;

    pa = srp[ib+0] + so;
    pb = srp[ib+1] + so;
    p0 = srp[ib+2] + so;
    p1 = srp[ib+3] + so;
    p2 = srp[ib+4] + so;
    p3 = srp[ib+5] + so;

    indct42D(p0,p1,p2,p3);
    lbt4post4x2(pa,pb,p0,p1);

    if(fbot) // rescale bottom left 2x2 pixels
    {
      p2[0] <<= 2;
      p2[1] <<= 2;
      p3[0] <<= 2;
      p3[1] <<= 2;
    }

    for(col=4;col<swidth;col+=4)
    {
      indct42D(p0+col,p1+col,p2+col,p3+col);
      lbt4post4x4(pa+col-2,pb+col-2,p0+col-2,p1+col-2);

      if(fbot)
        lbt4post2x4(p2+col-2,p3+col-2);
    }

    lbt4post4x2(pa+col-2,pb+col-2,p0+col-2,p1+col-2);

    // rescale bottom right 2x2 pixels
    if(fbot)
    {
      p2[swidth-2] <<= 2;
      p2[swidth-1] <<= 2;
      p3[swidth-2] <<= 2;
      p3[swidth-1] <<= 2;
    }
  }

  static int32_t updatebp(int16_t **srp,int16_t *sb,int32_t fr,int32_t width,int32_t mode)
  {
    fr = mode ? 0 : (fr ^ 16);
    for(int32_t i=0;i<32;i++)
      srp[i] = sb + ((fr + i) & 31) * width;

    return fr;
  }

  static int32_t PerformDecode(DecodeContext &ctx,const uint8_t *bitsStart,int32_t nbytes)
  {
    int16_t *srp[32];
    int32_t fr,ib,k;
    const uint8_t *bits,*bitsEnd;
    int32_t cols,rows,chans;
    int32_t stsize;

    cols = ctx.XResPadded;
    rows = ctx.YResPadded;
    chans = ctx.FH.Channels;
    stsize = chans * cols;

    bits = bitsStart;
    bitsEnd = bits + nbytes;

    // actual decoding loop
    fr = updatebp(srp,ctx.SB,0,stsize,1);
    ib = 16;
    k = 2;

    for(int32_t row=0;row<rows;row++,k++,ib++)
    {
      if(row == 0)
      {
        int32_t sizeStripe = decodeStripe(ctx,cols,chans,bits,bitsEnd - bits,srp);
        if(sizeStripe < 0)
          return -1;

        bits += sizeStripe;
        
        for(int32_t ch=0;ch<chans;ch++)
          ihlbt_group1(cols,ctx.Chans[ch].StripeOffset,srp);
      }

      if(ib == 16)
      {
        fr = updatebp(srp,ctx.SB,fr,stsize,0);
        ib = 0;

        if(row != rows - 16)
        {
          bool bot = (row == rows - 32);
          int32_t sizeStripe = decodeStripe(ctx,cols,chans,bits,bitsEnd - bits,srp);
          if(sizeStripe < 0)
            return -1;

          bits += sizeStripe;

          for(int32_t ch=0;ch<chans;ch++)
            ihlbt_group2(cols,ctx.Chans[ch].StripeOffset,srp,bot);
        }
      }

      if(k == 4 && row != rows - 2)
      {
        bool bot = (row == rows - 6);

        for(int32_t ch=0;ch<chans;ch++)
          ihlbt_group3(cols,ctx.Chans[ch].StripeOffset,ib,srp,bot);

        k = 0;
      }

      writeBitmapRow(ctx,row,srp[ib]);
    }

    return bitsEnd - bits;
  }
}

using namespace FRIED;

[[maybe_unused]] const char* getSupportedFileVersion()
{
    return FRIED_FILE_VERSION;
}
void FreeFRIED(const uint8_t* allocated)
{
    delete[] allocated;
}
bool LoadFRIED(const uint8_t *data,int32_t size,int32_t &xout,int32_t &yout, int32_t &outSize, uint8_t *&dataout)
{
  DecodeContext ctx;
  const uint8_t *dataEnd = data + size;

  // check file format
  if(static_cast<size_t>(size) < sizeof(FileHeader))
    return false;

  // check signature
  if(sCmpMem(data,FRIED_FILE_VERSION,8))
    return false;

  // copy header over
  sCopyMem(&ctx.FH,data,sizeof(FileHeader));
  data += sizeof(FileHeader);

  // check number of channels and copy channel headers over
  if(ctx.FH.Channels > 16)
    return false;

  sCopyMem(ctx.Chans,data,ctx.FH.Channels * sizeof(ChannelHeader));
  data += ctx.FH.Channels * sizeof(ChannelHeader);

  // calculate some important constants
  int32_t chans = ctx.FH.Channels;

  ctx.XResPadded = (ctx.FH.XRes + 31) & ~31;
  ctx.YResPadded = (ctx.FH.YRes + 31) & ~31;
  int32_t sbw = chans * ctx.XResPadded;
  int32_t cbw = chans * ctx.FH.ChunkWidth;

  ctx.SB = new int16_t[sbw * 32];
  ctx.QB = new int32_t[cbw * 16];
  ctx.CK = new int16_t[cbw * 16];

  // determine channel setup (rather faked at the moment)
  if(chans <= 1 || ctx.Chans[0].Type != CHANNEL_Y)
    return false;

  if(chans == 1)
    ctx.ChannelSetup = 0; // gray w/out alpha
  else if(chans == 2 && ctx.Chans[1].Type == CHANNEL_ALPHA)
    ctx.ChannelSetup = 1; // gray w/ alpha
  else if(chans >= 3 && ctx.Chans[1].Type == CHANNEL_CO && ctx.Chans[2].Type == CHANNEL_CG)
  {
    if(chans == 3)
      ctx.ChannelSetup = 2; // color w/out alpha
    else if(chans == 4 && ctx.Chans[3].Type == CHANNEL_ALPHA)
      ctx.ChannelSetup = 3; // color w/ alpha
    else
      return false;
  }
  else
    return false;

  outSize = ctx.FH.XRes * ctx.FH.YRes * (ctx.ChannelSetup >= 2 ? 4 : 2);
  // allocate image
  ctx.Image = new uint8_t[outSize];

  // decode
  if(PerformDecode(ctx,data,dataEnd - data) >= 0)
  {
    xout = ctx.FH.XRes;
    yout = ctx.FH.YRes;
    dataout = ctx.Image;
  }
  else
  {
    xout = 0;
    yout = 0;
    dataout = nullptr;
    outSize = 0;
    delete[] ctx.Image;
  }

  // free everything
  delete[] ctx.SB;
  delete[] ctx.QB;
  delete[] ctx.CK;

  return dataout != nullptr;
}
