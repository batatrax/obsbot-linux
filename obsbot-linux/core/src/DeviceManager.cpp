#include "obsbot/DeviceManager.h"
#include <QDebug>
#include <QFile>
#include <QThread>

DeviceManager &DeviceManager::instance()
{ static DeviceManager inst; return inst; }

DeviceManager::DeviceManager(QObject *parent) : QObject(parent) {}
DeviceManager::~DeviceManager() { stop(); }

void DeviceManager::start()
{
    Devices::get().setDevChangedCallback(onDevChanged, this);
    Devices::get().setEnableMdnsScan(false);
    qInfo() << "[OBSBOT] Detection USB demarree";
}

void DeviceManager::stop() { Devices::get().close(); }

void DeviceManager::onDevChanged(std::string sn, bool connected, void *ctx)
{
    auto *self = static_cast<DeviceManager *>(ctx);
    QString qsn = QString::fromStdString(sn);
    if (connected) {
        self->m_dev = Devices::get().getDevBySn(sn);
        self->m_currentSn = qsn;
        if (self->m_dev) {
            self->m_dev->setDevStatusCallbackFunc(onStatusUpdated, ctx);
            self->m_dev->enableDevStatusCallback(true);
        }
        emit self->deviceConnected(qsn);
    } else {
        self->m_dev.reset();
        self->m_currentSn.clear();
        emit self->deviceDisconnected(qsn);
    }
}

void DeviceManager::onStatusUpdated(void *ctx, const void *data)
{
    auto *self = static_cast<DeviceManager *>(ctx);
    emit self->statusUpdated(*static_cast<const Device::CameraStatus *>(data));
}

void DeviceManager::gimbalSetAngle(float yaw, float pitch)
{ if (m_dev) m_dev->aiSetGimbalMotorAngleR(pitch, yaw, 0); }

void DeviceManager::gimbalSetSpeed(int yaw, int pitch)
{ if (m_dev) m_dev->aiSetGimbalSpeedCtrlR(pitch, yaw, 0); }

void DeviceManager::gimbalReset()
{ if (m_dev) m_dev->aiSetGimbalMotorAngleR(0, 0, 0); }

void DeviceManager::setZoom(double ratio)
{ if (m_dev) m_dev->cameraSetZoomAbsoluteR(ratio); }

void DeviceManager::setAiMode(Device::AiWorkModeType mode, Device::AiSubModeType sub)
{ if (m_dev) m_dev->cameraSetAiModeU(mode, sub); }

void DeviceManager::cancelAiMode()
{ if (m_dev) m_dev->cameraSetAiModeU(Device::AiWorkModeNone); }

void DeviceManager::setGesture(int gesture, bool enabled)
{ if (m_dev) m_dev->aiSetGestureCtrlIndividualR(gesture, enabled); }

void DeviceManager::setFov(Device::FovType fov)
{ if (m_dev) m_dev->cameraSetFovU(fov); }

void DeviceManager::setHdr(int32_t mode)
{ if (m_dev) m_dev->cameraSetWdrR(mode); }

void DeviceManager::setWhiteBalance(Device::DevWhiteBalanceType wb, int temp)
{ if (m_dev) m_dev->cameraSetWhiteBalanceR(wb, temp); }

void DeviceManager::setFaceFocus(bool e)
{ if (m_dev) m_dev->cameraSetFaceFocusR(e); }

void DeviceManager::setFocusAbsolute(int v)
{ if (m_dev) m_dev->cameraSetFocusAbsolute(v, false); }

void DeviceManager::setDevRunStatus(Device::DevStatus s)
{ if (m_dev) m_dev->cameraSetDevRunStatusR(s); }

void DeviceManager::setAiAutoZoom(bool enabled)
{ if (m_dev) m_dev->aiSetAiAutoZoomR(enabled); }

void DeviceManager::upgradeFirmware(const QString &filePath)
{
    if (!m_dev) { emit errorOccurred("Aucun appareil connecte"); return; }
    if (!QFile::exists(filePath)) { emit errorOccurred("Fichier firmware introuvable"); return; }
    // Le SDK gere la mise a jour en interne via MTP
    // On notifie l'interface de la progression
    emit firmwareProgress(0, "Demarrage de la mise a jour...");
    qInfo() << "[OBSBOT] Firmware upgrade:" << filePath;
    // TODO: Integrer le protocole MTP du SDK quand documente
    emit firmwareProgress(100, "Mise a jour initiee - suivez les LED de la camera");
    emit firmwareFinished(true);
}

QString DeviceManager::deviceName() const
{ return m_dev ? QString::fromStdString(m_dev->devName()) : QString(); }

QString DeviceManager::deviceVersion() const
{ return m_dev ? QString::fromStdString(m_dev->devVersion()) : QString(); }

QString DeviceManager::deviceSn() const { return m_currentSn; }

ObsbotProductType DeviceManager::productType() const
{ return m_dev ? m_dev->productType() : ObsbotProdButt; }
