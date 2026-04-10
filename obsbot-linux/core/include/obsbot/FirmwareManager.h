#pragma once
#include <QObject>
#include <QString>

class FirmwareManager : public QObject
{
    Q_OBJECT
public:
    static FirmwareManager &instance();
    void upgradeFromFile(const QString &path);
    QString currentVersion() const;

signals:
    void progress(int percent, const QString &message);
    void finished(bool success, const QString &message);

private:
    explicit FirmwareManager(QObject *parent = nullptr);
};
