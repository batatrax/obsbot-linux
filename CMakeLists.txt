cmake_minimum_required(VERSION 3.20)
project(ObsbotLinux VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── Options ───────────────────────────────────────────────────────────────────
option(BUILD_OBSBOT_LINUX "Construire obsbot-linux (Qt6 pur)"  ON)
option(BUILD_OBSBOT_KDE   "Construire obsbot-kde (KDE Frameworks 6)" ON)

# ── SDK OBSBOT ────────────────────────────────────────────────────────────────
set(OBSBOT_SDK_DIR "${CMAKE_SOURCE_DIR}/sdk" CACHE PATH "Dossier SDK OBSBOT")
set(OBSBOT_INCLUDE_DIR "${OBSBOT_SDK_DIR}/include")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(OBSBOT_LIB_DIR "${OBSBOT_SDK_DIR}/linux/arm64-release")
else()
    set(OBSBOT_LIB_DIR "${OBSBOT_SDK_DIR}/linux/x86_64-release")
endif()

if(NOT EXISTS "${OBSBOT_INCLUDE_DIR}/dev/dev.hpp")
    message(FATAL_ERROR
        "SDK OBSBOT introuvable dans ${OBSBOT_INCLUDE_DIR}\n"
        "Décompressez libdev_v2_1_0_8.zip dans sdk/\n"
        "  mkdir -p sdk && unzip libdev_v2_1_0_8.zip -d /tmp/s\n"
        "  cp -r /tmp/s/libdev_v2.1.0_8/* sdk/\n")
endif()

find_library(OBSBOT_LIB NAMES dev PATHS "${OBSBOT_LIB_DIR}" NO_DEFAULT_PATH REQUIRED)
message(STATUS "OBSBOT libdev : ${OBSBOT_LIB}")

# ── Core partagé ──────────────────────────────────────────────────────────────
add_subdirectory(core)

# ── Applications ──────────────────────────────────────────────────────────────
if(BUILD_OBSBOT_LINUX)
    add_subdirectory(apps/obsbot-linux)
endif()

if(BUILD_OBSBOT_KDE)
    add_subdirectory(apps/obsbot-kde)
endif()
