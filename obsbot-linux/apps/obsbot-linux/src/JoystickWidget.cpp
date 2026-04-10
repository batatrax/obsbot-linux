#include "JoystickWidget.h"
#include <obsbot/DeviceManager.h>
#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QRadialGradient>
#include <QtMath>

JoystickWidget::JoystickWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    m_stickPos = center();

    m_timer = new QTimer(this);
    m_timer->setInterval(40); // 25 Hz
    connect(m_timer, &QTimer::timeout, this, &JoystickWidget::onTick);
}

JoystickWidget::~JoystickWidget() {}

QSize JoystickWidget::sizeHint()        const { return m_compact ? QSize(120,120) : QSize(220,220); }
QSize JoystickWidget::minimumSizeHint() const { return m_compact ? QSize(90,90)   : QSize(140,140); }

void JoystickWidget::setCompact(bool c)
{
    m_compact = c;
    updateGeometry();
    update();
}

QPointF JoystickWidget::center() const { return {width()/2.0, height()/2.0}; }
float   JoystickWidget::radius() const { return qMin(width(),height())/2.0f - 8.0f; }

QPointF JoystickWidget::clampToCircle(QPointF pos) const
{
    QPointF c = center(), d = pos - c;
    float r = radius(), dist = qSqrt(d.x()*d.x()+d.y()*d.y());
    if (dist > r) d = d/dist*r;
    return c + d;
}

void JoystickWidget::computeSmoothed()
{
    float nx = float(m_stickNorm.x()), ny = float(m_stickNorm.y());
    float norm = qSqrt(nx*nx+ny*ny);

    if (norm < m_deadzone) { nx = 0; ny = 0; }
    else {
        float t = (norm-m_deadzone)/(1.0f-m_deadzone);
        float curved = qPow(t, 2.5f) * m_sensitivity;
        nx = (nx/norm)*curved;
        ny = (ny/norm)*curved;
    }

    if (m_invertX) nx = -nx;
    if (m_invertY) ny = -ny;

    m_smoothPan  = m_smoothPan  * (1.0f-m_alpha) + nx * m_alpha;
    m_smoothTilt = m_smoothTilt * (1.0f-m_alpha) + ny * m_alpha;

    if (qAbs(m_smoothPan)  < 0.008f) m_smoothPan  = 0.0f;
    if (qAbs(m_smoothTilt) < 0.008f) m_smoothTilt = 0.0f;
}

void JoystickWidget::sendSpeed()
{
    int pan  = int(m_smoothPan  * MAX_PAN);
    int tilt = int(-m_smoothTilt * MAX_TILT);
    if (qAbs(pan)  < MIN_SPEED) pan  = 0;
    if (qAbs(tilt) < MIN_SPEED) tilt = 0;
    if (pan == m_lastPan && tilt == m_lastTilt) return;
    m_lastPan = pan; m_lastTilt = tilt;
    DeviceManager::instance().gimbalSetSpeed(pan, tilt);
    emit speedChanged(pan, tilt);
}

void JoystickWidget::stopGimbal()
{
    m_stickPos = center(); m_stickNorm = {0,0};
    m_smoothPan = m_smoothTilt = 0.0f;
    m_lastPan = m_lastTilt = 0;
    m_timer->stop();
    DeviceManager::instance().gimbalSetSpeed(0,0);
    emit speedChanged(0,0);
    emit userReleased();
    update();
}

void JoystickWidget::updateKeyStick()
{
    float x = 0, y = 0;
    if (m_keyLeft)  x -= 0.75f;
    if (m_keyRight) x += 0.75f;
    if (m_keyUp)    y -= 0.75f;
    if (m_keyDown)  y += 0.75f;
    float len = qSqrt(x*x+y*y);
    if (len > 1.0f) { x /= len; y /= len; }
    QPointF c = center(); float r = radius();
    m_stickPos  = c + QPointF(x*r*0.75f, y*r*0.75f);
    m_stickNorm = {x, y};
    update();
}

