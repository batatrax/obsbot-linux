#include "VideoWindow.h"
#include <obsbot/DeviceManager.h>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QImageCapture>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QCameraFormat>
#include <QVideoFrameFormat>
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
#include <QSettings>
#include <QProcess>

// Downscale progressif : deux fois plus net qu'un seul passage SmoothTransformation
static QImage sharpScale(QImage img, const QSize &target)
{
    while (img.width() > target.width() * 2 || img.height() > target.height() * 2)
        img = img.scaled(img.width() / 2, img.height() / 2,
                         Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return img.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

VideoWindow::VideoWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle(tr("OBSBOT — Video"));
    setMinimumSize(320, 240);
    resize(480, 320);
    setStyleSheet("background:#000;");
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    QSettings s("obsbot-linux", "VideoWindow");
    m_mirrorEnabled = s.value("mirror/enabled", true).toBool();
    m_flipV         = s.value("flip/vertical",  false).toBool();

    buildUi();

    // Overlay zoom : mis à jour depuis le status SDK
    connect(&DeviceManager::instance(), &DeviceManager::statusUpdated,
            this, [this](Device::CameraStatus status){
        // zoom_ratio : 0-100 → 1x-4x
        m_currentZoom = 1.0f + (status.tiny.zoom_ratio / 100.0f) * 3.0f;
        if (m_zoomOverlay)
            m_zoomOverlay->setText(QString("%1×").arg(double(m_currentZoom), 0, 'f', 1));
    });

    m_hudTimer = new QTimer(this);
    m_hudTimer->setSingleShot(true);
    m_hudTimer->setInterval(3000);
    connect(m_hudTimer, &QTimer::timeout, this, &VideoWindow::onHudTimeout);
}

VideoWindow::~VideoWindow()
{
    stopPipeWireBridge();
    stopCamera();
}

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

    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet("background:#000;");
    m_videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    root->addWidget(m_videoLabel);
    
    m_virtualMsgLbl = new QLabel(tr("Virtual Camera Active\nVideo shared with the system"), m_videoLabel);
    m_virtualMsgLbl->setAlignment(Qt::AlignCenter);
    m_virtualMsgLbl->setStyleSheet("color: #a6e3a1; font-size: 18px; font-weight: bold; background: rgba(30, 30, 46, 200); border-radius: 10px; padding: 20px;");
    m_virtualMsgLbl->hide();

    m_sink = new QVideoSink(this);
    connect(m_sink, &QVideoSink::videoFrameChanged,
            this, [this](const QVideoFrame &frame) {
        if (!frame.isValid()) return;
        QImage img = frame.toImage();
        if (img.isNull()) return;
        if (m_mirrorEnabled) img = img.flipped(Qt::Horizontal);
        if (m_flipV)         img = img.flipped(Qt::Vertical);
        m_videoLabel->setPixmap(QPixmap::fromImage(
            sharpScale(std::move(img), m_videoLabel->size())));
    });

    // Overlay zoom — affiché en bas à gauche, au-dessus du HUD
    m_zoomOverlay = new QLabel("1.0×", m_videoLabel);
    m_zoomOverlay->setStyleSheet(
        "color:rgba(255,255,255,200); font-size:12px; font-weight:bold;"
        "background:rgba(0,0,0,130); border-radius:3px; padding:1px 5px;");
    m_zoomOverlay->hide(); // visible seulement quand connecté

    buildHud();
    setMouseTracking(true);
    m_videoLabel->setMouseTracking(true);
}

