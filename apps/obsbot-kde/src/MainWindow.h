#pragma once
#include <KXmlGuiWindow>
#include <dev/dev.hpp>

class QStackedWidget;
class QListWidget;
class VideoWidget;
class GimbalWidget;
class AiWidget;
class CameraWidget;
class PresetWidget;
class GestureWidget;
class FirmwareWidget;
class StatusBar;
class TrayIcon;

class MainWindow : public KXmlGuiWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onDeviceConnected(const QString &sn);
    void onDeviceDisconnected(const QString &sn);
    void onStatusUpdated(Device::CameraStatus status);
    void onNavChanged(int row);

private:
    void buildUi();
    void setupActions();
    void updateConnectionState(bool connected);

    QStackedWidget  *m_stack    = nullptr;
    QListWidget     *m_nav      = nullptr;
    VideoWidget     *m_video    = nullptr;
    GimbalWidget    *m_gimbal   = nullptr;
    AiWidget        *m_ai       = nullptr;
    CameraWidget    *m_camera   = nullptr;
    PresetWidget    *m_presets  = nullptr;
    GestureWidget   *m_gestures = nullptr;
    FirmwareWidget  *m_firmware = nullptr;
    StatusBar       *m_statusBar= nullptr;
    TrayIcon        *m_tray     = nullptr;
};
