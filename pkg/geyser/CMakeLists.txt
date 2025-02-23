cmake_minimum_required(VERSION 3.10)
project(geyser)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${CMAKE_SOURCE_DIR}/src)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

set(CPP_SOURCES
    src/geyser.pb.cc
    src/geyser.grpc.pb.cc
    src/solana-storage.pb.cc
    src/solana-storage.grpc.pb.cc
)

set(ALL_SOURCES
    ${CPP_SOURCES}
)

add_library(geyser STATIC ${ALL_SOURCES})
