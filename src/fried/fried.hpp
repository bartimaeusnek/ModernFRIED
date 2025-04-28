// This file is distributed under a BSD license. See LICENSE.txt for details.
#pragma once

#ifndef __FRIED_HPP__
#define __FRIED_HPP__
#include <cstdint>
#include "types_updated.h"
// Farbrausch Image Encoder/Decoder

// Save options
#define FRIED_DEFAULT         0x0000
#define FRIED_GRAYSCALE       0x0001
#define FRIED_SAVEALPHA       0x0002
//#define FRIED_CHROMASUBSAMPLE 0x0004 // not implemented yet

#define FRIED_FILE_VERSION "FRIED002"
#if defined(_WIN32) || defined(WIN32)
#define exportAttrib __declspec(dllexport)
#else
#define exportAttrib
#endif

// Loading/saving
#ifdef __cplusplus
extern "C" {
#endif
[[maybe_unused]] exportAttrib const char* getSupportedFileVersion();
    // Loading/saving
exportAttrib bool LoadFRIED(const uint8_t *data,int32_t size,int32_t &xout,int32_t &yout, int32_t &outSize, uint8_t *&dataout);
exportAttrib uint8_t *SaveFRIED(const uint8_t *image, int32_t xsize, int32_t ysize, int32_t flags, uint8_t quality, int32_t &outsize);
exportAttrib void FreeFRIED(const uint8_t* allocated);
#ifdef __cplusplus
}
#endif
#endif
