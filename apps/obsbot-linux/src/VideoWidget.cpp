#include "VideoWidget.h"
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QImageCapture>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QVBoxLayout>
#include <QLabel>

// Downscale progressif : deux fois plus net qu'un seul passage SmoothTransformation
static QImage sharpScale(QImage img, const QSize &target)
{
    while (img.width() > target.width() * 2 || img.height() > target.height() * 2)
        img = img.scaled(img.width() / 2, img.height() / 2,
                         Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return img.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent) { buildUi(); }
VideoWidget::~VideoWidget() { stopCamera(); }

void VideoWidget::buildUi()
{
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0,0,0,0);

    m_videoLabel = new QLabel;
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet("background:#000;");
    m_videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    l->addWidget(m_videoLabel);

    m_sink = new QVideoSink(this);
    connect(m_sink, &QVideoSink::videoFrameChanged,
            this, [this](const QVideoFrame &frame) {
        if (!frame.isValid()) return;
        QImage img = frame.toImage().flipped(Qt::Horizontal);
        if (img.isNull()) return;
        m_videoLabel->setPixmap(QPixmap::fromImage(
            sharpScale(std::move(img), m_videoLabel->size())));
    });

    m_placeholder = new QLabel("Branchez votre OBSBOT Tiny 2 Lite");
    m_placeholder->setAlignment(Qt::AlignCenter);
    m_placeholder->setStyleSheet("color:#45475a;font-size:16px;background:#0a0a14;");
    l->addWidget(m_placeholder);
    m_videoLabel->hide();
}

void VideoWidget::startCamera()
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
    m_session->setVideoOutput(m_sink);
    m_session->setImageCapture(m_capture);
    m_camera->start();

    if (m_camera->isFocusModeSupported(QCamera::FocusModeAuto))
        m_camera->setFocusMode(QCamera::FocusModeAuto);

    m_videoLabel->show();
    m_placeholder->hide();
    m_paused = false;
}

void VideoWidget::stopCamera()
{
    if (m_camera) {
        m_camera->stop();
        delete m_capture; m_capture = nullptr;
        delete m_session; m_session = nullptr;
        delete m_camera;  m_camera  = nullptr;
    }
    m_videoLabel->clear();
    m_videoLabel->hide();
    m_placeholder->show();
}

void VideoWidget::pause()
{
    if (m_camera && !m_paused) { m_camera->stop(); m_paused = true; }
}

void VideoWidget::resume()
{
    if (m_camera && m_paused) { m_camera->start(); m_paused = false; }
}

bool VideoWidget::captureToFile(const QString &path)
{
    if (!m_capture || !m_camera) return false;
    m_capture->captureToFile(path);
    return true;
}
