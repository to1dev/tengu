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

set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/tests/Test_gRPC.cpp
)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
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
)

set(THIRD_PARTY_LIBS
    sodium
    rocksdb
    bz2
    snappy
    lz4

    spdlog

    prometheus-cpp-core
    prometheus-cpp-push
    prometheus-cpp-pull

    Rpcrt4
    Mswsock
)

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
