#pragma once
#include <QWidget>
class QCamera;
class QMediaCaptureSession;
class QVideoWidget;
class QLabel;

class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();
    void startCamera();
    void stopCamera();
private:
    void buildUi();
    QCamera              *m_camera  = nullptr;
    QMediaCaptureSession *m_session = nullptr;
    QVideoWidget         *m_view    = nullptr;
    QLabel               *m_placeholder = nullptr;
};
