#pragma once
#ifndef FRIED_MODERN_EXTERNALAPI_H
#define FRIED_MODERN_EXTERNALAPI_H
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include "fried.hpp"

#if defined(_WIN32) || defined(WIN32)
#define exportAttrib __declspec(dllexport)
#else
#define exportAttrib
#endif

#ifdef __cplusplus
extern "C" {
#endif
exportAttrib bool fried_encode(const char* inputPath, const char* outputPath, uint_fast8_t quality);
exportAttrib bool fried_decode(const char* inputPath, const char* outputPath);
#ifdef __cplusplus
}
#endif

#endif //FRIED_MODERN_EXTERNALAPI_H
