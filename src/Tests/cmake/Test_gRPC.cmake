project(test_grpc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/tests/Test_gRPC.cpp
)

set(GRPC_PKG_FILES
    ${CMAKE_SOURCE_DIR}/pkg/geyser/src/geyser.grpc.pb.cc
    ${CMAKE_SOURCE_DIR}/pkg/geyser/src/geyser.pb.cc
    ${CMAKE_SOURCE_DIR}/pkg/geyser/src/solana-storage.grpc.pb.cc
    ${CMAKE_SOURCE_DIR}/pkg/geyser/src/solana-storage.pb.cc
)

set(GRPC_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/Core/ConfigManager.cpp
    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/Core/DataSourceManager.cpp
    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/Core/DexFilter.cpp
    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/Core/FilterManager.cpp
    #${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/Core/MetricsManager.cpp
    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/Core/NotificationManager.cpp
    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/Core/StorageManager.cpp
    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/Core/SwapFilter.cpp

    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/Core/TransactionFilter.hpp

    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/HTTP/HttpServer.cpp

    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/gRPC/Utils/Logger.hpp
)

add_executable(${PROJECT_NAME} ${GRPC_PKG_FILES} ${GRPC_SOURCE_FILES} ${TEST_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/pkg/geyser/src
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/3rd/inc
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/3rd/lib
    ${CMAKE_SOURCE_DIR}/src/3rd/lib/grpc
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_STATIC
    QT_NO_KEYWORDS
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wno-unused-parameter
    -Wno-template-id-cdtor
    -Wno-tautological-compare
    -Wno-unused-local-typedefs
    -Wno-volatile
    -Wno-misleading-indentation
    -Wno-unused-but-set-parameter
    -Wno-attributes
)

set(THIRD_PARTY_LIBS
    sodium
    rocksdb
    bz2
    snappy
    lz4

    spdlog

    #prometheus-cpp-core
    #prometheus-cpp-push
    #prometheus-cpp-pull

    Rpcrt4
    Mswsock
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    gRPC::grpc++
    protobuf::libprotobuf

    ssl
    crypto
    bcrypt
    re2

    imagehlp
    Crypt32
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_LIBS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
)

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
    )
endif()
