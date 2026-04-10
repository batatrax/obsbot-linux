#include "TrayIcon.h"
#include <QMenu>
#include <QAction>

TrayIcon::TrayIcon(QObject *parent) : QObject(parent)
{
    m_tray = new KStatusNotifierItem("obsbot-kde", this);
    m_tray->setTitle("OBSBOT KDE");
    m_tray->setIconByName("camera-web");
    m_tray->setStatus(KStatusNotifierItem::Passive);
    m_tray->setCategory(KStatusNotifierItem::Hardware);

    auto *menu = new QMenu;
    menu->addAction(QIcon::fromTheme("camera-web"), "OBSBOT KDE");
    menu->addSeparator();
    menu->addAction(QIcon::fromTheme("application-exit"), "Quitter",
        qApp, &QApplication::quit);
    m_tray->setContextMenu(menu);
}

void TrayIcon::setConnected(bool connected, const QString &name)
{
    m_tray->setStatus(connected ? KStatusNotifierItem::Active
                                : KStatusNotifierItem::Passive);
    m_tray->setToolTip("camera-web", "OBSBOT KDE",
        connected ? name + " connectée" : "Aucun appareil");
    m_tray->setIconByName(connected ? "camera-web" : "camera-web-disabled");
}
