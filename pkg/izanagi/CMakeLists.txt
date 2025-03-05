cmake_minimum_required(VERSION 3.10)
project(izanagi)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
add_definitions(-DUSE_PRECOMPUTED_CP=1)
include_directories(${CMAKE_SOURCE_DIR}/src)

set(CRYPTO_SOURCES
    src/bignum.c
    src/hasher.c
    src/ecdsa.c
    src/curves.c
    src/nist256p1.c
    src/secp256k1.c
    src/segwit_addr.c
    src/groestl.c
)

set(C_SOURCES
    src/ed25519-donna/ed25519.c
    src/ed25519-donna/ed25519-sha3.c
    src/ed25519-donna/ed25519-keccak.c
    src/ed25519-donna/modm-donna-32bit.c
    src/ed25519-donna/ed25519-donna-impl-base.c
    src/ed25519-donna/ed25519-donna-32bit-tables.c
    src/ed25519-donna/curve25519-donna-32bit.c
    src/ed25519-donna/curve25519-donna-helpers.c
    src/ed25519-donna/curve25519-donna-scalarmult-base.c
    src/bip32.c
    src/bip39.c
    src/bip39_english.c
    src/hmac.c
    src/memzero.c
    src/pbkdf2.c
    src/rand.c
    src/sha2.c
    src/sha3.c
    src/base58.c
    src/blake2b.c
    src/blake256.c
    src/ripemd160.c
    src/rfc6979.c
    src/address.c
    src/hmac_drbg.c
)

set(ALL_SOURCES
    ${CRYPTO_SOURCES}
    ${C_SOURCES}
)

add_library(izanagi STATIC ${ALL_SOURCES})
