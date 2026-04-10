/**
 * @file AiWidget.h
 * @brief Panneau de controle de l'IA de suivi OBSBOT.
 *
 * Fonctionnalites :
 *   - Mode de suivi : Humain (corps/buste/gros plan), Groupe, Main
 *   - Mise au point automatique sur le visage
 *   - Verrouillage du zoom (empeche le dezoom automatique de l'IA)
 *   - Gesture Control 2.0 : selection cible, zoom, photo par geste
 *
 * Gestion du conflit joystick/IA :
 *   - suspendAi() : suspend le suivi pendant l'utilisation du joystick
 *   - resumeAi()  : reprend le suivi 500ms apres le relachement
 *   Cela evite que l'IA et le joystick se battent pour le controle.
 */
#pragma once
#include <QWidget>
#include <dev/dev.hpp>
class QComboBox; class QPushButton; class QLabel; class QCheckBox;

class AiWidget : public QWidget {
    Q_OBJECT
public:
    explicit AiWidget(QWidget *parent = nullptr);
public slots:
    void setEnabled(bool e);
    void onStatusUpdated(Device::CameraStatus status);
    void suspendAi();
    void resumeAi();
signals:
    void aiActivated(Device::AiWorkModeType mode, Device::AiSubModeType sub);
    void aiDeactivated();
private slots:
    void onActivateToggle();
    void onGestureChanged(int gesture, bool enabled);
private:
    void buildUi();
    void applySettings();
    QComboBox   *m_modeCombo     = nullptr;
    QPushButton *m_activateBtn   = nullptr;
    QLabel      *m_statusLbl     = nullptr;
    QCheckBox   *m_faceFocusCb   = nullptr;
    QCheckBox   *m_zoomLockCb    = nullptr;
    QCheckBox   *m_gestureChk[3] = {};
    bool m_aiActive    = false;
    bool m_aiSuspended = false;
    Device::AiWorkModeType m_savedMode = Device::AiWorkModeNone;
    Device::AiSubModeType  m_savedSub  = Device::AiSubModeNormal;
};
