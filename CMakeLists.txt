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
    Concurrent
)

option(ENABLE_GRPC "Enable gRPC support" ON)

if (ENABLE_GRPC)
    message(STATUS "gRPC is ENABLED")

    set(GRPC_PKG_FILES
        pkg/geyser/src/geyser.grpc.pb.cc
        pkg/geyser/src/geyser.pb.cc
        pkg/geyser/src/solana-storage.grpc.pb.cc
        pkg/geyser/src/solana-storage.pb.cc
    )

    set(GRPC_SOURCE_FILES
        src/Clients/Solana/gRPC/Core/ConfigManager.cpp
        src/Clients/Solana/gRPC/Core/DataSourceManager.cpp
        src/Clients/Solana/gRPC/Core/DexFilter.cpp
        src/Clients/Solana/gRPC/Core/FilterManager.cpp
        src/Clients/Solana/gRPC/Core/MetricsManager.cpp
        src/Clients/Solana/gRPC/Core/NotificationManager.cpp
        src/Clients/Solana/gRPC/Core/StorageManager.cpp
        src/Clients/Solana/gRPC/Core/SwapFilter.cpp

        src/Clients/Solana/gRPC/Core/TransactionFilter.hpp

        src/Clients/Solana/gRPC/HTTP/HttpServer.cpp

        src/Clients/Solana/gRPC/Utils/Logger.hpp
    )
else()
    message(STATUS "gRPC is DISABLED")
    set(GRPC_PKG_FILES "")
    set(GRPC_SOURCE_FILES "")
    set(GRPC_HEADER_FILES "")
endif()

set(SOLANA_WEBSOCKET_FILES
)

set(SOURCE_FILES
    ${GRPC_SOURCE_FILES}
    ${SOLANA_WEBSOCKET_FILES}
    src/Clients/Core/Hydra/Hydra.cpp
    src/Clients/Core/Hydra/PriceDataSource.cpp
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
    src/Components/AddressImport/CryptoAddressEdit.cpp
    src/Components/Dialogs/ToolDialog.cpp
    src/Components/WalletImport/CryptoTextEdit.cpp
    src/Components/WalletListWidget.cpp
    src/Components/WalletPanel.cpp
    src/Databases/Database.cpp
    src/Forms/ImportWalletForm.cpp
    src/Forms/MessageForm.cpp
    src/Forms/NewAddressForm.cpp
    src/Forms/NewSmartAddressForm.cpp
    src/Forms/NewWalletForm.cpp
    src/Forms/SmartWalletForm.cpp
    src/Forms/UpdateSmartWalletForm.cpp
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
    src/Scripting/JSScriptEngine.cpp
    src/Scripting/LuaScriptEngine.cpp
    src/Tengu.cpp
    src/UI/Frameless.cpp
    src/Utils/AutoUpdater.cpp
    src/Utils/Encryption.cpp
    src/Utils/KeyStore.cpp
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
    src/Wallets/Monitor/Factory.cpp
    src/Wallets/Monitor/Manager.cpp
    src/Wallets/Monitor/Monitor.cpp
    src/Wallets/Monitor/Handlers/BitcoinHandler.cpp
    src/Wallets/Monitor/Handlers/EthereumHandler.cpp
    src/Wallets/Monitor/Handlers/SolanaHandler.cpp
    src/Wallets/Monitor/Providers/BitcoinBlockchainProvider.cpp
    src/Wallets/Monitor/Providers/BitcoinMempoolProvider.cpp
    src/Wallets/Monitor/Providers/EthereumBlockscoutProvider.cpp
    src/Wallets/Monitor/Providers/EthereumPublicNodeProvider.cpp
    src/Wallets/Monitor/Providers/SolanaMainnetProvider.cpp
    src/Wallets/Monitor/Providers/SolanaPublicNodeProvider.cpp
    src/Wallets/Transaction/Bitcoin/PSBT.cpp
    src/Wallets/Transaction/Bitcoin/Transaction.cpp
    src/Wallets/Transaction/Bitcoin/TransactionSigner.cpp
    src/Wallets/Transaction/Solana/Builder.cpp
    src/Wallets/Transaction/Solana/Transaction.cpp
    src/Wallets/Utils/SecureBytes.cpp
)

set(HEADER_FILES
    ${GRPC_HEADER_FILES}
    src/Clients/Core/Hydra/DataSource.h
)

