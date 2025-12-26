cmake_minimum_required(VERSION 3.23)

project(ArbitraryInteger VERSION 1.0 LANGUAGES CXX)

add_executable(ArbitraryInteger)

target_compile_features(ArbitraryInteger PRIVATE cxx_std_23)

include(FetchContent)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.12
)
FetchContent_MakeAvailable(doctest)

if(MSVC)
    target_compile_options(ArbitraryInteger PRIVATE /W4 /WX /EHsc /utf-8)
else()
    target_compile_options(ArbitraryInteger PRIVATE -Wall -Wextra -Wpedantic -march=native)
endif()

target_sources(ArbitraryInteger 
    PRIVATE 
        main.cpp
)

target_sources(ArbitraryInteger
    PUBLIC
        FILE_SET HEADERS
        FILES 
            ArbitraryInteger.hpp
)

target_link_libraries(ArbitraryInteger PRIVATE doctest::doctest)

enable_testing()
add_test(NAME ArbitraryIntegerTests COMMAND ArbitraryInteger)
