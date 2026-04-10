#pragma once
#include <QObject>
#include <QString>
#include <memory>
#include <dev/devs.hpp>

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    static DeviceManager &instance();
    void start();
    void stop();

    std::shared_ptr<Device> currentDevice() const { return m_dev; }
    bool isConnected() const { return m_dev != nullptr; }

    void gimbalSetAngle(float yaw, float pitch);
    void gimbalSetSpeed(int yaw, int pitch);
    void gimbalReset();
    void setZoom(double ratio);
    void setAiMode(Device::AiWorkModeType mode, Device::AiSubModeType sub = Device::AiSubModeNormal);
    void cancelAiMode();
    void setGesture(int gesture, bool enabled);
    void setFov(Device::FovType fov);
    void setHdr(int32_t mode);
    void setWhiteBalance(Device::DevWhiteBalanceType wb, int temp = 5000);
    void setFaceFocus(bool enabled);
    void setFocusAbsolute(int value);
    void setDevRunStatus(Device::DevStatus status);
    void setAiAutoZoom(bool enabled);
    void upgradeFirmware(const QString &filePath);

    QString deviceName() const;
    QString deviceVersion() const;
    QString deviceSn() const;
    ObsbotProductType productType() const;

signals:
    void deviceConnected(const QString &sn);
    void deviceDisconnected(const QString &sn);
    void statusUpdated(Device::CameraStatus status);
    void firmwareProgress(int percent, const QString &message);
    void firmwareFinished(bool success);
    void errorOccurred(const QString &message);

private:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager();
    static void onDevChanged(std::string sn, bool connected, void *ctx);
    static void onStatusUpdated(void *ctx, const void *data);
    std::shared_ptr<Device> m_dev;
    QString m_currentSn;
};
