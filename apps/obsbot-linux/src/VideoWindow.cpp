#include "VideoWindow.h"
#include <obsbot/DeviceManager.h>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoWidget>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QImageCapture>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QCameraFormat>

VideoWindow::VideoWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("OBSBOT — Video");
    setMinimumSize(320, 240);
    resize(480, 320);
    setStyleSheet("background:#000;");
    // Always on top par defaut
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    buildUi();

    m_hudTimer = new QTimer(this);
    m_hudTimer->setSingleShot(true);
    m_hudTimer->setInterval(3000);
    connect(m_hudTimer, &QTimer::timeout, this, &VideoWindow::onHudTimeout);
}

VideoWindow::~VideoWindow() { stopCamera(); }

QString VideoWindow::hudBtnStyle()
{
    return "QPushButton{"
           "background:rgba(0,0,0,140);color:rgba(255,255,255,220);"
           "border:1px solid rgba(255,255,255,40);border-radius:5px;"
           "padding:4px 8px;font-size:11px;font-weight:bold;}"
           "QPushButton:hover{background:rgba(137,180,250,180);color:#1e1e2e;}";
}

void VideoWindow::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    m_view = new QVideoWidget(this);
    m_view->setStyleSheet("background:#000;");
    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    root->addWidget(m_view);

    buildHud();
    setMouseTracking(true);
    m_view->setMouseTracking(true);
}

void VideoWindow::buildHud()
{
    // ── HUD haut : statut ────────────────────────────────────────────────────
    m_hudTop = new QWidget(this);
    m_hudTop->setStyleSheet("background:rgba(0,0,0,120);");
    m_hudTop->setFixedHeight(28);
    auto *tl = new QHBoxLayout(m_hudTop);
    tl->setContentsMargins(8,0,8,0);

    m_statusLbl = new QLabel("En attente...");
    m_statusLbl->setStyleSheet("color:rgba(255,255,255,180);font-size:11px;");
    tl->addWidget(m_statusLbl, 1);

    m_hudToggle = new QPushButton("⊗");
    m_hudToggle->setFixedSize(22,22);
    m_hudToggle->setToolTip("Masquer les controles");
    m_hudToggle->setStyleSheet(
        "QPushButton{background:transparent;color:rgba(255,255,255,120);"
        "border:none;font-size:13px;}"
        "QPushButton:hover{color:white;}");
    connect(m_hudToggle, &QPushButton::clicked, this, [this]{
        setHudVisible(!m_hudVisible);
    });
    tl->addWidget(m_hudToggle);

    // ── HUD bas : boutons + qualite ───────────────────────────────────────────
    m_hudBottom = new QWidget(this);
    m_hudBottom->setStyleSheet("background:rgba(0,0,0,120);");
    m_hudBottom->setFixedHeight(36);
    auto *bl = new QHBoxLayout(m_hudBottom);
    bl->setContentsMargins(6,4,6,4);
    bl->setSpacing(4);

    m_wakeBtn  = new QPushButton("⏻");
    m_shutBtn  = new QPushButton("⏼");
    m_pauseBtn = new QPushButton("⏸");
    m_photoBtn = new QPushButton("📷");

    for (auto *b : {m_wakeBtn, m_shutBtn, m_pauseBtn, m_photoBtn}) {
        b->setFixedSize(30, 26);
        b->setStyleSheet(hudBtnStyle());
        bl->addWidget(b);
    }

    m_wakeBtn->setToolTip("Reveiller");
    m_shutBtn->setToolTip("Eteindre");
    m_pauseBtn->setToolTip("Pause / Reprendre");
    m_photoBtn->setToolTip("Capturer photo");

    bl->addStretch();

    // Selecteur qualite compact
    m_qualCombo = new QComboBox;
    m_qualCombo->setFixedHeight(26);
    m_qualCombo->setStyleSheet(
        "QComboBox{background:rgba(0,0,0,140);color:rgba(255,255,255,200);"
        "border:1px solid rgba(255,255,255,40);border-radius:4px;padding:2px 6px;"
        "font-size:10px;}"
        "QComboBox::drop-down{border:none;width:14px;}"
        "QComboBox QAbstractItemView{background:#24243e;color:#cdd6f4;"
        "selection-background-color:#89b4fa;selection-color:#1e1e2e;}");
    m_qualCombo->addItem("1080p MJPEG");
    m_qualCombo->addItem("720p MJPEG");
    m_qualCombo->addItem("4K MJPEG");
    m_qualCombo->addItem("1080p H264");
    m_qualCombo->addItem("720p H264");
    bl->addWidget(m_qualCombo);

    connect(m_wakeBtn,  &QPushButton::clicked, this, &VideoWindow::wakeupRequested);
    connect(m_shutBtn,  &QPushButton::clicked, this, &VideoWindow::shutdownRequested);
    connect(m_pauseBtn, &QPushButton::clicked, this, [this]{
        m_paused = !m_paused;
        if (m_paused) { pause();  m_pauseBtn->setText("▶"); }
        else          { resume(); m_pauseBtn->setText("⏸"); }
        emit pauseToggled();
    });
    connect(m_photoBtn, &QPushButton::clicked, this, [this]{
        QString dir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        QDir().mkpath(dir);
        QString path = dir + "/OBSBOT_" +
            QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".jpg";
        captureToFile(path);
        m_photoBtn->setText("✓");
        QTimer::singleShot(1200, this, [this]{ m_photoBtn->setText("📷"); });
    });
    connect(m_qualCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VideoWindow::onQualityChanged);

    updateHudPositions();
}

