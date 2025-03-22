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

option(ENABLE_GRPC "Enable gRPC support" OFF)

if (ENABLE_GRPC)
    message(STATUS "gRPC is ENABLED")
    find_package(gRPC CONFIG REQUIRED)
    find_package(Protobuf CONFIG REQUIRED)
    find_package(absl CONFIG REQUIRED)

    set(GRPC_PKG_FILES
        pkg/geyser/src/geyser.grpc.pb.cc
        pkg/geyser/src/geyser.pb.cc
        pkg/geyser/src/solana-storage.grpc.pb.cc
        pkg/geyser/src/solana-storage.pb.cc
    )

    set(GRPC_SOURCE_FILES
        src/Clients/Solana/gRPC/GeyserClientManager.cpp
        src/Clients/Solana/gRPC/GeyserClientWorker.cpp
        src/Clients/Solana/gRPC/SwapFilter.cpp
        src/Clients/Solana/gRPC/TransactionFilter.cpp
    )

    set(GRPC_HEADER_FILES
        src/Clients/Solana/gRPC/GeyserClientManager.h
        src/Clients/Solana/gRPC/GeyserClientWorker.h
        src/Clients/Solana/gRPC/SwapFilter.h
        src/Clients/Solana/gRPC/TransactionFilter.h
    )
else()
    message(STATUS "gRPC is DISABLED")
    set(GRPC_PKG_FILES "")
    set(GRPC_SOURCE_FILES "")
    set(GRPC_HEADER_FILES "")
endif()

set(SOLANA_WEBSOCKET_FILES
    src/Clients/Solana/WebSocket/Client.cpp
)

set(SOURCE_FILES
    ${GRPC_SOURCE_FILES}
    ${SOLANA_WEBSOCKET_FILES}
    src/Components/AddressListWidget.cpp
    src/Components/AnimatedTabWidget.cpp
    src/Components/ClickableLabel.cpp
    src/Components/ComboBoxEx.cpp
    src/Components/FlowLayout.cpp
    src/Components/LineEditEx.cpp
    src/Components/MnemonicView.cpp
    src/Components/PlainTextEditEx.cpp
    src/Components/SVGWidget.cpp
    src/Components/Splash.cpp
    src/Components/TitleBar.cpp
    src/Components/Toast/Toast.cpp
    src/Components/TxListWidget.cpp
    src/Components/WalletImport/CryptoTextEdit.cpp
    src/Components/WalletListWidget.cpp
    src/Components/WalletPanel.cpp
    src/Databases/Database.cpp
    src/Forms/ImportWalletForm.cpp
    src/Forms/MessageForm.cpp
    src/Forms/NewAddressForm.cpp
    src/Forms/NewWalletForm.cpp
    src/Forms/UpdateWalletForm.cpp
    src/Forms/WalletDock.cpp
    src/Forms/WalletForm.cpp
    src/Forms/WalletSelectorForm.cpp
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
    src/UI/Frameless.cpp
    src/Utils/Encryption.cpp
    src/Utils/NameGenerator.cpp
    src/Utils/RunGuard.cpp
    src/Wallets/Core/BaseMnemonic.cpp
    src/Wallets/Core/BaseWallet.cpp
    src/Wallets/Core/BitcoinWallet.cpp
    src/Wallets/Core/ChainWallet.cpp
    src/Wallets/Core/Context.cpp
    src/Wallets/Core/EthereumWallet.cpp
    src/Wallets/Core/SolanaWallet.cpp
    src/Wallets/Core/SuiWallet.cpp
    src/Wallets/Monitor/BlockchainMonitor.cpp
    src/Wallets/Utils/SecureBytes.cpp
)

set(HEADER_FILES
    ${GRPC_HEADER_FILES}
    src/Components/AddressListWidget.h
    src/Components/AnimatedTabWidget.h
    src/Components/ClickableLabel.h
    src/Components/ComboBoxEx.h
    src/Components/FlowLayout.h
    src/Components/LineEditEx.h
    src/Components/MnemonicView.h
    src/Components/PlainTextEditEx.h
    src/Components/SVGWidget.h
    src/Components/Splash.h
    src/Components/TitleBar.h
    src/Components/Toast/Toast.h
    src/Components/TxListWidget.h
    src/Components/WalletListWidget.h
    src/Components/WalletPanel.h
    src/Consts.h
    src/Databases/Database.h
    src/Forms/ImportWalletForm.h
    src/Forms/MessageForm.h
    src/Forms/NewAddressForm.h
    src/Forms/NewWalletForm.h
    src/Forms/UpdateWalletForm.h
    src/Forms/WalletDock.h
    src/Forms/WalletForm.h
    src/Forms/WalletSelectorForm.h
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
    src/Utils/RSS.hpp
    src/Utils/RunGuard.h
    src/Wallets/Core/BaseMnemonic.h
    src/Wallets/Core/BaseWallet.h
    src/Wallets/Core/BitcoinWallet.h
    src/Wallets/Core/ChainWallet.h
    src/Wallets/Core/Context.h
    src/Wallets/Core/EthereumWallet.h
    src/Wallets/Core/Errors.hpp
    src/Wallets/Core/SolanaWallet.h
    src/Wallets/Core/SuiWallet.h
    src/Wallets/Core/Types.h
    src/Wallets/Monitor/BlockchainMonitor.h
    src/Wallets/Utils/Hex.hpp
    src/Wallets/Utils/SecureBytes.h
)

set(UI_FILES
    src/Forms/ImportWalletForm.ui
    src/Forms/MessageForm.ui
    src/Forms/NewAddressForm.ui
    src/Forms/NewWalletForm.ui
    src/Forms/UpdateWalletForm.ui
    src/Forms/WalletDock.ui
    src/Forms/WalletForm.ui
    src/Forms/WalletSelectorForm.ui
    src/Tengu.ui
)

set(RESOURCE_FILES
    src/res/Avatars.qrc
    src/res/Fonts.qrc
    src/res/Images.qrc
    src/res/Logos.qrc
    src/res/Toast.qrc
    src/res/Themes/dark/dark.qrc
    src/res/Themes/light/light.qrc
)

if(WIN32)
    set(WIN32_RC_FILE src/res/Tengu.rc)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/bin")
endforeach()

add_executable(${PROJECT_NAME} WIN32
    ${GRPC_PKG_FILES}
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
    pugixml
    sodium
    sqlite3
    luajit-5.1
    izanagi
    secp256k1
    fmt
    clip
)

if (ENABLE_GRPC)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        gRPC::grpc++
        protobuf::libprotobuf
        absl::log
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_LIBS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    Qt5::Charts
    Qt5::WebSockets
)

if (WIN32)
    #add_definitions(-DQT_STATIC)
    #add_definitions(-DWIN32_LEAN_AND_MEAN)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    SECP256K1_STATIC
    QT_STATIC
    QT_NO_KEYWORDS
)

option(ENABLE_TESTS "Enable tests" ON)

if (ENABLE_TESTS)
    add_subdirectory(src/tests)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