set(UI_FILES
    src/Forms/ImportWalletForm.ui
    src/Forms/MessageForm.ui
    src/Forms/NewAddressForm.ui
    src/Forms/NewSmartAddressForm.ui
    src/Forms/NewWalletForm.ui
    src/Forms/SmartWalletForm.ui
    src/Forms/UpdateSmartWalletForm.ui
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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd/inc/luajit-2.1
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
        -Wno-attributes
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
        -Wno-attributes
    )
endif()

target_link_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/src/3rd/lib/grpc
)

set(THIRD_PARTY_LIBS
    xxhash
    pugixml
    sodium
    sqlite3
    rocksdb
    bz2
    snappy
    lz4
    luajit

    qjs
    izanagi
    secp256k1
    clip
    spdlog
    QCoro5Core
    QCoro5Network
    QCoro5WebSockets

    prometheus-cpp-core
    prometheus-cpp-push
    prometheus-cpp-pull

    Rpcrt4
    Mswsock
)

if (ENABLE_GRPC)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        grpc++_unsecure
        grpc++_error_details
        grpc++_alts
        grpc++
        grpc_authorization_provider
        grpc_unsecure
        grpc_plugin_support
        grpcpp_channelz
        grpc
        gpr

        cares

        absl_bad_any_cast_impl
        absl_bad_optional_access
        absl_bad_variant_access
        absl_base
        absl_city
        absl_civil_time
        absl_cord
        absl_cord_internal
        absl_cordz_functions
        absl_cordz_handle
        absl_cordz_info
        absl_cordz_sample_token
        absl_crc_cord_state
        absl_crc_cpu_detect
        absl_crc_internal
        absl_crc32c
        absl_debugging_internal
        absl_decode_rust_punycode
        absl_demangle_internal
        absl_demangle_rust
        absl_die_if_null
        absl_examine_stack
        absl_exponential_biased
        absl_failure_signal_handler
        absl_flags_commandlineflag
        absl_flags_commandlineflag_internal
        absl_flags_config
        absl_flags_internal
        absl_flags_marshalling
        absl_flags_parse
        absl_flags_private_handle_accessor
        absl_flags_program_name
        absl_flags_reflection
        absl_flags_usage
        absl_flags_usage_internal
        absl_graphcycles_internal
        absl_hash
        absl_hashtablez_sampler
        absl_int128
        absl_kernel_timeout_internal
        absl_leak_check
        absl_log_entry
        absl_log_flags
        absl_log_globals
        absl_log_initialize
        absl_log_internal_check_op
        absl_log_internal_conditions
        absl_log_internal_fnmatch
        absl_log_internal_format
        absl_log_internal_globals
        absl_log_internal_log_sink_set
        absl_log_internal_message
        absl_log_internal_nullguard
        absl_log_internal_proto
        absl_log_severity
        absl_log_sink
        absl_low_level_hash
        absl_malloc_internal
        absl_periodic_sampler
        absl_poison
        absl_random_distributions
        absl_random_internal_distribution_test_util
        absl_random_internal_platform
        absl_random_internal_pool_urbg
        absl_random_internal_randen
        absl_random_internal_randen_hwaes
        absl_random_internal_randen_hwaes_impl
        absl_random_internal_randen_slow
        absl_random_internal_seed_material
        absl_random_seed_gen_exception
        absl_random_seed_sequences
        absl_raw_hash_set
        absl_raw_logging_internal
        absl_scoped_set_env
        absl_spinlock_wait
        absl_stacktrace
        absl_status
        absl_statusor
        absl_str_format_internal
        absl_strerror
        absl_string_view
        absl_strings
        absl_strings_internal
        absl_symbolize
        absl_synchronization
        absl_throw_delegate
        absl_time
        absl_time_zone
        absl_utf8_for_code_point
        absl_vlog_config_internal
        address_sorting
        upb
        upb_base_lib
        upb_json_lib
        upb_mem_lib
        upb_message_lib
        upb_mini_descriptor_lib
        upb_textformat_lib
        upb_wire_lib
        utf8_range_lib

        cares
        protobuf
        protobuf-lite
        protoc
        utf8_range
        utf8_validity
        ssl
        crypto
        bcrypt
        re2
        imagehlp
        Crypt32
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
    Qt5::Concurrent
)

if (WIN32)
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
