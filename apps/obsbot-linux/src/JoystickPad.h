#pragma once
#include <QWidget>
#include <QTimer>
#include <QPointF>

class JoystickPad : public QWidget
{
    Q_OBJECT
public:
    explicit JoystickPad(QWidget *parent = nullptr);
    bool speedMode() const { return m_speedMode; }
    void setSpeedMode(bool enabled);
    void recenter();

signals:
    void moved(float pan, float tilt);
    void released();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private slots:
    void onTick();

private:
    QPointF clampToCircle(QPointF p, float r) const;
    QPointF center() const;
    float   radius() const;

    QPointF m_pos{0,0};
    QPointF m_knobPos{0,0};
    bool    m_pressed   = false;
    bool    m_speedMode = false;
    float   m_deadzone  = 0.05f;
    QTimer *m_ticker    = nullptr;
};
