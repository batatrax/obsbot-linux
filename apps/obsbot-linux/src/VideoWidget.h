/**
 * @file VideoWidget.h
 * @brief Widget de previsualisation video en direct.
 *
 * Utilise Qt Multimedia (backend GStreamer/V4L2).
 * Detecte automatiquement la camera OBSBOT parmi les peripheriques.
 *
 * Fonctions :
 *   - startCamera()     : demarre le flux video
 *   - stopCamera()      : arrete le flux
 *   - pause()           : gele l'image
 *   - resume()          : reprend le flux
 *   - captureToFile()   : capture une image JPEG
 */
#pragma once
#include <QWidget>
class QCamera;
class QMediaCaptureSession;
class QVideoWidget;
class QLabel;
class QImageCapture;

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
    QCamera              *m_camera   = nullptr;
    QMediaCaptureSession *m_session  = nullptr;
    QVideoWidget         *m_view     = nullptr;
    QImageCapture        *m_capture  = nullptr;
    QLabel               *m_placeholder = nullptr;
    bool                  m_paused   = false;
};
