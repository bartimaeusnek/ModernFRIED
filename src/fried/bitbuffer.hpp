// This file is distributed under a BSD license. See LICENSE.txt for details.

// FRIED
// lowlevel bitstream functions.
// quite fast.
#pragma once
#ifndef __BITBUFFER_HPP__
#define __BITBUFFER_HPP__

namespace FRIED
{
  // encoder
  class BitEncoder
  {
	  uint8_t *bp;
	  int32_t length;
	  int32_t acck;
	  int32_t accb;
	  int32_t nbytes;

  public:
    void Init(uint8_t *bytes,int32_t len)
    {
      bp = bytes;
      length = len;
      nbytes = 0;
      acck = 0;
      accb = 0;
    }

    void Flush()
    {
      if(acck)
        PutBits(0,8-acck);

      acck = 0;
      accb = 0;
    }

    void PutByte(int32_t code)
    {
      static const int32_t mask[8] = { 0,1,3,7,15,31,63,127 };

      if(nbytes < length)
      {
        code &= 0xff;
        *bp++ = (accb << (8 - acck)) | (code >> acck);
        accb = code & mask[acck];
        nbytes++;
      }
    }

    void PutBits(int32_t code,int32_t nb)
    {
      static const int32_t mask[8] = { 0,1,3,7,15,31,63,127 };

      if(nbytes < length)
      {
        while(nb >= 8)
        {
          nb -= 8;
          PutByte(code >> nb);
        }

        if(nb)
        {
          code &= mask[nb];
          acck += nb;
          accb = (accb << nb) | code;

          if(acck >= 8)
          {
            acck -= 8;
            *bp++ = accb >> acck;
            accb &= mask[acck];
            nbytes++;
          }
        }
      }
    }

    int32_t BytesWritten() const
    {
      return nbytes;
    }
  };

#if 0
  // encoder.
  class BitEncoder
  {
    uint8_t *Bytes;
    int32_t BytePos;
    int32_t Length;

    uint32_t Bits;
    int32_t BitPos;

  public:
    void Init(uint8_t *bytes,int32_t length)
    {
      Bytes = bytes;
      Length = length;
      BytePos = 0;
      Bits = 0;
      BitPos = 32;
    }

    void Flush()
    {
      if(BytePos < Length && BitPos != 32)
        Bytes[BytePos++] = (uint8_t) (Bits >> 24);

      BitPos = 32;
    }

    void PutBits(uint32_t value,int32_t nBits)
    {
      BitPos -= nBits;
      Bits |= value << BitPos;

      if(BitPos <= 24 && BytePos < Length)
      {
        do
        {
          Bytes[BytePos++] = (uint8_t) (Bits >> 24);
          Bits <<= 8;
          BitPos += 8;
        }
        while(BitPos <= 24 && BytePos < Length);
      }
    }

    int32_t BytesWritten() const
    {
      return BytePos;
    }
  };
#endif

  // decoder.
  class BitDecoder
  {
    const uint8_t *Bytes;
    const uint8_t *BytePtr;
    const uint8_t *ByteEnd;
    uint32_t Bits;
    int32_t BitFill;

    uint32_t Mask[25];

    inline void Refill()
    {
      while(BitFill <= 24)
      {
        Bits = (Bits << 8) | *BytePtr;
        BytePtr++;
        BitFill += 8;
      }
    }

  public:
    void Init(const uint8_t *bytes,int32_t length)
    {
      Bytes = bytes;
      BytePtr = Bytes;
      ByteEnd = Bytes + length;

      Bits = 0;
      BitFill = 0;
      Refill();

      for(int32_t i=0;i<=24;i++)
        Mask[i] = (1 << i) - 1;
    }

    inline void SkipBitsNoCheck(int32_t nBits)
    {
      BitFill -= nBits;
    }

    inline void SkipBits(int32_t nBits)
    {
      SkipBitsNoCheck(nBits);
      Refill();
    }

    inline uint32_t PeekBits(int32_t nBits)
    {
      return (Bits >> (BitFill - nBits)) & Mask[nBits];
    }

    inline uint32_t GetBitsNoCheck(int32_t nBits)
    {
      return (Bits >> (BitFill -= nBits)) & Mask[nBits];
    }

    inline uint32_t GetBits(int32_t nBits)
    {
      uint32_t val = (Bits >> (BitFill -= nBits)) & Mask[nBits];
      Refill();

      return val;
    }

    int32_t BytesRead() const
    {
      return int32_t(BytePtr - Bytes) - (BitFill >> 3);
    }
  };
}

#endif
