/**
 * @file JoystickWidget.h
 * @brief Joystick virtuel interactif pour le controle du gimbal.
 *
 * Controles :
 *   - Glisser la souris      : controle de vitesse pan/tilt
 *   - Relacher               : deceleration progressive
 *   - Double-clic            : reset gimbal au centre
 *   - Clic droit             : menu contextuel (positions, presets, visage)
 *   - Clic simple + ZQSD     : controle clavier (AZERTY)
 *   - Clic simple + WASD     : controle clavier (QWERTY)
 *   - Clic simple + fleches  : controle clavier universel
 *
 * IMPORTANT : Le clavier n'est actif QUE quand le widget a le focus
 * (cliquer dessus). Cela evite toute interference avec les autres
 * applications (navigateur, OBS, etc.).
 *
 * Parametres configurables :
 *   - setInvertX/Y()    : inverser les axes
 *   - setDeadzone()     : zone morte centrale
 *   - setSensitivity()  : sensibilite generale
 *   - setCompact()      : mode compact (taille reduite)
 *
 * Signaux :
 *   - userTouched()     : emis quand l'utilisateur prend le controle
 *   - userReleased()    : emis quand le joystick revient au centre
 *   Ces signaux sont connectes a AiWidget::suspendAi/resumeAi()
 */
#pragma once
#include <QWidget>
#include <QPointF>
#include <QTimer>

class JoystickWidget : public QWidget
{
    Q_OBJECT
public:
    enum class Size { Normal, Compact };

    explicit JoystickWidget(QWidget *parent = nullptr);
    ~JoystickWidget();

    void setInvertX(bool v)      { m_invertX = v; }
    void setInvertY(bool v)      { m_invertY = v; }
    void setDeadzone(float dz)   { m_deadzone = dz; }
    void setSensitivity(float s) { m_sensitivity = s; }
    void setCompact(bool c);

    // Appele par la fenetre parente pour transmettre les touches
    void handleKeyPress(int key);
    void handleKeyRelease(int key);

signals:
    void speedChanged(int panSpeed, int tiltSpeed);
    void resetRequested();
    void faceTargetRequested();
    void presetRequested(int id);
    // Emis quand l utilisateur touche le joystick (pour suspendre l IA)
    void userTouched();
    // Emis quand le joystick revient au centre
    void userReleased();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void contextMenuEvent(QContextMenuEvent *) override;
    void leaveEvent(QEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void onTick();

private:
    QPointF center() const;
    float   radius() const;
    QPointF clampToCircle(QPointF pos) const;
    void    computeSmoothed();
    void    sendSpeed();
    void    stopGimbal();
    void    updateKeyStick();

    // Souris
    QPointF m_stickPos;
    QPointF m_stickNorm = {0, 0};
    bool    m_dragging  = false;

    // Clavier
    bool    m_keyUp    = false, m_keyDown  = false;
    bool    m_keyLeft  = false, m_keyRight = false;

    // Lissage
    float   m_smoothPan  = 0.0f;
    float   m_smoothTilt = 0.0f;
    float   m_alpha      = 0.13f;

    // Parametres
    float   m_deadzone    = 0.13f;
    float   m_sensitivity = 0.48f;
    bool    m_invertX     = false;
    bool    m_invertY     = false;
    bool    m_compact     = false;

    // Derniere vitesse envoyee (evite les doublons)
    int     m_lastPan  = 0;
    int     m_lastTilt = 0;

    static constexpr int MIN_SPEED = 2;
    static constexpr int MAX_PAN   = 50;
    static constexpr int MAX_TILT  = 55;

    QTimer *m_timer;
};
