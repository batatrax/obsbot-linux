#pragma once
#include <QWidget>
class QCamera;
class QMediaCaptureSession;
class QVideoSink;
class QImageCapture;
class QLabel;

class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();
    void startCamera();
    void stopCamera();
    void pause();
    void resume();
    bool captureToFile(const QString &path);
private:
    void buildUi();
    QCamera              *m_camera      = nullptr;
    QMediaCaptureSession *m_session     = nullptr;
    QVideoSink           *m_sink        = nullptr;
    QImageCapture        *m_capture     = nullptr;
    QLabel               *m_videoLabel  = nullptr;
    QLabel               *m_placeholder = nullptr;
    bool                  m_paused      = false;
};
