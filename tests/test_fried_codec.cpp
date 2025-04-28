#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "stb_image.h"
#include "stb_image_write.h"
#include "fried/externalApi.h"

TEST_CASE("FRIED encode/decode roundtrip") {
    auto input = "tests/test_image.png";
    auto tempFried = "tests/test.fried";
    auto output = "tests/test_result.png";

    // Call encode
    CHECK(fried_encode(input, tempFried, 31));

    // Call decode
    CHECK(fried_decode(tempFried, output));

    // Pixel-by-pixel comparison with threshold
    int origWidth, origHeight, origChannels;
    int decodedWidth, decodedHeight, decodedChannels;

    // Load the original and decoded images
    unsigned char* origData = stbi_load(input, &origWidth, &origHeight, &origChannels, 0);
    unsigned char* decodedData = stbi_load(output, &decodedWidth, &decodedHeight, &decodedChannels, 0);

    REQUIRE(origData != nullptr);
    REQUIRE(decodedData != nullptr);

    // Check dimensions match
    REQUIRE(origWidth == decodedWidth);
    REQUIRE(origHeight == decodedHeight);
    REQUIRE(origChannels == decodedChannels);

    // Calculate pixel differences
    double totalDiff = 0.0;
    size_t totalPixels = origWidth * origHeight * origChannels;

    for (size_t i = 0; i < totalPixels; i++) {
        int diff = std::abs(static_cast<int>(origData[i]) - static_cast<int>(decodedData[i]));
        totalDiff += diff;
    }

    // Calculate average difference per channel
    double avgDifference = totalDiff / totalPixels;


    MESSAGE("Average pixel difference: " << avgDifference);
    CHECK(abs(avgDifference - 0.419201) < 0.000001);

    // Free the image data
    stbi_image_free(origData);
    stbi_image_free(decodedData);
}

TEST_CASE("FRIED encode all levels roundtrip"){
    auto input = "tests/test_image.png";
    auto tempFried = "tests/test.fried";
    auto outputRaw = "tests/test_result_";

    for (int j = 0; j < 128; ++j) {
        std::string buf(outputRaw);
        buf.append(std::to_string(j));
        buf.append(".fried");
        auto output = buf.c_str();
        // Call encode
        CHECK(fried_encode(input, tempFried, j));

        // Call decode
        CHECK(fried_decode(tempFried, output));

        // Pixel-by-pixel comparison with threshold
        int origWidth, origHeight, origChannels;
        int decodedWidth, decodedHeight, decodedChannels;

        // Load the original and decoded images
        unsigned char* origData = stbi_load(input, &origWidth, &origHeight, &origChannels, 4);
        unsigned char* decodedData = stbi_load(output, &decodedWidth, &decodedHeight, &decodedChannels, 4);

        REQUIRE(origData != nullptr);
        REQUIRE(decodedData != nullptr);

        // Check dimensions match
        REQUIRE(origWidth == decodedWidth);
        REQUIRE(origHeight == decodedHeight);
        REQUIRE(origChannels == decodedChannels);

        // Calculate pixel differences
        double totalDiff = 0.0;
        size_t totalPixels = origWidth * origHeight * origChannels;

        for (size_t i = 0; i < totalPixels; i++) {
            int diff = std::abs(static_cast<int>(origData[i]) - static_cast<int>(decodedData[i]));
            totalDiff += diff;
        }

        // Calculate average difference per channel
        double avgDifference = totalDiff / totalPixels;
        MESSAGE("Average pixel difference: " << avgDifference << " @ " << j);
        stbi_image_free(origData);
        stbi_image_free(decodedData);
    }
}