cmake_minimum_required(VERSION 3.10)
project(wsbinance CXX)

 set(CMAKE_CXX_STANDARD 23)
 set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG        v11.4.6
)
FetchContent_MakeAvailable(ixwebsocket)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

# add_executable(${PROJECT_NAME} src/MdBinance.cpp)
# add_executable(${PROJECT_NAME} src/MdOkx.cpp)
add_executable(${PROJECT_NAME} src/MdGateIo.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC
    "${ixwebsocket_SOURCE_DIR}"
    "${simdjson_SOURCE_DIR}"
)
target_link_libraries(${PROJECT_NAME} PRIVATE ixwebsocket nlohmann_json::nlohmann_json)
