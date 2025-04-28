#include "externalApi.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#pragma clang diagnostic pop

bool fried_encode(const char *inputPath, const char *outputPath, uint_fast8_t quality) {
    int width, height, channels;
    uint8_t *inputImage = stbi_load(inputPath, &width, &height, &channels, 4); // force RGBA
    if (!inputImage) {
        std::cerr << "Failed to load image: " << inputPath << "\n";
        return false;
    }

    int32_t outsize = 0;
    int32_t flags = FRIED_DEFAULT | FRIED_SAVEALPHA;     // use default settings
    uint8_t *friedData = SaveFRIED(inputImage, width, height, flags, quality, outsize);

    stbi_image_free(inputImage);

    if (!friedData || outsize <= 0) {
        std::cerr << "FRIED compression failed.\n";
        return false;
    }

    std::ofstream out(outputPath, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to write output: " << outputPath << "\n";
        delete[] friedData;
        return false;
    }

    out.write(reinterpret_cast<const char *>(friedData), outsize);
    delete[] friedData;
    return true;
}

bool fried_decode(const char *inputPath, const char *outputPath) {
    std::ifstream in(inputPath, std::ios::binary | std::ios::ate);
    if (!in) {
        std::cerr << "Failed to open FRIED file: " << inputPath << "\n";
        return false;
    }

    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);

    std::vector<uint8_t> friedData(size);
    if (!in.read(reinterpret_cast<char *>(friedData.data()), size)) {
        std::cerr << "Failed to read FRIED file\n";
        return false;
    }

    int32_t width = 0, height = 0, outsize = 0;
    uint8_t *decodedImage = nullptr;
    if (!LoadFRIED(friedData.data(), static_cast<int32_t>(size), width, height, outsize, decodedImage)) {
        std::cerr << "FRIED decompression failed.\n";
        return false;
    }

    if (!stbi_write_png(outputPath, width, height, 4, decodedImage, width * 4)) {
        std::cerr << "Failed to write PNG\n";
        free(decodedImage);
        return false;
    }

    free(decodedImage);
    return true;
}