void VideoWindow::updateHudPositions()
{
    if (m_hudTop)    m_hudTop->setGeometry(0, 0, width(), 28);
    if (m_hudBottom) m_hudBottom->setGeometry(0, height()-36, width(), 36);
}

void VideoWindow::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    updateHudPositions();
}

void VideoWindow::setHudVisible(bool visible)
{
    m_hudVisible = visible;
    if (m_hudTop)    m_hudTop->setVisible(visible);
    if (m_hudBottom) m_hudBottom->setVisible(visible);
    // Afficher seulement le toggle quand HUD cache
    if (!visible) {
        auto *show = new QPushButton("⊕", this);
        show->setObjectName("showHudBtn");
        show->setFixedSize(22,22);
        show->move(width()-26, 4);
        show->setStyleSheet(
            "QPushButton{background:rgba(0,0,0,100);color:rgba(255,255,255,120);"
            "border:none;border-radius:4px;font-size:13px;}"
            "QPushButton:hover{color:white;background:rgba(0,0,0,180);}");
        show->show();
        connect(show, &QPushButton::clicked, this, [this, show]{
            setHudVisible(true);
            show->deleteLater();
        });
    }
}

void VideoWindow::enterEvent(QEnterEvent *) { m_hudTimer->stop(); }
void VideoWindow::leaveEvent(QEvent *)
{ if (m_hudVisible) m_hudTimer->start(); }
void VideoWindow::mouseMoveEvent(QMouseEvent *)
{ if (!m_hudVisible) return; m_hudTimer->stop(); m_hudTimer->start(); }
void VideoWindow::mouseDoubleClickEvent(QMouseEvent *)
{
    // Double-clic : basculer plein ecran
    if (isFullScreen()) showNormal(); else showFullScreen();
}
void VideoWindow::onHudTimeout() {} // On garde le HUD visible

// ── Camera ────────────────────────────────────────────────────────────────────

void VideoWindow::startCamera()
{
    stopCamera();
    QCameraDevice target;
    for (auto &cam : QMediaDevices::videoInputs())
        if (cam.description().contains("OBSBOT", Qt::CaseInsensitive))
            target = cam;
    if (target.isNull() && !QMediaDevices::videoInputs().isEmpty())
        target = QMediaDevices::videoInputs().first();
    if (target.isNull()) return;

    m_camera  = new QCamera(target, this);
    m_session = new QMediaCaptureSession(this);
    m_capture = new QImageCapture(this);
    m_session->setCamera(m_camera);
    m_session->setVideoOutput(m_view);
    m_session->setImageCapture(m_capture);
    applyQuality();
    m_camera->start();
    m_paused = false;
    m_pauseBtn->setText("⏸");
}

void VideoWindow::stopCamera()
{
    if (m_camera) {
        m_camera->stop();
        delete m_capture; m_capture = nullptr;
        delete m_session; m_session = nullptr;
        delete m_camera;  m_camera  = nullptr;
    }
}

void VideoWindow::pause()  { if (m_camera) m_camera->stop(); }
void VideoWindow::resume() { if (m_camera) m_camera->start(); }

bool VideoWindow::captureToFile(const QString &path)
{
    if (!m_capture) return false;
    m_capture->captureToFile(path);
    return true;
}

void VideoWindow::onQualityChanged() { if (m_camera) { stopCamera(); startCamera(); } }

void VideoWindow::applyQuality()
{
    if (!m_camera) return;
    QString sel = m_qualCombo->currentText();
    // La resolution sera appliquee par Qt selon les formats disponibles
    // On peut affiner avec QCameraFormat si necessaire
}

void VideoWindow::onDeviceConnected()
{
    auto &dm = DeviceManager::instance();
    m_statusLbl->setText(
        QString("● %1  FW %2").arg(dm.deviceName(), dm.deviceVersion()));
    m_statusLbl->setStyleSheet("color:rgba(166,227,161,220);font-size:11px;font-weight:bold;");
    m_wakeBtn->setEnabled(true);
    m_shutBtn->setEnabled(true);
    m_pauseBtn->setEnabled(true);
    m_photoBtn->setEnabled(true);
    startCamera();
}

void VideoWindow::onDeviceDisconnected()
{
    m_statusLbl->setText("Camera deconnectee");
    m_statusLbl->setStyleSheet("color:rgba(243,139,168,200);font-size:11px;");
    m_wakeBtn->setEnabled(false);
    m_shutBtn->setEnabled(false);
    m_pauseBtn->setEnabled(false);
    m_photoBtn->setEnabled(false);
    stopCamera();
}