void JoystickWidget::handleKeyPress(int key)
{
    bool changed = false;
    switch(key) {
        case Qt::Key_Up:    case Qt::Key_Z: case Qt::Key_W: m_keyUp    = true; changed=true; break;
        case Qt::Key_Down:  case Qt::Key_S:                  m_keyDown  = true; changed=true; break;
        case Qt::Key_Left:  case Qt::Key_Q: case Qt::Key_A: m_keyLeft  = true; changed=true; break;
        case Qt::Key_Right: case Qt::Key_D:                  m_keyRight = true; changed=true; break;
        default: break;
    }
    if (changed) {
        emit userTouched();
        if (!m_timer->isActive()) m_timer->start();
        updateKeyStick();
    }
}

void JoystickWidget::handleKeyRelease(int key)
{
    switch(key) {
        case Qt::Key_Up:    case Qt::Key_Z: case Qt::Key_W: m_keyUp    = false; break;
        case Qt::Key_Down:  case Qt::Key_S:                  m_keyDown  = false; break;
        case Qt::Key_Left:  case Qt::Key_Q: case Qt::Key_A: m_keyLeft  = false; break;
        case Qt::Key_Right: case Qt::Key_D:                  m_keyRight = false; break;
        default: break;
    }
    if (!m_keyUp && !m_keyDown && !m_keyLeft && !m_keyRight && !m_dragging)
        updateKeyStick();
}

void JoystickWidget::onTick()
{
    bool anyKey = m_keyLeft||m_keyRight||m_keyUp||m_keyDown;
    if (anyKey) updateKeyStick();
    else if (!m_dragging) {
        // Deceleration progressive vers zero
        m_stickNorm = {m_smoothPan*0.4f, -m_smoothTilt*0.4f};
        m_stickPos  = center() + QPointF(float(m_stickNorm.x())*radius(),
                                         float(m_stickNorm.y())*radius());
        update();
    } else {
        QPointF c=center(); float r=radius();
        QPointF d=m_stickPos-c;
        m_stickNorm = {d.x()/r, d.y()/r};
    }
    computeSmoothed();
    sendSpeed();
    // Arreter si tout est a zero
    if (!m_dragging && !anyKey &&
        m_smoothPan==0.0f && m_smoothTilt==0.0f)
        stopGimbal();
}

void JoystickWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button()==Qt::LeftButton) {
        m_dragging=true;
        m_stickPos=clampToCircle(e->position());
        setCursor(Qt::ClosedHandCursor);
        if (!m_timer->isActive()) m_timer->start();
        emit userTouched();
        update();
    }
}

void JoystickWidget::mouseMoveEvent(QMouseEvent *e)
{ if (m_dragging) { m_stickPos=clampToCircle(e->position()); update(); } }

void JoystickWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button()==Qt::LeftButton) {
        m_dragging=false;
        setCursor(Qt::OpenHandCursor);
        // Laisser le timer gerer la deceleration
    }
}

void JoystickWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button()==Qt::LeftButton) {
        stopGimbal();
        DeviceManager::instance().gimbalReset();
        emit resetRequested();
    }
}

void JoystickWidget::leaveEvent(QEvent *)
{ if (m_dragging) { m_dragging=false; setCursor(Qt::OpenHandCursor); } }

void JoystickWidget::focusOutEvent(QFocusEvent *)
{ m_keyUp=m_keyDown=m_keyLeft=m_keyRight=false; stopGimbal(); }

void JoystickWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QString st="QMenu{background:#313244;color:#cdd6f4;border:1px solid #45475a;"
        "border-radius:6px;padding:4px;}"
        "QMenu::item{padding:6px 20px;border-radius:4px;}"
        "QMenu::item:selected{background:#89b4fa;color:#1e1e2e;}"
        "QMenu::separator{height:1px;background:#45475a;margin:4px 0;}";
    QMenu menu(this); menu.setStyleSheet(st);
    menu.addAction("Recentrer", this, [this]{
        stopGimbal(); DeviceManager::instance().gimbalReset(); emit resetRequested();
    });
    menu.addAction("Cibler visage", this, [this]{ emit faceTargetRequested(); });
    menu.addSeparator();
    auto *pm=menu.addMenu("Preset..."); pm->setStyleSheet(st);
    for (int i=0;i<3;i++)
        pm->addAction(QString("Preset %1").arg(i+1),this,[this,i]{emit presetRequested(i);});
    menu.addSeparator();
    auto *am=menu.addMenu("Position..."); am->setStyleSheet(st);
    struct P{const char*l;float p,t;};
    P pos[]=
        {{"Face",0,0},{"Haut",0,30},{"Bas",0,-30},
         {"Gauche",-45,0},{"Droite",45,0}};
    for (auto &pp:pos)
        am->addAction(pp.l,this,[this,pp]{
            DeviceManager::instance().gimbalSetAngle(pp.p,pp.t);});
    menu.exec(e->globalPos());
}

void JoystickWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    QPointF c=center(); float r=radius(), sr=r*0.24f;

    QRadialGradient bg(c,r);
    bg.setColorAt(0,QColor("#252538")); bg.setColorAt(1,QColor("#181825"));
    p.setPen(QPen(QColor("#45475a"),1.5f)); p.setBrush(bg);
    p.drawEllipse(c,r,r);

    p.setPen(QPen(QColor("#3a3a55"),1,Qt::DashLine));
    p.drawLine(QPointF(c.x()-r,c.y()),QPointF(c.x()+r,c.y()));
    p.drawLine(QPointF(c.x(),c.y()-r),QPointF(c.x(),c.y()+r));
    p.setPen(QPen(QColor("#2e2e45"),1)); p.setBrush(Qt::NoBrush);
    p.drawEllipse(c,r*0.5f,r*0.5f);
    p.drawEllipse(c,r*0.25f,r*0.25f);

    // Fleches directionnelles
    if (!m_compact) {
        p.setPen(QColor("#4a4a70")); p.setFont(QFont("sans-serif",7));
        p.drawText(QRectF(c.x()-6,c.y()-r-12,12,12),Qt::AlignCenter,"N");
        p.drawText(QRectF(c.x()-6,c.y()+r+2,12,12),Qt::AlignCenter,"S");
        p.drawText(QRectF(c.x()-r-14,c.y()-6,14,12),Qt::AlignCenter,"W");
        p.drawText(QRectF(c.x()+r+2,c.y()-6,14,12),Qt::AlignCenter,"E");
    }

    bool active = m_dragging||m_keyLeft||m_keyRight||m_keyUp||m_keyDown
                  || qAbs(m_smoothPan)>0.01f || qAbs(m_smoothTilt)>0.01f;
    QPointF sp = active ? m_stickPos : c;

    if (active) {
        float norm=qSqrt(m_smoothPan*m_smoothPan+m_smoothTilt*m_smoothTilt);
        p.setPen(QPen(QColor("#89b4fa"),1.5f));
        p.drawLine(c,sp);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0x89,0xb4,0xfa,int(norm*45)));
        p.drawEllipse(c,norm*r,norm*r);
    }

    QRadialGradient sg(sp-QPointF(sr*0.3f,sr*0.3f),sr*1.5f);
    if (active) { sg.setColorAt(0,QColor("#b4d0fa")); sg.setColorAt(1,QColor("#4a80c0")); }
    else        { sg.setColorAt(0,QColor("#5a5a8a")); sg.setColorAt(1,QColor("#3a3a5a")); }
    p.setPen(QPen(active?QColor("#cdd6f4"):QColor("#585b70"),1.5f));
    p.setBrush(sg); p.drawEllipse(sp,sr,sr);

    float cs=sr*0.35f;
    p.setPen(QPen(QColor(255,255,255,90),1.5f));
    p.drawLine(QPointF(sp.x()-cs,sp.y()),QPointF(sp.x()+cs,sp.y()));
    p.drawLine(QPointF(sp.x(),sp.y()-cs),QPointF(sp.x(),sp.y()+cs));

    if (!active && !m_compact) {
        p.setPen(QColor("#3a3a55")); p.setFont(QFont("sans-serif",7));
        p.drawText(rect().adjusted(0,0,0,-2),
            Qt::AlignBottom|Qt::AlignHCenter,
            "Glisser  |  ZQSD/fleches  |  Clic-droit");
    }

    // Indicateur de focus clavier — bordure coloree quand le widget a le focus
    if (hasFocus()) {
        p.setPen(QPen(QColor("#89b4fa"), 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(c, r + 4, r + 4);
        if (!m_compact) {
            p.setPen(QColor("#89b4fa"));
            p.setFont(QFont("sans-serif", 8));
            p.drawText(rect().adjusted(0, 4, 0, 0),
                Qt::AlignTop | Qt::AlignHCenter, "Clavier actif");
        }
    }
}
