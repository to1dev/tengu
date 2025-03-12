project(test_runner LANGUAGES CXX)

find_package(Qt5 REQUIRED COMPONENTS
    Core
)

file(GLOB ALL_SOURCES
    "${CMAKE_SOURCE_DIR}/src/Wallets/Core/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/Wallets/Utils/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/Utils/Encryption.cpp"
)

file(GLOB TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(${PROJECT_NAME} ${ALL_SOURCES} ${TEST_SOURCES})

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
    catch2
    Qt5::Core
)
