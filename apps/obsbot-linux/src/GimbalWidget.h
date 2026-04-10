/**
 * @file GimbalWidget.h
 * @brief Panneau de controle du gimbal (pan/tilt/zoom/FOV).
 *
 * Contient :
 *   - JoystickWidget : controle interactif pan/tilt
 *   - Slider zoom 1x-4x (synchronise avec le zoom reel)
 *   - Selecteur FOV (86/78/65 degres)
 *   - Options : mode compact, inversion X/Y
 *   - Boutons Reset et Cibler visage
 *
 * Signaux emis vers MainWindow pour suspendre/reprendre l'IA
 * pendant l'utilisation du joystick.
 *
 * Le controle clavier (ZQSD/WASD/fleches) est local au JoystickWidget :
 * il ne fonctionne que quand le joystick a le focus (clic dessus).
 */
#pragma once
#include <QWidget>
#include <dev/dev.hpp>
class QSlider; class QLabel; class QPushButton;
class JoystickWidget; class QComboBox; class QCheckBox;

class GimbalWidget : public QWidget {
    Q_OBJECT
public:
    explicit GimbalWidget(QWidget *parent = nullptr);
public slots:
    void setEnabled(bool e);
    void onStatusUpdated(Device::CameraStatus status);
signals:
    void userTouchedJoystick();
    void userReleasedJoystick();
private slots:
    void onSpeedChanged(int pan, int tilt);
    void onZoomChanged(int v);
    void onReset();
    void onFaceTarget();
    void onFovChanged(int i);
    void onCompactToggled(bool c);
    void onPresetRequested(int id);
private:
    void buildUi();
    JoystickWidget *m_joystick  = nullptr;
    QSlider        *m_zoom      = nullptr;
    QLabel         *m_zoomLbl   = nullptr;
    QLabel         *m_speedLbl  = nullptr;
    QPushButton    *m_resetBtn  = nullptr;
    QPushButton    *m_faceBtn   = nullptr;
    QComboBox      *m_fovCombo  = nullptr;
    QCheckBox      *m_invertXCb = nullptr;
    QCheckBox      *m_invertYCb = nullptr;
    QCheckBox      *m_compactCb = nullptr;
};
