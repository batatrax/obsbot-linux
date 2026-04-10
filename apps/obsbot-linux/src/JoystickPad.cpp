#include "JoystickPad.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

JoystickPad::JoystickPad(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(200, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setCursor(Qt::CrossCursor);
    m_ticker = new QTimer(this);
    m_ticker->setInterval(50);
    connect(m_ticker, &QTimer::timeout, this, &JoystickPad::onTick);
}

void JoystickPad::setSpeedMode(bool e) { m_speedMode = e; if (!e) recenter(); }

void JoystickPad::recenter()
{
    m_pos = {0,0}; m_knobPos = center();
    m_ticker->stop(); update(); emit released();
}

QPointF JoystickPad::center() const { return {width()/2.0, height()/2.0}; }
float   JoystickPad::radius() const { return qMin(width(),height())/2.0f - 12; }

QPointF JoystickPad::clampToCircle(QPointF p, float r) const
{
    float len = qSqrt(p.x()*p.x()+p.y()*p.y());
    if (len > r) { p.rx() *= r/len; p.ry() *= r/len; }
    return p;
}

void JoystickPad::mousePressEvent(QMouseEvent *e)
{
    m_pressed = true;
    QPointF d = clampToCircle(e->position() - center(), radius());
    m_knobPos = center() + d;
    m_pos = {float(d.x()/radius()), float(d.y()/radius())};
    if (m_speedMode) m_ticker->start();
    update(); emit moved(m_pos.x(), -m_pos.y());
}

void JoystickPad::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_pressed) return;
    QPointF d = clampToCircle(e->position() - center(), radius());
    m_knobPos = center() + d;
    m_pos = {float(d.x()/radius()), float(d.y()/radius())};
    update(); if (!m_speedMode) emit moved(m_pos.x(), -m_pos.y());
}

void JoystickPad::mouseReleaseEvent(QMouseEvent *)
{
    m_pressed = false;
    if (m_speedMode) recenter();
    update();
}

void JoystickPad::onTick()
{
    float px = m_pos.x(), py = -m_pos.y();
    if (qAbs(px) < m_deadzone) px = 0;
    if (qAbs(py) < m_deadzone) py = 0;
    if (px != 0 || py != 0) emit moved(px, py);
}

void JoystickPad::resizeEvent(QResizeEvent *)
{ if (!m_pressed) m_knobPos = center(); }

void JoystickPad::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPointF c = center();
    float   r = radius();

    // Fond
    QRadialGradient bg(c, r);
    bg.setColorAt(0, QColor("#2a2a4a")); bg.setColorAt(1, QColor("#1a1a2e"));
    p.setPen(QPen(QColor("#45475a"), 2)); p.setBrush(bg);
    p.drawEllipse(c, r, r);

    // Croix + cercles
    p.setPen(QPen(QColor("#45475a"), 1, Qt::DashLine));
    p.drawLine(QPointF(c.x(), c.y()-r), QPointF(c.x(), c.y()+r));
    p.drawLine(QPointF(c.x()-r, c.y()), QPointF(c.x()+r, c.y()));
    for (float f : {0.33f, 0.66f}) {
        p.setPen(QPen(QColor("#313244"), 1)); p.setBrush(Qt::NoBrush);
        p.drawEllipse(c, r*f, r*f);
    }

    // Labels
    p.setPen(QColor("#585b70")); p.setFont(QFont("sans", 9));
    p.drawText(QRectF(c.x()-8, c.y()-r+2,  16, 14), Qt::AlignCenter, "H");
    p.drawText(QRectF(c.x()-8, c.y()+r-14, 16, 14), Qt::AlignCenter, "B");
    p.drawText(QRectF(c.x()-r+2,  c.y()-8, 16, 16), Qt::AlignCenter, "G");
    p.drawText(QRectF(c.x()+r-16, c.y()-8, 16, 16), Qt::AlignCenter, "D");

    // Knob
    float   kR = 22;
    QPointF kp = m_pressed ? m_knobPos : c;
    p.setPen(Qt::NoPen); p.setBrush(QColor(0,0,0,60));
    p.drawEllipse(kp+QPointF(2,3), kR, kR);
    bool active = m_pressed && (qAbs(m_pos.x()) > m_deadzone || qAbs(m_pos.y()) > m_deadzone);
    QRadialGradient kg(kp-QPointF(kR*0.3,kR*0.3), kR*1.2);
    kg.setColorAt(0, active ? QColor("#b4d0fa") : QColor("#89b4fa"));
    kg.setColorAt(1, active ? QColor("#5a8fda") : QColor("#4a74ca"));
    p.setBrush(kg); p.setPen(QPen(active ? QColor("#cba6f7") : QColor("#74c7ec"), 2));
    p.drawEllipse(kp, kR, kR);
    p.setBrush(QColor("#1e1e2e")); p.setPen(Qt::NoPen);
    p.drawEllipse(kp, 4, 4);

    if (m_speedMode) {
        p.setPen(QColor("#f9e2af")); p.setFont(QFont("sans", 8));
        p.drawText(QRectF(0, height()-18, width(), 16), Qt::AlignCenter, "MODE VITESSE");
    }
}