void VideoWindow::buildHud()
{
    m_hudTop = new QWidget(this);
    m_hudTop->setStyleSheet("background:rgba(0,0,0,120);");
    m_hudTop->setFixedHeight(28);
    auto *tl = new QHBoxLayout(m_hudTop);
    tl->setContentsMargins(8,0,8,0);

    m_statusLbl = new QLabel(tr("Waiting..."));
    m_statusLbl->setStyleSheet("color:rgba(255,255,255,180);font-size:11px;");
    tl->addWidget(m_statusLbl, 1);

    m_hudToggle = new QPushButton("⊗");
    m_hudToggle->setFixedSize(22,22);
    m_hudToggle->setToolTip(tr("Hide controls"));
    m_hudToggle->setStyleSheet(
        "QPushButton{background:transparent;color:rgba(255,255,255,120);"
        "border:none;font-size:13px;}"
        "QPushButton:hover{color:white;}");
    connect(m_hudToggle, &QPushButton::clicked, this, [this]{
        setHudVisible(!m_hudVisible);
    });
    tl->addWidget(m_hudToggle);

    m_hudBottom = new QWidget(this);
    m_hudBottom->setStyleSheet("background:rgba(0,0,0,120);");
    m_hudBottom->setFixedHeight(36);
    auto *bl = new QHBoxLayout(m_hudBottom);
    bl->setContentsMargins(6,4,6,4);
    bl->setSpacing(4);

    m_powerBtn  = new QPushButton("⏼");
    m_pauseBtn  = new QPushButton("⏸");
    m_photoBtn  = new QPushButton("📷");
    m_mirrorBtn = new QPushButton("⟺");
    m_flipVBtn  = new QPushButton("⇅");

    for (auto *b : {m_powerBtn, m_pauseBtn, m_photoBtn, m_mirrorBtn, m_flipVBtn}) {
        b->setFixedSize(30, 26);
        b->setStyleSheet(hudBtnStyle());
        bl->addWidget(b);
    }

    m_mirrorBtn->setToolTip(tr("Horizontal mirror (saved)"));
    m_flipVBtn->setToolTip(tr("Vertical flip (saved)"));

    // Style actif/inactif pour les boutons de flip
    static const QString kDimStyle =
        "QPushButton{background:rgba(0,0,0,140);color:rgba(255,255,255,60);"
        "border:1px solid rgba(255,255,255,20);border-radius:5px;"
        "padding:4px 8px;font-size:11px;font-weight:bold;}";
    auto updateMirrorStyle = [this]{
        m_mirrorBtn->setStyleSheet(m_mirrorEnabled ? hudBtnStyle() : kDimStyle);
    };
    auto updateFlipVStyle = [this]{
        m_flipVBtn->setStyleSheet(m_flipV ? hudBtnStyle() : kDimStyle);
    };
    updateMirrorStyle();
    updateFlipVStyle();

    m_powerBtn->setToolTip(tr("Sleep"));
    m_pauseBtn->setToolTip(tr("Pause / Resume"));
    m_photoBtn->setToolTip(tr("Take photo"));

    connect(m_mirrorBtn, &QPushButton::clicked, this, [this, updateMirrorStyle]{
        m_mirrorEnabled = !m_mirrorEnabled;
        QSettings("obsbot-linux", "VideoWindow").setValue("mirror/enabled", m_mirrorEnabled);
        updateMirrorStyle();
    });
    connect(m_flipVBtn, &QPushButton::clicked, this, [this, updateFlipVStyle]{
        m_flipV = !m_flipV;
        QSettings("obsbot-linux", "VideoWindow").setValue("flip/vertical", m_flipV);
        updateFlipVStyle();
    });

    bl->addStretch();

    m_qualCombo = new QComboBox;
    m_qualCombo->setFixedHeight(26);
    m_qualCombo->setStyleSheet(
        "QComboBox{background:rgba(0,0,0,140);color:rgba(255,255,255,200);"
        "border:1px solid rgba(255,255,255,40);border-radius:4px;padding:2px 6px;"
        "font-size:10px;}"
        "QComboBox::drop-down{border:none;width:14px;}"
        "QComboBox QAbstractItemView{background:#24243e;color:#cdd6f4;"
        "selection-background-color:#89b4fa;selection-color:#1e1e2e;}");
    // 720p par défaut : moins de downscaling → image plus nette en aperçu
    m_qualCombo->addItem(tr("1080p MJPEG"));
    m_qualCombo->addItem(tr("720p MJPEG"));
    m_qualCombo->addItem(tr("4K MJPEG"));
    m_qualCombo->addItem(tr("1080p H264"));
    m_qualCombo->addItem(tr("720p H264"));
    bl->addWidget(m_qualCombo);

    connect(m_powerBtn, &QPushButton::clicked, this, [this]{
        if (m_sleeping) {
            DeviceManager::instance().setDevRunStatus(Device::DevStatusRun);
            m_sleeping = false;
            m_powerBtn->setText("⏼");
            m_powerBtn->setToolTip(tr("Sleep"));
            QTimer::singleShot(800, this, &VideoWindow::startCamera);
        } else {
            stopCamera();
            DeviceManager::instance().setDevRunStatus(Device::DevStatusSleep);
            m_sleeping = true;
            m_powerBtn->setText("⏻");
            m_powerBtn->setToolTip(tr("Wake up"));
        }
    });
    connect(m_pauseBtn, &QPushButton::clicked, this, [this]{
        m_paused = !m_paused;
        if (m_paused) { pause();  m_pauseBtn->setText("▶"); }
        else          { resume(); m_pauseBtn->setText("⏸"); }
        emit pauseToggled();
    });
    connect(m_photoBtn, &QPushButton::clicked, this, [this]{
        if (!m_capture || !m_capture->isReadyForCapture()) return;
        QString dir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                      + "/OBSBOT";
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

    if (m_virtualMsgLbl && m_virtualMsgLbl->isVisible()) {
        m_virtualMsgLbl->adjustSize();
        m_virtualMsgLbl->move((width() - m_virtualMsgLbl->width()) / 2,
                              (height() - m_virtualMsgLbl->height()) / 2);
    }
    // Overlay zoom : coin bas-gauche, au-dessus du HUD
    if (m_zoomOverlay) {
        m_zoomOverlay->adjustSize();
        m_zoomOverlay->move(8, height() - 36 - m_zoomOverlay->height() - 4);
    }
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

void VideoWindow::enterEvent(QEnterEvent *)  { m_hudTimer->stop(); }
void VideoWindow::leaveEvent(QEvent *)       { if (m_hudVisible) m_hudTimer->start(); }
void VideoWindow::mouseMoveEvent(QMouseEvent *)
{ if (!m_hudVisible) return; m_hudTimer->stop(); m_hudTimer->start(); }
void VideoWindow::mouseDoubleClickEvent(QMouseEvent *)
{ if (isFullScreen()) showNormal(); else showFullScreen(); }
void VideoWindow::onHudTimeout() {}

// ── Camera ────────────────────────────────────────────────────────────────────

void VideoWindow::startCamera()
{
    stopCamera();
    
    if (m_virtualMsgLbl) m_virtualMsgLbl->hide();
    
    QCameraDevice target;
    const auto inputs = QMediaDevices::videoInputs();
    for (const auto &cam : inputs) {
        const QString desc = cam.description();
        // Exclure les devices virtuels (v4l2loopback) même si leur label contient "OBSBOT"
        if (desc.contains("OBSBOT", Qt::CaseInsensitive) &&
            !desc.contains("Virtual", Qt::CaseInsensitive)) {
            target = cam;
            break;
        }
    }
    if (target.isNull()) {
        for (const auto &cam : inputs) {
            if (!cam.description().contains("Virtual", Qt::CaseInsensitive)) {
                target = cam;
                break;
            }
        }
    }
    if (target.isNull()) return;

    m_camera  = new QCamera(target, this);
    m_session = new QMediaCaptureSession(this);
    m_capture = new QImageCapture(this);
    m_session->setCamera(m_camera);
    m_session->setVideoOutput(m_sink);
    m_session->setImageCapture(m_capture);
    
    // Appliquer la qualite selectionnee dans la combobox
    // Il faut le faire APRES avoir demarre la camera pour que Qt prenne en compte le format
    // Sauvegarder la photo avec le même flip horizontal que le preview
    connect(m_capture, &QImageCapture::imageCaptured,
            this, [this](int, const QImage &img) {
        if (m_pendingCapturePath.isEmpty()) return;
        QImage out = img;
        if (m_mirrorEnabled) out = out.flipped(Qt::Horizontal);
        if (m_flipV)         out = out.flipped(Qt::Vertical);
        out.save(m_pendingCapturePath, "JPEG", 95);
        m_pendingCapturePath.clear();
    });

    applyQuality();
    m_camera->start();

    if (m_camera->isFocusModeSupported(QCamera::FocusModeAuto))
        m_camera->setFocusMode(QCamera::FocusModeAuto);

    m_paused = false;
    m_pauseBtn->setText("⏸");
    if (m_zoomOverlay) { m_zoomOverlay->show(); updateHudPositions(); }
}

void VideoWindow::stopCamera()
{
    if (m_camera) {
        m_camera->stop();
        delete m_capture; m_capture = nullptr;
        delete m_session; m_session = nullptr;
        delete m_camera;  m_camera  = nullptr;
    }
    m_videoLabel->clear();
    if (m_zoomOverlay) m_zoomOverlay->hide();

    if (m_virtualMsgLbl && m_virtualCamActive) {
        m_virtualMsgLbl->show();
        updateHudPositions();
    }
}

void VideoWindow::onVirtualCamToggled(bool active)
{
    m_virtualCamActive = active;
    if (m_virtualMsgLbl) {
        m_virtualMsgLbl->setVisible(active);
        if (active) updateHudPositions();
    }
    if (active) {
        startPipeWireBridge();
    } else {
        stopPipeWireBridge();
        startCamera();
    }
}

// ── Virtual cam — ffmpeg MJPEG→YUYV422 ───────────────────────────────────────

void VideoWindow::startPipeWireBridge()
{
    stopPipeWireBridge();

    // Tuer tout ffmpeg orphelin qui écrirait encore sur /dev/video99 (session précédente)
    QProcess::execute("pkill", {"-f", "ffmpeg.*video99"});

    // Trouver le chemin /dev/videoX de la caméra OBSBOT (exclure /dev/video99)
    QString camPath;
    for (const auto &cam : QMediaDevices::videoInputs()) {
        if (cam.description().contains("OBSBOT", Qt::CaseInsensitive) &&
            !cam.description().contains("Virtual", Qt::CaseInsensitive)) {
            camPath = QString::fromLatin1(cam.id());
            break;
        }
    }
    if (camPath.isEmpty() && !QMediaDevices::videoInputs().isEmpty())
        camPath = QString::fromLatin1(QMediaDevices::videoInputs().first().id());
    if (camPath.isEmpty()) {
        qWarning("virtual cam: aucune caméra trouvée");
        return;
    }

    // Libérer /dev/videoX avant que ffmpeg l'ouvre
    stopCamera();

    m_vcamProcess = new QProcess(this);
    m_vcamProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_vcamProcess, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError err) {
        qWarning("virtual cam: ffmpeg erreur %d", (int)err);
    });

    // Attendre 500ms que Qt libère effectivement le device V4L2
    const QString path = camPath;
    QTimer::singleShot(500, this, [this, path]() {
        if (!m_vcamProcess) return;
        // I420 (yuv420p) : format natif de libwebrtc, supporté universellement par Firefox/Chrome.
        // NV12 nécessite une conversion interne qui peut produire un écran noir sur certains drivers.
        // YUYV422 fonctionne pour OBS (V4L2 direct) mais mal supporté par WebRTC.
        m_vcamProcess->start("ffmpeg", {
            "-f", "v4l2", "-input_format", "mjpeg",
            "-video_size", "1920x1080", "-framerate", "30",
            "-i", path,
            "-pix_fmt", "yuv420p",
            "-f", "v4l2", "/dev/video99"
        });
        qDebug("virtual cam: ffmpeg lancé %s → /dev/video99 (MJPEG→I420)", qPrintable(path));
    });
    // Redémarrer WirePlumber 2s après que ffmpeg ait posé le format sur /dev/video99
    // pour que Firefox voie le bon format via PipeWire
    QTimer::singleShot(2000, this, []() {
        QProcess::startDetached("systemctl", {"--user", "restart", "wireplumber"});
        qDebug("virtual cam: WirePlumber redémarré pour republier le format");
    });
}

void VideoWindow::stopPipeWireBridge()
{
    if (!m_vcamProcess) return;
    m_vcamProcess->terminate();
    if (!m_vcamProcess->waitForFinished(2000))
        m_vcamProcess->kill();
    m_vcamProcess->deleteLater();
    m_vcamProcess = nullptr;
    qDebug("virtual cam: ffmpeg arrêté");
}


void VideoWindow::pause()  { if (m_camera) m_camera->stop(); }
void VideoWindow::resume() { if (m_camera) m_camera->start(); }

bool VideoWindow::captureToFile(const QString &path)
{
    if (!m_capture || !m_capture->isReadyForCapture()) return false;
    m_pendingCapturePath = path;
    m_capture->capture();  // imageCaptured → flip → save manuel
    return true;
}

void VideoWindow::onQualityChanged() { if (m_camera) { stopCamera(); startCamera(); } }

void VideoWindow::applyQuality()
{
    if (!m_camera) return;
    const QString sel = m_qualCombo->currentText();

    QSize targetRes;
    if (sel.contains("4K"))        targetRes = QSize(3840, 2160);
    else if (sel.contains("1080")) targetRes = QSize(1920, 1080);
    else                           targetRes = QSize(1280, 720);

    const bool wantMjpeg = sel.contains("MJPEG");

    QCameraFormat best;
    for (const auto &fmt : m_camera->cameraDevice().videoFormats()) {
        if (fmt.resolution() != targetRes) continue;
        const bool isMjpeg = (fmt.pixelFormat() == QVideoFrameFormat::Format_Jpeg);
        if (isMjpeg != wantMjpeg) continue;
        if (best.isNull() || fmt.maxFrameRate() > best.maxFrameRate())
            best = fmt;
    }
    if (!best.isNull())
        m_camera->setCameraFormat(best);
}

void VideoWindow::onDeviceConnected()
{
    auto &dm = DeviceManager::instance();
    m_statusLbl->setText(
        QString("● %1  FW %2").arg(dm.deviceName(), dm.deviceVersion()));
    m_statusLbl->setStyleSheet("color:rgba(166,227,161,220);font-size:11px;font-weight:bold;");
    m_powerBtn->setEnabled(true);
    m_pauseBtn->setEnabled(true);
    m_photoBtn->setEnabled(true);
    m_sleeping = false;
    m_powerBtn->setText("⏼");
    m_powerBtn->setToolTip(tr("Sleep"));
    QTimer::singleShot(800, this, &VideoWindow::startCamera);
}

void VideoWindow::onDeviceDisconnected()
{
    m_statusLbl->setText(tr("Camera disconnected"));
    m_statusLbl->setStyleSheet("color:rgba(243,139,168,200);font-size:11px;");
    m_powerBtn->setEnabled(false);
    m_pauseBtn->setEnabled(false);
    m_photoBtn->setEnabled(false);
    stopCamera();
}
