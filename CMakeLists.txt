cmake_minimum_required(VERSION 3.5...3.28)
project(pricer CXX)

 set(CMAKE_CXX_STANDARD 23)
 set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# FetchContent_Declare(
#     ixwebsocket
#     GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
#     GIT_TAG        v11.4.6
# )
# FetchContent_MakeAvailable(ixwebsocket)

# FetchContent_Declare(
#     json
#     GIT_REPOSITORY https://github.com/nlohmann/json.git
#     GIT_TAG        v3.12.0
# )
# FetchContent_MakeAvailable(json)

# FetchContent_Declare(
#   Catch2
#   GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#   GIT_TAG        v3.6.0
# )
# FetchContent_MakeAvailable(Catch2)

# FetchContent_Declare(
#     cpr
#     GIT_REPOSITORY https://github.com/libcpr/cpr.git
#     GIT_TAG        1.12.0
# )
# FetchContent_MakeAvailable(cpr)





FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v32.0
)
FetchContent_MakeAvailable(protobuf)
# find_package(Protobuf REQUIRED)

FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG v1.74.1
)
FetchContent_MakeAvailable(gRPC)

add_executable(test_pub src/test_pub.cpp proto/pubsub.proto)
target_link_libraries(test_pub PRIVATE protobuf::libprotobuf grpc++)

# add_executable(test_sub src/test_sub.cpp proto/pubsub.proto)
# target_link_libraries(test_sub PRIVATE protobuf::libprotobuf)

protobuf_generate(
    LANGUAGE cpp
    TARGET test_pub
    PROTOS proto/pubsub.proto
)





# add_executable(${PROJECT_NAME}
#     src/pricer.cpp
#     src/md/gateio.cpp
#     src/md/okx.cpp
#     src/md/binance.cpp
# )
# target_include_directories(${PROJECT_NAME} PUBLIC
#     "${CMAKE_CURRENT_SOURCE_DIR}/include"
#     "${ixwebsocket_SOURCE_DIR}"
#     "${cpr_SOURCE_DIR}"
# )
# target_link_libraries(${PROJECT_NAME} PRIVATE
#     htlib
#     ixwebsocket
#     nlohmann_json::nlohmann_json
#     cpr::cpr
# )

# add_library(htlib STATIC src/OrderBook.cpp)
# target_include_directories(htlib PUBLIC
#     "${CMAKE_CURRENT_SOURCE_DIR}/include"
# )

# add_dependencies(${PROJECT_NAME} htlib)

# enable_testing()
# add_subdirectory(test)
