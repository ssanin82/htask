cmake_minimum_required(VERSION 3.5...3.28)
project(htask CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)
find_package(catch2 REQUIRED)
find_package(cpr REQUIRED)
find_package(websocketpp REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(absl REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG)
find_package(libpsl CONFIG)

set(CPR_USE_SYSTEM_LIB_PSL 1)

# TARGETS

set(PROTO_FILE proto/pubsub.proto)

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
    PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
)

set(MORE_INCLUDES
    ${protobuf_INCLUDE_DIRS_RELEASE}
    ${abseil_INCLUDE_DIRS_RELEASE}
    ${grpc_INCLUDE_DIRS_RELEASE}
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
    src/md/md_base.cpp
    src/md/gateio.cpp
    src/md/okx.cpp
    src/md/binance.cpp
)
target_include_directories(pricer PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${websocketpp_INCLUDE_DIRS_RELEASE}
    ${cpr_INCLUDE_DIRS_RELEASE}
    ${MORE_INCLUDES}
)
target_link_libraries(pricer PRIVATE
    htlib
    ${DEP_LIBS}
    websocketpp::websocketpp
    Boost::boost
    OpenSSL::SSL
    OpenSSL::Crypto
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
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MORE_INCLUDES}
)

add_executable(consumer src/consumer.cpp)
target_link_libraries(consumer pubsub_proto ${DEP_LIBS})
target_include_directories(consumer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    ${CMAKE_CURRENT_BINARY_DIR}
    ${MORE_INCLUDES}
)

# -----------------

enable_testing()
add_subdirectory(test)
