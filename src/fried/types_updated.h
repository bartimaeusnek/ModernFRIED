#pragma once
#ifndef FRIED_MODERN_TYPES_UPDATED_H
#define FRIED_MODERN_TYPES_UPDATED_H
#include <cstdint>
#include <cmath>
#include <memory.h>

inline int32_t sAbs(int32_t i)                                 { return abs(i); }
inline void sSetMem(void* dd, int32_t s, int32_t c)               { memset(dd,s,c); }
inline void sCopyMem(void* dd,const void *ss,int32_t c)      { memcpy(dd,ss,c); }
inline int32_t sCmpMem(const void *dd,const void *ss,int32_t c) { return (int32_t)memcmp(dd,ss,c); }

template <class Type> inline Type sMin(Type a,Type b)            {return (a<b) ? a : b;}
template <class Type> inline Type sMax(Type a,Type b)            {return (a>b) ? a : b;}
template <class Type> inline Type sSign(Type a)                  {return (a==0) ? 0 : (a>0) ? 1 : -1;}
template <class Type> inline Type sRange(Type a,Type max,Type min) {return (a>=max) ? max : (a<=min) ? min : a;}
template <class Type> inline void sSwap(Type &a,Type &b)         {Type s; s=a; a=b; b=s;}
template <class Type> inline Type sAlign(Type a,int32_t b)          {return (Type)((((int32_t)a)+b-1)&(~(b-1)));}

template <class Type> inline Type sSquare(Type a)                {return a*a;}

template <class Type> inline void sDelete(Type &a)               {if(a) delete a; a=0;}
template <class Type> inline void sDeleteArray(Type &a)          {if(a) delete[] a; a=0;}
template <class Type> inline void sRelease(Type &a)              {if(a) a->Release(); a=0;}
#endif //FRIED_MODERN_TYPES_UPDATED_H
