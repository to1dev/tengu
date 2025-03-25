cmake_minimum_required(VERSION 3.16)
project(t2d LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "-static -static-libstdc++ -static-libgcc")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

add_compile_options(-Wall -Wextra -Wpedantic -O2)

add_executable(${PROJECT_NAME} main.cpp)

#target_include_directories(${PROJECT_NAME}  PRIVATE
#    ${CMAKE_CURRENT_SOURCE_DIR}/
#    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/inc
#)

#target_link_directories(${PROJECT_NAME} PRIVATE
#    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/lib
#)

target_link_options(${PROJECT_NAME} PRIVATE -s)

target_link_libraries(${PROJECT_NAME} PRIVATE
    lmdb
    simdjson
)
