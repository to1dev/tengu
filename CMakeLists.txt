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
    src/Clients/Solana/gRPC/GeyserClientManager.cpp
    src/Clients/Solana/gRPC/GeyserClientWorker.cpp
    src/Clients/Solana/gRPC/SwapFilter.cpp
    src/Clients/Solana/gRPC/TransactionFilter.cpp
    src/Components/AnimatedTabWidget.cpp
    src/Components/ClickableLabel.cpp
    src/Components/ComboBoxEx.cpp
    src/Components/FlowLayout.cpp
    src/Components/LineEditEx.cpp
    src/Components/MnemonicView.cpp
    src/Components/SVGWidget.cpp
    src/Components/Splash.cpp
    src/Components/TitleBar.cpp
    src/Components/Toast/Toast.cpp
    src/Components/TxListWidget.cpp
    src/Components/WalletListWidget.cpp
    src/Components/WalletPanel.cpp
    src/Databases/Database.cpp
    src/Forms/MessageForm.cpp
    src/Forms/NewWalletForm.cpp
    src/Forms/UpdateWalletForm.cpp
    src/Forms/WalletForm.cpp
    src/Layouts/AnimationController.cpp
    src/Layouts/GridLayoutEngine.cpp
    src/Layouts/LayoutEngine.cpp
    src/Layouts/LayoutOrchestrator.cpp
    src/Layouts/PredefinedLayoutEngine.cpp
    src/Layouts/ScreenManager.cpp
    src/Layouts/WindowState.cpp
    src/Main.cpp
    src/Managers/GlobalManager.cpp
    src/Managers/ResourceManager.cpp
    src/Managers/SettingManager.cpp
    src/Managers/ThemeManager.cpp
    src/Managers/WindowManager.cpp
    src/Tengu.cpp
    src/Tests/Test_BIP39.cpp
    src/Tests/Test_DotEnv.cpp
    src/Tests/Test_Encryption.cpp
    src/UI/Frameless.cpp
    src/Utils/Encryption.cpp
    src/Utils/NameGenerator.cpp
    src/Utils/RunGuard.cpp
    src/Wallets/Core/BaseMnemonic.cpp
    src/Wallets/Core/BaseWallet.cpp
    src/Wallets/Core/BitcoinWallet.cpp
    src/Wallets/Core/ChainWallet.cpp
    src/Wallets/Core/SolanaWallet.cpp
    src/Wallets/Utils/SecureBytes.cpp
)

set(HEADER_FILES
    src/Clients/Solana/gRPC/GeyserClientManager.h
    src/Clients/Solana/gRPC/GeyserClientWorker.h
    src/Clients/Solana/gRPC/SwapFilter.h
    src/Clients/Solana/gRPC/TransactionFilter.h
    src/Components/AnimatedTabWidget.h
    src/Components/ClickableLabel.h
    src/Components/ComboBoxEx.h
    src/Components/FlowLayout.h
    src/Components/LineEditEx.h
    src/Components/MnemonicView.h
    src/Components/SVGWidget.h
    src/Components/Splash.h
    src/Components/TitleBar.h
    src/Components/Toast/Toast.h
    src/Components/TxListWidget.h
    src/Components/WalletListWidget.h
    src/Components/WalletPanel.h
    src/Consts.h
    src/Databases/Database.h
    src/Forms/MessageForm.h
    src/Forms/NewWalletForm.h
    src/Forms/UpdateWalletForm.h
    src/Forms/WalletForm.h
    src/Globals.h
    src/Layouts/AnimationController.h
    src/Layouts/GridLayoutEngine.h
    src/Layouts/LayoutEngine.h
    src/Layouts/LayoutOrchestrator.h
    src/Layouts/PredefinedLayoutEngine.h
    src/Layouts/ScreenManager.h
    src/Layouts/WindowState.h
    src/Managers/GlobalManager.h
    src/Managers/ResourceManager.h
    src/Managers/SettingManager.h
    src/Managers/ThemeManager.h
    src/Managers/WindowManager.h
    src/Rust.h
    src/Security/Security.h
    src/Tengu.h
    src/UI/Frameless.h
    src/Utils/Base58.hpp
    src/Utils/DotEnv.hpp
    src/Utils/Encryption.h
    src/Utils/Helpers.hpp
    src/Utils/NameGenerator.h
    src/Utils/PathUtils.hpp
    src/Utils/RunGuard.h
    src/Wallets/Core/BaseMnemonic.h
    src/Wallets/Core/BaseWallet.h
    src/Wallets/Core/BitcoinWallet.h
    src/Wallets/Core/ChainWallet.h
    src/Wallets/Core/Errors.hpp
    src/Wallets/Core/SolanaWallet.h
    src/Wallets/Core/Types.h
    src/Wallets/Utils/SecureBytes.h
)

set(UI_FILES
    src/Forms/MessageForm.ui
    src/Forms/NewWalletForm.ui
    src/Forms/UpdateWalletForm.ui
    src/Forms/WalletForm.ui
    src/Tengu.ui
)

set(RESOURCE_FILES
    src/res/Avatars.qrc
    src/res/Fonts.qrc
    src/res/Images.qrc
    src/res/Logos.qrc
    src/res/Themes/dark/dark.qrc
    src/res/Themes/light/light.qrc
)

if(WIN32)
    set(WIN32_RC_FILE src/res/Tengu.rc)
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
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd/inc/izanagi
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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd/lib
)

set(THIRD_PARTY_LIBS
    xxhash
    sodium
    sqlite3
    luajit-5.1
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

if (WIN32)
    #add_definitions(-DQT_STATIC)
    #add_definitions(-DWIN32_LEAN_AND_MEAN)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    USE_TEST
    QT_STATIC
    QT_NO_KEYWORDS
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
