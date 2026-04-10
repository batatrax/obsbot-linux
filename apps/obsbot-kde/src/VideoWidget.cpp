#include "VideoWidget.h"
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoWidget>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QVBoxLayout>
#include <QLabel>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent) { buildUi(); }
VideoWidget::~VideoWidget() { stopCamera(); }

void VideoWidget::buildUi()
{
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    m_view = new QVideoWidget;
    m_view->setStyleSheet("background:#000;");
    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    l->addWidget(m_view);

    m_placeholder = new QLabel("🎥  Branchez votre OBSBOT Tiny 2 Lite");
    m_placeholder->setAlignment(Qt::AlignCenter);
    m_placeholder->setStyleSheet("color:#45475a; font-size:16px;");
    l->addWidget(m_placeholder);
    m_view->hide();
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
    m_session->setCamera(m_camera);
    m_session->setVideoOutput(m_view);
    m_camera->start();
    m_view->show();
    m_placeholder->hide();
}

void VideoWidget::stopCamera()
{
    if (m_camera) { m_camera->stop(); delete m_session; delete m_camera; m_camera = nullptr; m_session = nullptr; }
    m_view->hide();
    m_placeholder->show();
}
