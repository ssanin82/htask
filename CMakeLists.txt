cmake_minimum_required(VERSION 3.5...3.28)
project(htask CXX)

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
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.6.0
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  libpsl
  GIT_REPOSITORY https://github.com/rockdaboot/libpsl.git
  GIT_TAG        0.21.5
)
FetchContent_MakeAvailable(libpsl)

set(CPR_USE_SYSTEM_LIB_PSL 1)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG        1.12.0
)
FetchContent_MakeAvailable(cpr)

find_package(absl REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG)
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

# TARGETS

set(PROTO_FILE proto/pubsub.proto)
get_target_property(Protobuf_INCLUDE_DIRS protobuf::libprotobuf
   INTERFACE_INCLUDE_DIRECTORIES
)

add_library(pubsub_proto)

protobuf_generate(
    LANGUAGE cpp
    TARGET pubsub_proto
    PROTOS ${PROTO_FILE}
)
protobuf_generate(
    TARGET pubsub_proto
    LANGUAGE grpc
    PROTOS ${PROTO_FILE}
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
)

set(MORE_INCLUDES
    ${gRPC_INSTALL_DIR}/third_party/protobuf/src
    ${gRPC_INSTALL_DIR}/third_party/abseil-cpp
    ${gRPC_INSTALL_DIR}/include
)
set(DEP_LIBS
    pubsub_proto
    gRPC::grpc++
    protobuf::libprotobuf
    absl::base
    absl::strings
    absl::time
    absl::status
)

target_sources(pubsub_proto PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/proto/pubsub.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/proto/pubsub.grpc.pb.cc
)
set(gRPC_INSTALL_DIR /root/dev/grpc)
target_include_directories(pubsub_proto PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MORE_INCLUDES}
)

add_executable(pricer
    src/pricer.cpp
    src/md/gateio.cpp
    src/md/okx.cpp
    src/md/binance.cpp
)
target_include_directories(pricer PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ixwebsocket_SOURCE_DIR}
    ${cpr_SOURCE_DIR}
    ${MORE_INCLUDES}
)
target_link_libraries(pricer PRIVATE
    htlib
    ${DEP_LIBS}
    ixwebsocket
    nlohmann_json::nlohmann_json
    cpr::cpr
)

add_library(htlib STATIC src/OrderBook.cpp)
target_include_directories(htlib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_dependencies(pricer htlib)

add_executable(server src/server.cpp)
target_link_libraries(server pubsub_proto ${DEP_LIBS})
target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MORE_INCLUDES}
)

add_executable(consumer src/consumer.cpp)
target_link_libraries(consumer pubsub_proto ${DEP_LIBS})
target_include_directories(consumer PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MORE_INCLUDES}
)

# -----------------

enable_testing()
add_subdirectory(test)
