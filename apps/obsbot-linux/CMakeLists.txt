find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/VideoWindow.cpp
    src/VideoWindow.h
    src/VideoWidget.cpp
    src/VideoWidget.h
    src/ControlPanel.cpp
    src/ControlPanel.h
    src/JoystickWidget.cpp
    src/JoystickWidget.h
    src/FirmwareWidget.cpp
    src/FirmwareWidget.h
    src/Style.h
)

add_executable(obsbot-linux ${SOURCES})

target_include_directories(obsbot-linux PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/core/include
    ${OBSBOT_INCLUDE_DIR}
)

target_link_libraries(obsbot-linux PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets
    obsbot-core
)

set_target_properties(obsbot-linux PROPERTIES
    INSTALL_RPATH "${OBSBOT_LIB_DIR}"
    BUILD_RPATH   "${OBSBOT_LIB_DIR}"
)

install(TARGETS obsbot-linux DESTINATION bin)
install(FILES "${CMAKE_SOURCE_DIR}/data/icons/hicolor/scalable/apps/obsbot-linux.svg"
        DESTINATION share/icons/hicolor/scalable/apps)
install(FILES "${CMAKE_SOURCE_DIR}/data/obsbot-linux.desktop"
        DESTINATION share/applications)
install(FILES "${CMAKE_SOURCE_DIR}/packaging/99-obsbot.rules"
        DESTINATION share/obsbot-linux)
