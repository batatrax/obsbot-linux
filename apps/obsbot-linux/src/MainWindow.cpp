#include "MainWindow.h"
#include "ControlPanel.h"
#include "VideoWindow.h"
#include "Style.h"
#include <obsbot/DeviceManager.h>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QApplication>
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("OBSBOT Linux — Controls");
    // Fenetre etroite — panneau de controle uniquement
    setMinimumSize(300, 400);
    buildUi();

    auto &dm = DeviceManager::instance();
    connect(&dm, &DeviceManager::deviceConnected,    this, &MainWindow::onDeviceConnected);
    connect(&dm, &DeviceManager::deviceDisconnected, this, &MainWindow::onDeviceDisconnected);
    connect(&dm, &DeviceManager::statusUpdated,      this, &MainWindow::onStatusUpdated);

    dm.start();
    updateConnectionState(false);

    resize(360, 660);

    // Afficher la fenetre video au demarrage, positionnée juste à droite du panneau.
    // 300ms : le WM (KWin/Xwayland) a eu le temps de placer et dimensionner la fenêtre.
    m_videoWin->show();
    QTimer::singleShot(300, this, [this]{
        const QRect fg = frameGeometry();
        m_videoWin->move(fg.right() + 8, fg.top());
    });
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();

    // 1. Tuer ffmpeg — waitForFinished garanti dans stopVirtualCam()
    if (m_panel) m_panel->stopVirtualCam();

    // 2. Libérer /dev/video0 explicitement (évite que le noyau le fasse à notre place)
    if (m_videoWin) m_videoWin->stopCamera();

    // 3. Mettre la caméra en veille matérielle
    if (DeviceManager::instance().isConnected())
        DeviceManager::instance().setDevRunStatus(Device::DevStatusSleep);

    DeviceManager::instance().stop();

    // 4. Le SDK OBSBOT maintient des threads natifs qui bloquent tout cleanup.
    //    _exit() passe directement au noyau — tous les FDs restants sont fermés.
    _exit(0);
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *root = new QVBoxLayout(central);
    root->setSpacing(0);
    root->setContentsMargins(0,0,0,0);

    // ── Barre de statut ───────────────────────────────────────────────────────
    m_statusBar = new QWidget;
    m_statusBar->setFixedHeight(32);
    m_statusBar->setStyleSheet("background:#11111b;border-bottom:1px solid #313244;");
    auto *sl = new QHBoxLayout(m_statusBar);
    sl->setContentsMargins(8,0,6,0);

    auto *dot = new QLabel("●");
    dot->setObjectName("statusDot");
    dot->setStyleSheet("color:#f38ba8;font-size:9px;");

    m_devLabel = new QLabel("Waiting...");
    m_devLabel->setStyleSheet("color:#a6adc8;font-size:11px;");

    // Bouton afficher/cacher la fenetre video
    m_videoBtn = new QPushButton("🎥 Hide");
    m_videoBtn->setFixedHeight(22);
    m_videoBtn->setMinimumWidth(85);
    m_videoBtn->setToolTip(
        "Show / hide the video window.\n"
        "The window opens automatically to the right of the panel.");
    m_videoBtn->setStyleSheet(
        "QPushButton{background:#24243e;color:#89b4fa;border:1px solid #45475a;"
        "border-radius:4px;font-size:11px;padding:0 6px;}"
        "QPushButton:hover{background:#313244;}");

    sl->addWidget(dot);
    sl->addSpacing(4);
    sl->addWidget(m_devLabel,1);
    sl->addWidget(m_videoBtn);
    root->addWidget(m_statusBar);

    // ── Panneau de controle ───────────────────────────────────────────────────
    m_panel = new ControlPanel;
    root->addWidget(m_panel,1);

    // ── Fenetre video independante ────────────────────────────────────────────
    m_videoWin = new VideoWindow;

    connect(m_panel, &ControlPanel::virtualCamToggled,
            m_videoWin, &VideoWindow::onVirtualCamToggled);

    connect(m_videoBtn, &QPushButton::clicked, this, [this]{
        if (m_videoWin->isVisible()) {
            m_videoWin->hide();
            m_videoBtn->setText("🎥 Show");
        } else {
            m_videoWin->show();
            m_videoBtn->setText("🎥 Hide");
        }
    });
}

void MainWindow::onDeviceConnected(const QString &sn)
{
    auto &dm = DeviceManager::instance();
    m_devLabel->setText(QString("%1 v%2").arg(dm.deviceName(), dm.deviceVersion()));
    m_devLabel->setStyleSheet("color:#a6e3a1;font-size:11px;font-weight:bold;");
    if (auto *d = findChild<QLabel*>("statusDot"))
        d->setStyleSheet("color:#a6e3a1;font-size:9px;");
    // Réveiller la caméra au cas où elle était en veille (fermeture précédente)
    dm.setDevRunStatus(Device::DevStatusRun);
    updateConnectionState(true);
    m_videoWin->onDeviceConnected();
}

void MainWindow::onDeviceDisconnected(const QString &)
{
    m_devLabel->setText("Waiting...");
    m_devLabel->setStyleSheet("color:#a6adc8;font-size:11px;");
    if (auto *d = findChild<QLabel*>("statusDot"))
        d->setStyleSheet("color:#f38ba8;font-size:9px;");
    updateConnectionState(false);
    m_videoWin->onDeviceDisconnected();
}

void MainWindow::onStatusUpdated(Device::CameraStatus status)
{ m_panel->onStatusUpdated(status); }

void MainWindow::updateConnectionState(bool c)
{ m_panel->setEnabled(c); }

// Clavier transmis au panneau (qui le transmet au joystick si focuse)
void MainWindow::keyPressEvent(QKeyEvent *e)
{ QMainWindow::keyPressEvent(e); }

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{ QMainWindow::keyReleaseEvent(e); }
