/**
 * @file VideoWindow.h
 * @brief Fenetre video independante et detachable.
 *
 * - Redimensionnable librement
 * - Always on top (optionnel)
 * - HUD overlay sur les bords (semi-transparent, masquable)
 * - Selecteur qualite (resolution + format)
 * - Boutons principaux incrustes dans la video
 */
#pragma once
#include <QWidget>
#include <dev/dev.hpp>

class QCamera;
class QMediaCaptureSession;
class QVideoWidget;
class QImageCapture;
class QLabel;
class QPushButton;
class QComboBox;
class QPropertyAnimation;

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

signals:
    void wakeupRequested();
    void shutdownRequested();
    void pauseToggled();
    void photoRequested();
    void qualityChanged(const QString &resolution, const QString &format);

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
    QCamera              *m_camera   = nullptr;
    QMediaCaptureSession *m_session  = nullptr;
    QVideoWidget         *m_view     = nullptr;
    QImageCapture        *m_capture  = nullptr;
    bool                  m_paused   = false;

    // HUD overlay
    QWidget     *m_hudTop    = nullptr;  // barre du haut (statut)
    QWidget     *m_hudBottom = nullptr;  // barre du bas (boutons + qualite)
    QPushButton *m_wakeBtn   = nullptr;
    QPushButton *m_shutBtn   = nullptr;
    QPushButton *m_pauseBtn  = nullptr;
    QPushButton *m_photoBtn  = nullptr;
    QPushButton *m_hudToggle = nullptr;
    QComboBox   *m_qualCombo = nullptr;
    QLabel      *m_statusLbl = nullptr;
    bool         m_hudVisible = true;
    QTimer      *m_hudTimer  = nullptr;

    // Style boutons HUD
    static QString hudBtnStyle();
};
