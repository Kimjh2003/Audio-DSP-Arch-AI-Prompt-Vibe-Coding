cmake_minimum_required(VERSION 3.22)
project(air_microsoc_runtime LANGUAGES CXX)

add_library(air_microsoc_runtime STATIC
    src/audio_format.cpp
    src/realtime_scheduler.cpp
    src/microsoc_runtime.cpp)
target_include_directories(air_microsoc_runtime PUBLIC include)
target_compile_features(air_microsoc_runtime PUBLIC cxx_std_20)
target_compile_options(air_microsoc_runtime PRIVATE -Wall -Wextra -Werror)

option(AIR_BUILD_TESTS "Build AIR MicroSoC tests" OFF)
if (AIR_BUILD_TESTS)
    enable_testing()
    add_executable(air_microsoc_tests tests/air_microsoc_tests.cpp)
    target_link_libraries(air_microsoc_tests PRIVATE air_microsoc_runtime)
    add_test(NAME air_microsoc_tests COMMAND air_microsoc_tests)
endif()
