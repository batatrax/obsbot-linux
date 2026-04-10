find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets)
find_package(KF6 REQUIRED COMPONENTS
    CoreAddons
    XmlGui
    I18n
    Notifications
    StatusNotifierItem
    ConfigWidgets
    WidgetsAddons
)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/VideoWidget.cpp
    src/VideoWidget.h
    src/GimbalWidget.cpp
    src/GimbalWidget.h
    src/AiWidget.cpp
    src/AiWidget.h
    src/CameraWidget.cpp
    src/CameraWidget.h
    src/PresetWidget.cpp
    src/PresetWidget.h
    src/GestureWidget.cpp
    src/GestureWidget.h
    src/FirmwareWidget.cpp
    src/FirmwareWidget.h
    src/TrayIcon.cpp
    src/TrayIcon.h
)

add_executable(obsbot-kde ${SOURCES})

target_include_directories(obsbot-kde PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/core/include
    ${OBSBOT_INCLUDE_DIR}
)

target_link_libraries(obsbot-kde PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets
    KF6::CoreAddons KF6::XmlGui KF6::I18n
    KF6::Notifications KF6::StatusNotifierItem
    KF6::ConfigWidgets KF6::WidgetsAddons
    obsbot-core
)

set_target_properties(obsbot-kde PROPERTIES
    INSTALL_RPATH "${OBSBOT_LIB_DIR}"
    BUILD_RPATH   "${OBSBOT_LIB_DIR}"
)

install(TARGETS obsbot-kde DESTINATION ${KDE_INSTALL_BINDIR})
install(FILES "${CMAKE_SOURCE_DIR}/data/icons/hicolor/scalable/apps/obsbot-linux.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps
        RENAME obsbot-kde.svg)
install(FILES "${CMAKE_SOURCE_DIR}/data/obsbot-kde.desktop"
        DESTINATION ${KDE_INSTALL_APPDIR})
