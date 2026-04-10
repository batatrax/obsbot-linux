#include "MainWindow.h"
#include "VideoWidget.h"
#include "GimbalWidget.h"
#include "AiWidget.h"
#include "CameraWidget.h"
#include "PresetWidget.h"
#include "GestureWidget.h"
#include "FirmwareWidget.h"
#include "StatusBar.h"
#include "TrayIcon.h"
#include "Style.h"
#include <obsbot/DeviceManager.h>
#include <KLocalizedString>
#include <KNotification>
#include <KActionCollection>
#include <QListWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent)
{
    setWindowTitle(i18n("OBSBOT KDE"));
    setMinimumSize(1200, 750);
    setStyleSheet(ObsbotStyle::globalStyleSheet());

    m_tray = new TrayIcon(this);
    buildUi();
    setupActions();

    auto &dm = DeviceManager::instance();
    connect(&dm, &DeviceManager::deviceConnected,    this, &MainWindow::onDeviceConnected);
    connect(&dm, &DeviceManager::deviceDisconnected, this, &MainWindow::onDeviceDisconnected);
    connect(&dm, &DeviceManager::statusUpdated,      this, &MainWindow::onStatusUpdated);
    dm.start();
    updateConnectionState(false);
}

void MainWindow::setupActions()
{
    // Les actions KDE (raccourcis clavier, menus) seraient definies ici
    // via actionCollection() et setupGUI()
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *root = new QHBoxLayout(central);
    root->setSpacing(0);
    root->setContentsMargins(0,0,0,0);

    // Sidebar
    auto *sidebar = new QWidget;
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet("background:#181825;");
    auto *sl = new QVBoxLayout(sidebar);
    sl->setContentsMargins(0,0,0,0);
    sl->setSpacing(0);

    auto *logo = new QWidget;
    logo->setStyleSheet("background:#11111b; padding:16px;");
    auto *ll = new QVBoxLayout(logo);
    auto *title = new QLabel("OBSBOT");
    title->setStyleSheet("color:#89b4fa; font-size:20px; font-weight:bold; letter-spacing:2px;");
    auto *sub = new QLabel("KDE Controller");
    sub->setStyleSheet("color:#a6adc8; font-size:11px;");
    ll->addWidget(title); ll->addWidget(sub);
    sl->addWidget(logo);

    m_nav = new QListWidget;
    m_nav->setStyleSheet(R"(
        QListWidget{background:#181825;border:none;outline:none;}
        QListWidget::item{color:#a6adc8;padding:12px 20px;border-left:3px solid transparent;font-size:13px;}
        QListWidget::item:selected{background:#24243e;color:#89b4fa;border-left:3px solid #89b4fa;font-weight:bold;}
        QListWidget::item:hover:!selected{background:#1e1e2e;color:#cdd6f4;}
    )");
    for (auto &item : QStringList{"🎥  Gimbal & Zoom","🤖  IA Tracking","⚙  Caméra","📌  Presets","✋  Gestes","🔄  Firmware"}) {
        auto *i = new QListWidgetItem(item);
        i->setSizeHint({200,46});
        m_nav->addItem(i);
    }
    sl->addWidget(m_nav,1);

    auto *devInfo = new QWidget;
    devInfo->setStyleSheet("background:#11111b;padding:12px;border-top:1px solid #313244;");
    auto *dl = new QVBoxLayout(devInfo);
    auto *devLabel = new QLabel(i18n("Aucun appareil"));
    devLabel->setObjectName("devLabel");
    devLabel->setStyleSheet("color:#a6adc8;font-size:11px;");
    devLabel->setWordWrap(true);
    dl->addWidget(devLabel);
    sl->addWidget(devInfo);
    root->addWidget(sidebar);

    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("color:#45475a;");
    root->addWidget(sep);

    // Contenu
    auto *content = new QWidget;
    auto *cl = new QVBoxLayout(content);
    cl->setContentsMargins(0,0,0,0);
    cl->setSpacing(0);

    auto *mainArea = new QHBoxLayout;
    m_video    = new VideoWidget;
    mainArea->addWidget(m_video,3);

    m_stack    = new QStackedWidget;
    m_stack->setMinimumWidth(380);
    m_stack->setMaximumWidth(480);
    m_gimbal   = new GimbalWidget;
    m_ai       = new AiWidget;
    m_camera   = new CameraWidget;
    m_presets  = new PresetWidget;
    m_gestures = new GestureWidget;
    m_firmware = new FirmwareWidget;
    m_stack->addWidget(m_gimbal);
    m_stack->addWidget(m_ai);
    m_stack->addWidget(m_camera);
    m_stack->addWidget(m_presets);
    m_stack->addWidget(m_gestures);
    m_stack->addWidget(m_firmware);
    mainArea->addWidget(m_stack);
    cl->addLayout(mainArea,1);

    m_statusBar = new StatusBar;
    cl->addWidget(m_statusBar);
    root->addWidget(content,1);

    connect(m_nav, &QListWidget::currentRowChanged, this, &MainWindow::onNavChanged);
    m_nav->setCurrentRow(0);
}

void MainWindow::onNavChanged(int row) { m_stack->setCurrentIndex(row); }

void MainWindow::onDeviceConnected(const QString &sn)
{
    auto &dm = DeviceManager::instance();
    if (auto *l = findChild<QLabel*>("devLabel"))
        l->setText(QString("<b style=\'color:#a6e3a1;\'>● %1</b><br><span style=\'color:#585b70;\'>FW %2</span>")
            .arg(dm.deviceName(), dm.deviceVersion()));
    m_statusBar->setStatus(true, dm.deviceName(), dm.deviceVersion());
    m_tray->setConnected(true, dm.deviceName());
    updateConnectionState(true);
    m_video->startCamera();

    auto *notif = new KNotification("deviceConnected", KNotification::CloseOnTimeout, this);
    notif->setTitle(i18n("OBSBOT connectée"));
    notif->setText(i18n("%1 détectée — firmware %2", dm.deviceName(), dm.deviceVersion()));
    notif->setIconName("camera-web");
    notif->sendEvent();
}

void MainWindow::onDeviceDisconnected(const QString &)
{
    if (auto *l = findChild<QLabel*>("devLabel"))
        l->setText("<span style=\'color:#f38ba8;\'>● Déconnecté</span>");
    m_statusBar->setStatus(false,{},{});
    m_tray->setConnected(false);
    updateConnectionState(false);
    m_video->stopCamera();
}

void MainWindow::onStatusUpdated(Device::CameraStatus s) { m_camera->onStatusUpdated(s); }

void MainWindow::updateConnectionState(bool c)
{
    m_gimbal->setEnabled(c); m_ai->setEnabled(c); m_camera->setEnabled(c);
    m_presets->setEnabled(c); m_gestures->setEnabled(c); m_firmware->setEnabled(c);
}
