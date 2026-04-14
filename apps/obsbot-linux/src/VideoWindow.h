/**
 * @file VideoWindow.h
 * @brief Fenetre video independante et detachable.
 */
#pragma once
#include <QWidget>
#include <dev/dev.hpp>

class QCamera;
class QMediaCaptureSession;
class QVideoSink;
class QImageCapture;
class QLabel;
class QPushButton;
class QComboBox;

class VideoWindow : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWindow(QWidget *parent = nullptr);
    ~VideoWindow();

    void startCamera();
    void stopCamera();
    void pause();
    void resume();
    bool captureToFile(const QString &path);

    void setHudVisible(bool visible);
    bool isHudVisible() const { return m_hudVisible; }

public slots:
    void onDeviceConnected();
    void onDeviceDisconnected();
    void onVirtualCamToggled(bool active);

signals:
    void pauseToggled();

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

private slots:
    void onHudTimeout();
    void onQualityChanged();

private:
    void buildUi();
    void buildHud();
    void updateHudPositions();
    void applyQuality();

    // Video
    QCamera              *m_camera     = nullptr;
    QMediaCaptureSession *m_session    = nullptr;
    QVideoSink           *m_sink       = nullptr;
    QImageCapture        *m_capture    = nullptr;
    QLabel               *m_videoLabel        = nullptr;
    QLabel               *m_virtualMsgLbl     = nullptr;
    bool                  m_paused            = false;
    bool                  m_sleeping          = false;
    bool                  m_virtualCamActive  = false;
    bool                  m_mirrorEnabled     = true;
    bool                  m_flipV             = false;
    float                 m_currentZoom       = 1.0f;
    QString               m_pendingCapturePath;

    // HUD
    QWidget     *m_hudTop    = nullptr;
    QWidget     *m_hudBottom = nullptr;
    QPushButton *m_powerBtn  = nullptr;
    QPushButton *m_pauseBtn  = nullptr;
    QPushButton *m_photoBtn  = nullptr;
    QPushButton *m_mirrorBtn = nullptr;
    QPushButton *m_flipVBtn  = nullptr;
    QPushButton *m_hudToggle = nullptr;
    QLabel      *m_zoomOverlay = nullptr;
    QComboBox   *m_qualCombo = nullptr;
    QLabel      *m_statusLbl = nullptr;
    bool         m_hudVisible = true;
    QTimer      *m_hudTimer  = nullptr;

    static QString hudBtnStyle();
};
