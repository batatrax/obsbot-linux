#pragma once
#include <QObject>
#include <QString>
#include <QJsonObject>
#include <array>

struct PresetData {
    int     id        = -1;
    QString name;
    float   yaw       = 0;
    float   pitch     = 0;
    double  zoom      = 1.0;
    int     aiMode    = 0;
    int     aiSub     = 0;
    int     fov       = 0;
    int     wb        = 0;
    int     wbTemp    = 5000;
    int     focus     = 0;
    bool    faceFocus = false;
    bool    valid     = false;

    QJsonObject toJson() const;
    static PresetData fromJson(const QJsonObject &o);
};

class PresetManager : public QObject
{
    Q_OBJECT
public:
    static PresetManager &instance();
    static constexpr int MAX_PRESETS = 3;

    void load();
    void save();
    PresetData get(int id) const;
    void set(int id, const PresetData &data);
    void remove(int id);
    void rename(int id, const QString &name);
    void applyToDevice(int id);
    void captureFromDevice(int id, const QString &name);

signals:
    void presetsChanged();

private:
    explicit PresetManager(QObject *parent = nullptr);
    QString settingsPath() const;
    std::array<PresetData, MAX_PRESETS> m_presets;
};
