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
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("OBSBOT Linux — Controles");
    // Fenetre etroite — panneau de controle uniquement
    setMinimumSize(300, 500);
    resize(340, 720);
    buildUi();

    auto &dm = DeviceManager::instance();
    connect(&dm, &DeviceManager::deviceConnected,    this, &MainWindow::onDeviceConnected);
    connect(&dm, &DeviceManager::deviceDisconnected, this, &MainWindow::onDeviceDisconnected);
    connect(&dm, &DeviceManager::statusUpdated,      this, &MainWindow::onStatusUpdated);

    // Connexion wakeup/shutdown depuis la fenetre video
    connect(m_videoWin, &VideoWindow::wakeupRequested, this, []{
        DeviceManager::instance().setDevRunStatus(Device::DevStatusRun);
    });
    connect(m_videoWin, &VideoWindow::shutdownRequested, this, []{
        DeviceManager::instance().setDevRunStatus(Device::DevStatusSleep);
    });

    dm.start();
    updateConnectionState(false);

    // Afficher la fenetre video au demarrage
    m_videoWin->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Eteindre camera sans dialogue et fermer
    if (DeviceManager::instance().isConnected())
        DeviceManager::instance().setDevRunStatus(Device::DevStatusSleep);
    DeviceManager::instance().stop();
    m_videoWin->close();
    event->accept();
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

    m_devLabel = new QLabel("En attente...");
    m_devLabel->setStyleSheet("color:#a6adc8;font-size:11px;");

    // Bouton afficher/cacher la fenetre video
    m_videoBtn = new QPushButton("📺");
    m_videoBtn->setFixedSize(26,22);
    m_videoBtn->setToolTip("Afficher / masquer la fenetre video");
    m_videoBtn->setStyleSheet(
        "QPushButton{background:#24243e;color:#89b4fa;border:1px solid #45475a;"
        "border-radius:4px;font-size:12px;}"
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

    connect(m_videoBtn, &QPushButton::clicked, this, [this]{
        if (m_videoWin->isVisible())
            m_videoWin->hide();
        else
            m_videoWin->show();
    });
}

void MainWindow::onDeviceConnected(const QString &sn)
{
    auto &dm = DeviceManager::instance();
    m_devLabel->setText(QString("%1 v%2").arg(dm.deviceName(), dm.deviceVersion()));
    m_devLabel->setStyleSheet("color:#a6e3a1;font-size:11px;font-weight:bold;");
    if (auto *d = findChild<QLabel*>("statusDot"))
        d->setStyleSheet("color:#a6e3a1;font-size:9px;");
    updateConnectionState(true);
    m_videoWin->onDeviceConnected();
}

void MainWindow::onDeviceDisconnected(const QString &)
{
    m_devLabel->setText("En attente...");
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
