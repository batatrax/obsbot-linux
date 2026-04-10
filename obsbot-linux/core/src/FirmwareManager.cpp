#include "obsbot/FirmwareManager.h"
#include "obsbot/DeviceManager.h"

FirmwareManager &FirmwareManager::instance()
{ static FirmwareManager inst; return inst; }

FirmwareManager::FirmwareManager(QObject *parent) : QObject(parent)
{
    connect(&DeviceManager::instance(), &DeviceManager::firmwareProgress,
            this, &FirmwareManager::progress);
    connect(&DeviceManager::instance(), &DeviceManager::firmwareFinished,
            this, [this](bool ok) {
                emit finished(ok, ok ? "Mise a jour reussie !" : "Echec de la mise a jour");
            });
}

void FirmwareManager::upgradeFromFile(const QString &path)
{
    DeviceManager::instance().upgradeFirmware(path);
}

QString FirmwareManager::currentVersion() const
{
    return DeviceManager::instance().deviceVersion();
}
