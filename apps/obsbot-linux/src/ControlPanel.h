/**
 * @file ControlPanel.h
 * @brief Panneau de controle unique — tout sur une vue scrollable.
 *
 * Sections (de haut en bas) :
 *   1. Joystick gimbal + zoom + FOV
 *   2. IA & Suivi (mode, face focus, zoom lock, gestes)
 *   3. Image (HDR, balance blancs, focus)
 *   4. Presets (3 boutons compacts sur une ligne)
 *   5. Firmware (menu deroulant discret)
 *
 * Pas d'onglets. Tout accessible en scrollant.
 * Le QScrollArea est configure pour que les fleches du clavier
 * ne soient PAS interceptees par le scroll (elles vont au joystick).
 */
#pragma once
#include <QWidget>
#include <QScrollArea>
#include <dev/dev.hpp>

class JoystickWidget;
class QSlider; class QLabel; class QPushButton;
class QComboBox; class QCheckBox; class QGroupBox;
class QSpinBox;

class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ControlPanel(QWidget *parent = nullptr);

public slots:
    void setEnabled(bool e);
    void onStatusUpdated(Device::CameraStatus status);

signals:
    void joystickTouched();
    void joystickReleased();

private slots:
    void onZoomChanged(int v);
    void onFovChanged(int i);
    void onAiToggle();
    void onFaceTarget();
    void onHdrChanged(int i);
    void onWbChanged(int i);
    void onWbTempChanged(int v);
    void onFocusChanged(int v);
    void onPresetLoad(int id);
    void onPresetSave(int id);
    void onFirmwareToggle();
    void onFirmwareBrowse();
    void onFirmwareUpgrade();
    void onGestureChanged(int g, bool e);
    void onReset();

private:
    void buildUi();
    QWidget* buildGimbalSection();
    QWidget* buildAiSection();
    QWidget* buildImageSection();
    QWidget* buildPresetSection();
    QWidget* buildFirmwareSection();

    // Gimbal
    JoystickWidget *m_joystick   = nullptr;
    QSlider        *m_zoom       = nullptr;
    QLabel         *m_zoomLbl    = nullptr;
    QLabel         *m_speedLbl   = nullptr;
    QComboBox      *m_fovCombo   = nullptr;
    QCheckBox      *m_invertXCb  = nullptr;
    QCheckBox      *m_invertYCb  = nullptr;

    // IA
    QComboBox      *m_aiModeCombo = nullptr;
    QPushButton    *m_aiBtn       = nullptr;
    QLabel         *m_aiStatusLbl = nullptr;
    QCheckBox      *m_faceFocusCb = nullptr;
    QCheckBox      *m_zoomLockCb  = nullptr;
    QCheckBox      *m_gesture[3]  = {};
    bool            m_aiActive       = false;
    bool            m_aiSuspended    = false;
    Device::AiWorkModeType m_savedMode    = Device::AiWorkModeNone;
    Device::AiSubModeType  m_savedSub     = Device::AiSubModeNormal;
    float           m_homeZoom       = 1.0f;
    float           m_homeYaw        = 0.0f;
    float           m_homePitch      = 0.0f;
    int             m_noTargetFrames = 0;

    // Image
    QComboBox      *m_hdrCombo      = nullptr;
    QComboBox      *m_wbCombo       = nullptr;
    QSlider        *m_wbTempSlider  = nullptr;
    QLabel         *m_wbTempLbl     = nullptr;
    QSlider        *m_focusSlider   = nullptr;
    QLabel         *m_focusLbl      = nullptr;

    // Presets
    QPushButton    *m_presetLoad[3] = {};
    QPushButton    *m_presetSave[3] = {};
    QLabel         *m_presetLbl[3] = {};

    // Firmware
    QWidget        *m_firmwarePanel = nullptr;
    QLabel         *m_fwVersionLbl  = nullptr;
    QPushButton    *m_fwToggleBtn   = nullptr;
    QLabel         *m_fwFileLbl     = nullptr;
    QPushButton    *m_fwUpgradeBtn  = nullptr;
    QString         m_fwFilePath;
    bool            m_firmwareExpanded = false;
};
