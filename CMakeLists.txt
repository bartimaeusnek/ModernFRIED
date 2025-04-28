cmake_minimum_required(VERSION 3.16)
project(fried_modern LANGUAGES CXX)

# Use C++17 or higher for modern features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings and treat warnings as errors (optional)
if(MSVC)
#    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -O3 -ffast-math)
endif()

# Add stb as header-only dependencies
include_directories(external)

# Add source files (you'll populate this list)
file(GLOB_RECURSE FRIED_SRC
    src/fried/*.cpp
    src/fried/*.c
    src/fried/*.hpp
    src/fried/*.h
)

add_library(fried_shared SHARED ${FRIED_SRC})
target_include_directories(fried_shared PUBLIC src)

add_library(fried STATIC ${FRIED_SRC})
target_include_directories(fried PUBLIC src)

# CLI Tool
add_executable(fried_codec_tool tools/fried_codec_tool.cpp)
target_link_libraries(fried_codec_tool fried)

# Tests
include(FetchContent)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

enable_testing()
add_executable(test_fried_codec tests/test_fried_codec.cpp)
target_link_libraries(test_fried_codec PRIVATE fried doctest::doctest)
add_custom_command(TARGET test_fried_codec POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/tests/test_image.png
        $<TARGET_FILE_DIR:test_fried_codec>/tests/test_image.png
)
add_test(NAME fried_codec_tests COMMAND test_fried_codec)

# Use folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(fried PROPERTIES FOLDER "Library")
set_target_properties(fried_codec_tool PROPERTIES FOLDER "Tools")
set_target_properties(test_fried_codec PROPERTIES FOLDER "Tests")