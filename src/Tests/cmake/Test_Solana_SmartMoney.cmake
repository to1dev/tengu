project(test_solana_smartmoney LANGUAGES CXX)

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
    WebSockets
)

set(ALL_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/WebSocket/SolanaConnectionManager.cpp
    ${CMAKE_SOURCE_DIR}/src/Clients/Solana/WebSocket/SmartMoneyTracker.cpp
)

set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/tests/Test_Solana_SmartMoney/main.cpp
)

add_executable(${PROJECT_NAME} ${ALL_SOURCES} ${TEST_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/3rd/inc
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/3rd/lib
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
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::WebSockets
)
