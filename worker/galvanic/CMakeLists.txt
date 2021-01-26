cmake_minimum_required(VERSION 3.13)

project(galvanic-manager C CXX CUDA)

set(CMAKE_CXX_STANDARD 14)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf REQUIRED QUIET)
message(STATUS "Using protobuf ${Protobuf_VERSION}")
if(TARGET protobuf::protoc)
  set(_PROTOBUFFERS_PROTOC $<TARGET_FILE:protobuf::protoc>)
else()
  find_program(_PROTOBUFFERS_PROTOC protoc)
endif()
message(STATUS "Using protoc ${_PROTOBUFFERS_PROTOC}")

find_package(PkgConfig REQUIRED)
pkg_search_module(gRPC REQUIRED grpc)
pkg_search_module(gRPCPP REQUIRED grpc++)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPCPP::grpc++_reflection)
set(_GRPC_GRPCPP gRPCPP::grpc++_unsecure)

# Proto file
# daemon_service.proto is used as the protocol between manager and spawn daemon.
get_filename_component(daemon_service_proto "daemon_service.proto" ABSOLUTE)
get_filename_component(daemon_service_proto_path "${daemon_service_proto}" PATH)

# Generated sources
set(daemon_service_grpc_srcs  "${CMAKE_CURRENT_BINARY_DIR}/daemon_service.grpc.fb.cc")
set(daemon_service_grpc_hdrs  "${CMAKE_CURRENT_BINARY_DIR}/daemon_service.grpc.fb.h")
add_custom_command(
  OUTPUT "${daemon_service_grpc_srcs}"
         "${daemon_service_grpc_hdrs}"
  COMMAND ${_PROTOBUFFERS_PROTOC}
  ARGS -I "${daemon_service_proto_path}"
       --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
       "${daemon_service_proto}"
  DEPENDS "${daemon_service_proto}"
)

add_custom_target(daemon_service_files ALL
  DEPENDS ${daemon_service_grpc_srcs} ${daemon_service_grpc_hdrs})

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories(${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})

find_library(NVML_LIB nvidia-ml ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
add_compile_options(-Wall -Wextra -pedantic -Wno-write-strings)

add_executable(galvanic-manager
  manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../../common/socket.cpp
  ${manager_service_proto_srcs}
  ${manager_service_ava_srcs}
  ${daemon_service_grpc_srcs}
)
target_link_libraries(galvanic-manager
  ${manager_service_libraries}
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  ${_FLATBUFFERS}
  ${NVML_LIB}
)
set_target_properties(galvanic-manager
  PROPERTIES OUTPUT_NAME manager
)

add_executable(galvanic-spawn_daemon
  spawn_daemon.cpp
  ${manager_service_grpc_srcs}
  ${daemon_service_grpc_srcs}
)
target_link_libraries(galvanic-spawn_daemon
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  ${_FLATBUFFERS}
  ${NVML_LIB}
)
set_target_properties(galvanic-spawn_daemon
  PROPERTIES OUTPUT_NAME daemon
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/daemon
  TYPE BIN
  RENAME galvanic_daemon
)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/manager
  TYPE BIN
  RENAME galvanic_manager
)
