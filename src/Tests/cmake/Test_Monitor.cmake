project(test_monitor LANGUAGES CXX)

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
    Network
    WebSockets
    Concurrent
)

set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Clients/Core/Hydra/Hydra.cpp
    ${CMAKE_SOURCE_DIR}/src/Clients/Core/Hydra/PriceDataSource.cpp
    ${CMAKE_SOURCE_DIR}/src/tests/Test_Monitor.cpp
)

set(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/src/Clients/Core/Hydra/DataSource.h
)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${TEST_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/3rd/inc
    ${CMAKE_SOURCE_DIR}/src/3rd/inc/izanagi
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/3rd/lib
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    SECP256K1_STATIC
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
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    xxhash
    sodium
    izanagi
    secp256k1
    spdlog
    Qt5::Core
    Qt5::Network
    Qt5::WebSockets
    Qt5::Concurrent
    QCoro5Network
    QCoro5WebSockets
    QCoro5Core # order matters
)
