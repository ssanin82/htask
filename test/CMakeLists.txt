cmake_minimum_required(VERSION 3.10)
project(httest CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# XXX For some reason conan has problems location Catch2, thus this hack
include(FetchContent)
FetchContent_Declare(
    Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.10.0.tar.gz
)
FetchContent_MakeAvailable(Catch2)

add_executable(httest
    TestOrderBook.cpp
    TestUtil.cpp
)

target_link_libraries(httest PRIVATE Catch2::Catch2WithMain htlib)
target_include_directories(httest PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
