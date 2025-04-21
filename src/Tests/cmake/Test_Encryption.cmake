project(test_encryption LANGUAGES CXX)

find_package(Qt5 REQUIRED COMPONENTS
    Core
)

set(ALL_SOURCES
    ${CMAKE_SOURCE_DIR}/src/Utils/Encryption.cpp
    ${CMAKE_SOURCE_DIR}/src/Utils/KeyStore.cpp
)

set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/tests/Test_Encryption.cpp
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
    xxhash
    sodium
    catch2
    Qt5::Core

    Crypt32
)
