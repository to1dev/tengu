cmake_minimum_required(VERSION 3.16)
project(Tengu LANGUAGES CXX)

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
    Svg
    Charts
    WebSockets
)

if (WIN32)
    add_definitions(-DQT_STATIC)
endif()

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)

set(PKG_FILES
    pkg/geyser/src/geyser.grpc.pb.cc
    pkg/geyser/src/geyser.pb.cc
    pkg/geyser/src/solana-storage.grpc.pb.cc
    pkg/geyser/src/solana-storage.pb.cc
)

set(SOURCE_FILES
    Clients/Solana/gRPC/GeyserClientManager.cpp
    Clients/Solana/gRPC/GeyserClientWorker.cpp
    Clients/Solana/gRPC/SwapFilter.cpp
    Clients/Solana/gRPC/TransactionFilter.cpp
    Components/AnimatedTabWidget.cpp
    Components/ClickableLabel.cpp
    Components/FlowLayout.cpp
    Components/LineEditEx.cpp
    Components/MnemonicView.cpp
    Components/SVGWidget.cpp
    Components/Splash.cpp
    Components/TitleBar.cpp
    Components/TxListWidget.cpp
    Components/WalletListWidget.cpp
    Components/WalletPanel.cpp
    Databases/Database.cpp
    Forms/MessageForm.cpp
    Forms/NewWalletForm.cpp
    Forms/WalletForm.cpp
    Layouts/AnimationController.cpp
    Layouts/GridLayoutEngine.cpp
    Layouts/LayoutEngine.cpp
    Layouts/LayoutOrchestrator.cpp
    Layouts/PredefinedLayoutEngine.cpp
    Layouts/ScreenManager.cpp
    Layouts/WindowState.cpp
    Main.cpp
    Managers/GlobalManager.cpp
    Managers/ResourceManager.cpp
    Managers/SettingManager.cpp
    Managers/ThemeManager.cpp
    Managers/WindowManager.cpp
    Tengu.cpp
    Tests/Test_BIP39.cpp
    Tests/Test_DotEnv.cpp
    Tests/Test_Encryption.cpp
    UI/Frameless.cpp
    Utils/Encryption.cpp
    Utils/NameGenerator.cpp
    Utils/RunGuard.cpp
    Wallets/Core/BaseMnemonic.cpp
    Wallets/Core/BaseWallet.cpp
    Wallets/Core/BitcoinWallet.cpp
    Wallets/Core/ChainWallet.cpp
    Wallets/Core/SolanaWallet.cpp
    Wallets/Utils/SecureBytes.cpp
)

set(HEADER_FILES
    Clients/Solana/gRPC/GeyserClientManager.h
    Clients/Solana/gRPC/GeyserClientWorker.h
    Clients/Solana/gRPC/SwapFilter.h
    Clients/Solana/gRPC/TransactionFilter.h
    Components/AnimatedTabWidget.h
    Components/ClickableLabel.h
    Components/FlowLayout.h
    Components/LineEditEx.h
    Components/MnemonicView.h
    Components/SVGWidget.h
    Components/Splash.h
    Components/TitleBar.h
    Components/TxListWidget.h
    Components/WalletListWidget.h
    Components/WalletPanel.h
    Consts.h
    Databases/Database.h
    Forms/MessageForm.h
    Forms/NewWalletForm.h
    Forms/WalletForm.h
    Globals.h
    Layouts/AnimationController.h
    Layouts/GridLayoutEngine.h
    Layouts/LayoutEngine.h
    Layouts/LayoutOrchestrator.h
    Layouts/PredefinedLayoutEngine.h
    Layouts/ScreenManager.h
    Layouts/WindowState.h
    Managers/GlobalManager.h
    Managers/ResourceManager.h
    Managers/SettingManager.h
    Managers/ThemeManager.h
    Managers/WindowManager.h
    Rust.h
    Security/Security.h
    Tengu.h
    UI/Frameless.h
    Utils/Base58.hpp
    Utils/DotEnv.hpp
    Utils/Encryption.h
    Utils/Helpers.hpp
    Utils/NameGenerator.h
    Utils/PathUtils.hpp
    Utils/RunGuard.h
    Wallets/Core/BaseMnemonic.h
    Wallets/Core/BaseWallet.h
    Wallets/Core/BitcoinWallet.h
    Wallets/Core/ChainWallet.h
    Wallets/Core/Errors.hpp
    Wallets/Core/SolanaWallet.h
    Wallets/Core/Types.h
    Wallets/Utils/SecureBytes.h
)

set(UI_FILES
    Forms/MessageForm.ui
    Forms/NewWalletForm.ui
    Forms/WalletForm.ui
    Tengu.ui
)

set(RESOURCE_FILES
    res/Avatars.qrc
    res/Fonts.qrc
    res/Images.qrc
    res/Logos.qrc
    res/Themes/dark/dark.qrc
    res/Themes/light/light.qrc
)

if(WIN32)
    set(WIN32_RC_FILE res/Tengu.rc)
endif()

add_executable(${PROJECT_NAME} WIN32
    ${PKG_FILES}
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${UI_FILES}
    ${RESOURCE_FILES}
    ${WIN32_RC_FILE}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/inc/izanagi
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wno-unused-parameter
        -Wno-template-id-cdtor
        -Wno-tautological-compare
        -Wno-unused-local-typedefs
        -Wno-volatile
        -Wno-misleading-indentation
        -Wno-unused-but-set-parameter
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wno-unused-parameter
        -Wno-template-id-cdtor
        -Wno-tautological-compare
        -Wno-unused-local-typedefs
        -Wno-volatile
        -Wno-misleading-indentation
        -Wno-unused-but-set-parameter
    )
endif()

target_link_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/lib
)

set(THIRD_PARTY_LIBS
    xxhash
    sodium
    sqlite3
    izanagi
    catch2
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_LIBS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    Qt5::Charts
    Qt5::WebSockets
    gRPC::grpc++
    protobuf::libprotobuf
    absl::log
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    USE_TEST
    QT_NO_KEYWORDS
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
