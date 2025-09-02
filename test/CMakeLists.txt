cmake_minimum_required(VERSION 3.10)
project(httest CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(httest TestOrderBook.cpp)

target_link_libraries(httest PRIVATE Catch2::Catch2WithMain htlib)
target_include_directories(httest PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
