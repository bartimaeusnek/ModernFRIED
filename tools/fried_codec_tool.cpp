#include "fried/externalApi.h"

int main(int argc, char** argv) {
    if (argc > 5 || argc < 4) {
        std::cerr << "Usage: " << argv[0] << " [encode|decode] input output [compression if encode]\n";
        return 1;
    }

    std::string mode = argv[1];
    const char* inputPath = argv[2];
    const char* outputPath = argv[3];
    if (mode == "encode") {
        if (argc != 5){
            std::cerr << "Usage: " << argv[0] << " encode input output compression\n";
            return 1;
        }
        const char* compression = argv[4];
        const int_fast32_t quality = atoi(compression);
        return fried_encode(inputPath, outputPath, quality) ? 0 : 1;
    } else if (mode == "decode") {
        return fried_decode(inputPath, outputPath) ? 0 : 1;
    } else {
        std::cerr << "Invalid mode: " << mode << "\n";
        return 1;
    }
}
