cmake_minimum_required(VERSION 3.20)
project(ObsbotLinux VERSION 1.1.0 LANGUAGES CXX)

# ── CPack (DEB + RPM) ─────────────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME              "obsbot-linux")
set(CPACK_PACKAGE_VENDOR            "batatrax")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Linux controller for OBSBOT Tiny 2 Lite camera / Contrôleur Linux pour caméra OBSBOT Tiny 2 Lite")
set(CPACK_PACKAGE_VERSION           "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT           "https://github.com/batatrax/obsbot-linux")
set(CPACK_PACKAGE_HOMEPAGE_URL      "https://github.com/batatrax/obsbot-linux")
set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_STRIP_FILES               ON)
set(CPACK_PACKAGE_SECTION          "video")

# ── DEB ───────────────────────────────────────────────────────────────────────
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "libqt6core6 (>= 6.4), libqt6widgets6 (>= 6.4), libqt6multimedia6 (>= 6.4), libusb-1.0-0")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "v4l2loopback-dkms, ffmpeg")
set(CPACK_DEBIAN_PACKAGE_SECTION    "video")
set(CPACK_DEBIAN_PACKAGE_PRIORITY   "optional")
set(CPACK_DEBIAN_FILE_NAME          "DEB-DEFAULT")
# Install udev rule via DEB post-install
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_SOURCE_DIR}/packaging/deb/postinst;${CMAKE_SOURCE_DIR}/packaging/deb/prerm")

# ── RPM ───────────────────────────────────────────────────────────────────────
set(CPACK_RPM_PACKAGE_LICENSE       "GPLv3")
set(CPACK_RPM_PACKAGE_GROUP         "Applications/Multimedia")
set(CPACK_RPM_PACKAGE_REQUIRES
    "qt6-qtbase >= 6.4, qt6-qtmultimedia >= 6.4, libusb1")
set(CPACK_RPM_PACKAGE_SUGGESTS      "v4l2loopback, ffmpeg")
set(CPACK_RPM_FILE_NAME             "RPM-DEFAULT")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE
    "${CMAKE_SOURCE_DIR}/packaging/rpm/post_install.sh")
set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE
    "${CMAKE_SOURCE_DIR}/packaging/rpm/pre_uninstall.sh")

include(CPack)

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
