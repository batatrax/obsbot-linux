#pragma once
#include <QObject>
#include <KStatusNotifierItem>

class TrayIcon : public QObject {
    Q_OBJECT
public:
    explicit TrayIcon(QObject *parent = nullptr);
    void setConnected(bool connected, const QString &name = {});

private:
    KStatusNotifierItem *m_tray;
};
