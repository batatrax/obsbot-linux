find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(CMAKE_AUTOMOC ON)
add_library(obsbot-core STATIC
    src/DeviceManager.cpp
    src/PresetManager.cpp
    src/GestureManager.cpp
    src/FirmwareManager.cpp
    # Headers Q_OBJECT — necessaires pour AUTOMOC
    include/obsbot/DeviceManager.h
    include/obsbot/PresetManager.h
    include/obsbot/GestureManager.h
    include/obsbot/FirmwareManager.h
)

target_include_directories(obsbot-core PUBLIC
    include
    ${OBSBOT_INCLUDE_DIR}
)

target_link_libraries(obsbot-core PUBLIC
    Qt6::Core
    Qt6::Network
    ${OBSBOT_LIB}
)

set_target_properties(obsbot-core PROPERTIES
    INSTALL_RPATH "${OBSBOT_LIB_DIR}"
    BUILD_RPATH   "${OBSBOT_LIB_DIR}"
)
