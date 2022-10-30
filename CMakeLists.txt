#
# Based on an example here: https://github.com/ndsev/zserio-service-grpc-cpp/blob/master/src/CMakeLists.txt
# and here: https://github.com/grpc/grpc/blob/master/examples/cpp/helloworld/CMakeLists.txt
#

cmake_minimum_required(VERSION 3.5.1)

project(FibServiceGrpc)

option(BUILD_SHARED_LIBS "Whether to build shared libraries" OFF)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

message(STATUS "Using protobuf ${Protobuf_VERSION}")
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(GRPC_GRPCPP gRPC::grpc++)
set(PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(REFLECTION gRPC::grpc++_reflection)
set(PROTOC_EXECUTABLE $<TARGET_FILE:protobuf::protoc>)
set(GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

get_filename_component(FIB_SERVICE_PROTO "proto/fib.proto" ABSOLUTE)
get_filename_component(FIB_SERVICE_PROTO_PATH "${FIB_SERVICE_PROTO}" PATH)

set(FIB_SERVICE_GEN_SOURCES
    "${CMAKE_CURRENT_BINARY_DIR}/fib.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/fib.pb.h"
    "${CMAKE_CURRENT_BINARY_DIR}/fib.grpc.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/fib.grpc.pb.h"
)

set(GOOGLE_API_ANNOT
    "${CMAKE_CURRENT_BINARY_DIR}/google/api/annotations.grpc.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/google/api/annotations.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/google/api/http.grpc.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/google/api/http.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/google/protobuf/descriptor.grpc.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/google/protobuf/descriptor.pb.cc"
)

add_custom_command(
    OUTPUT ${FIB_SERVICE_GEN_SOURCES}
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS
        --grpc_out="${CMAKE_CURRENT_BINARY_DIR}" --cpp_out="${CMAKE_CURRENT_BINARY_DIR}"
        --include_imports --include_source_info --descriptor_set_out="${FIB_SERVICE_PROTO}.pb"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_EXECUTABLE}"
        -I"${FIB_SERVICE_PROTO_PATH}" -I"$ENV{GOOGLEAPIS_DIR}" "${FIB_SERVICE_PROTO}"
    DEPENDS "${FIB_SERVICE_PROTO}"
    COMMENT "Generating Protobuf GRPC API"
)

add_custom_command(
    OUTPUT ${GOOGLE_API_ANNOT}
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS
        --grpc_out="${CMAKE_CURRENT_BINARY_DIR}" --cpp_out="${CMAKE_CURRENT_BINARY_DIR}"
        --include_imports --include_source_info --descriptor_set_out="${FIB_SERVICE_PROTO}.desc"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_EXECUTABLE}"
        -I"$ENV{GOOGLEAPIS_DIR}" "$ENV{GOOGLEAPIS_DIR}/google/api/annotations.proto"
        "$ENV{GOOGLEAPIS_DIR}/google/api/http.proto" "$ENV{MY_INSTALL_DIR}/include/google/protobuf/descriptor.proto"
    DEPENDS "${FIB_SERVICE_PROTO}"
    COMMENT "Generating Protobuf GRPC API - Google API annotations"
)

add_library(${PROJECT_NAME}
    ${FIB_SERVICE_GEN_SOURCES}
    ${GOOGLE_API_ANNOT}
)

target_link_libraries(${PROJECT_NAME}
    ${REFLECTION}
    ${GRPC_GRPCPP}
    ${PROTOBUF_LIBPROTOBUF}
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}")

# Targets fib_server and fib_client
foreach(TARGET fib_server fib_client)
    add_executable(${TARGET} "${TARGET}.cc")
    target_link_libraries(${TARGET}
        ${PROJECT_NAME}
        ${REFLECTION}
        ${GRPC_GRPCPP}
        ${PROTOBUF_LIBPROTOBUF})
endforeach()
