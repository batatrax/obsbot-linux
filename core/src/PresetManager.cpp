#include "obsbot/PresetManager.h"
#include "obsbot/DeviceManager.h"
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDir>

PresetManager &PresetManager::instance()
{ static PresetManager inst; return inst; }

PresetManager::PresetManager(QObject *parent) : QObject(parent) { load(); }

QString PresetManager::settingsPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + "/presets.json";
}

QJsonObject PresetData::toJson() const
{
    return {
        {"id",        id},
        {"name",      name},
        {"yaw",       yaw},
        {"pitch",     pitch},
        {"zoom",      zoom},
        {"aiMode",    aiMode},
        {"aiSub",     aiSub},
        {"fov",       fov},
        {"wb",        wb},
        {"wbTemp",    wbTemp},
        {"focus",     focus},
        {"faceFocus", faceFocus},
        {"valid",     valid}
    };
}

PresetData PresetData::fromJson(const QJsonObject &o)
{
    PresetData d;
    d.id        = o["id"].toInt(-1);
    d.name      = o["name"].toString();
    d.yaw       = float(o["yaw"].toDouble());
    d.pitch     = float(o["pitch"].toDouble());
    d.zoom      = o["zoom"].toDouble(1.0);
    d.aiMode    = o["aiMode"].toInt();
    d.aiSub     = o["aiSub"].toInt();
    d.fov       = o["fov"].toInt();
    d.wb        = o["wb"].toInt();
    d.wbTemp    = o["wbTemp"].toInt(5000);
    d.focus     = o["focus"].toInt();
    d.faceFocus = o["faceFocus"].toBool();
    d.valid     = o["valid"].toBool();
    return d;
}

void PresetManager::load()
{
    QFile f(settingsPath());
    if (!f.open(QIODevice::ReadOnly)) return;
    auto arr = QJsonDocument::fromJson(f.readAll()).array();
    for (auto v : arr) {
        auto d = PresetData::fromJson(v.toObject());
        if (d.id >= 0 && d.id < MAX_PRESETS) m_presets[d.id] = d;
    }
}

void PresetManager::save()
{
    QJsonArray arr;
    for (auto &p : m_presets) if (p.valid) arr.append(p.toJson());
    QFile f(settingsPath());
    QFileInfo(f).dir().mkpath(".");
    if (f.open(QIODevice::WriteOnly))
        f.write(QJsonDocument(arr).toJson());
}

PresetData PresetManager::get(int id) const
{
    if (id < 0 || id >= MAX_PRESETS) return {};
    return m_presets[id];
}

void PresetManager::set(int id, const PresetData &data)
{
    if (id < 0 || id >= MAX_PRESETS) return;
    m_presets[id] = data;
    save();
    emit presetsChanged();
}

void PresetManager::remove(int id)
{
    if (id < 0 || id >= MAX_PRESETS) return;
    m_presets[id] = PresetData{};
    save();
    emit presetsChanged();
}

void PresetManager::rename(int id, const QString &name)
{
    if (id < 0 || id >= MAX_PRESETS) return;
    m_presets[id].name = name;
    save();
    emit presetsChanged();
}

void PresetManager::captureFromDevice(int id, const QString &name)
{
    if (id < 0 || id >= MAX_PRESETS) return;
    auto &dm = DeviceManager::instance();
    PresetData d;
    d.id    = id;
    d.name  = name.isEmpty() ? QString("Preset %1").arg(id + 1) : name;
    d.valid = true;
    // Les valeurs actuelles seraient lues depuis le statut camera
    set(id, d);
}

void PresetManager::applyToDevice(int id)
{
    if (id < 0 || id >= MAX_PRESETS) return;
    auto d = m_presets[id];
    if (!d.valid) return;
    auto &dm = DeviceManager::instance();
    dm.gimbalSetAngle(d.yaw, d.pitch);
    dm.setZoom(d.zoom);
    dm.setFov(static_cast<Device::FovType>(d.fov));
    dm.setAiMode(static_cast<Device::AiWorkModeType>(d.aiMode),
                 static_cast<Device::AiSubModeType>(d.aiSub));
    dm.setWhiteBalance(static_cast<Device::DevWhiteBalanceType>(d.wb), d.wbTemp);
    dm.setFocusAbsolute(d.focus);
    dm.setFaceFocus(d.faceFocus);
}
