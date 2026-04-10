#include "GimbalWidget.h"
#include "JoystickWidget.h"
#include <obsbot/DeviceManager.h>
#include <obsbot/PresetManager.h>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QScrollArea>

GimbalWidget::GimbalWidget(QWidget *parent) : QWidget(parent) { buildUi(); }

void GimbalWidget::buildUi()
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;}");
    auto *inner = new QWidget;
    auto *root  = new QVBoxLayout(inner);
    root->setContentsMargins(12,10,12,10);
    root->setSpacing(10);

    // Options
    {
        auto *row = new QHBoxLayout;
        m_compactCb = new QCheckBox("Compact");
        m_invertXCb = new QCheckBox("Inv. X");
        m_invertYCb = new QCheckBox("Inv. Y");
        m_invertXCb->setToolTip("Inverser gauche/droite");
        m_invertYCb->setToolTip("Inverser haut/bas");
        row->addWidget(m_compactCb); row->addStretch();
        row->addWidget(m_invertXCb); row->addWidget(m_invertYCb);
        root->addLayout(row);
    }

    // Joystick
    auto *jg = new QGroupBox("Joystick Gimbal");
    auto *jl = new QVBoxLayout(jg);
    jl->setSpacing(6);

    auto *kbInfo = new QLabel("Cliquer pour activer clavier  |  ZQSD / WASD / Fleches  |  Double-clic: reset");
    kbInfo->setAlignment(Qt::AlignCenter);
    kbInfo->setStyleSheet("color:#45475a;font-size:10px;font-style:italic;");
    jl->addWidget(kbInfo);

    m_joystick = new JoystickWidget;
    jl->addWidget(m_joystick, 0, Qt::AlignCenter);

    m_speedLbl = new QLabel("Pan: 0  Tilt: 0");
    m_speedLbl->setAlignment(Qt::AlignCenter);
    m_speedLbl->setStyleSheet("color:#585b70;font-size:10px;");
    jl->addWidget(m_speedLbl);

    auto *qr = new QHBoxLayout;
    m_resetBtn = new QPushButton("Reset");
    m_faceBtn  = new QPushButton("Cibler visage");
    m_faceBtn->setStyleSheet(
        "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;"
        "border-radius:5px;font-weight:bold;padding:3px 8px;}"
        "QPushButton:hover{background:#b4d0fa;}"
        "QPushButton:disabled{background:#313244;color:#585b70;}");
    m_resetBtn->setMaximumHeight(28);
    m_faceBtn->setMaximumHeight(28);
    qr->addWidget(m_resetBtn); qr->addWidget(m_faceBtn);
    jl->addLayout(qr);
    root->addWidget(jg);

    // Zoom
    auto *zg = new QGroupBox("Zoom");
    auto *zl = new QHBoxLayout(zg);
    zl->addWidget(new QLabel("1x"));
    m_zoom = new QSlider(Qt::Horizontal);
    m_zoom->setRange(100,400); m_zoom->setValue(100);
    m_zoomLbl = new QLabel("1.0x"); m_zoomLbl->setFixedWidth(36);
    zl->addWidget(m_zoom,1); zl->addWidget(new QLabel("4x")); zl->addWidget(m_zoomLbl);
    root->addWidget(zg);

    // FOV
    auto *fg = new QGroupBox("Champ de vision (FOV)");
    auto *fl = new QFormLayout(fg);
    m_fovCombo = new QComboBox;
    m_fovCombo->addItem("Large (86 deg)",  int(Device::FovType86));
    m_fovCombo->addItem("Moyen (78 deg)",  int(Device::FovType78));
    m_fovCombo->addItem("Etroit (65 deg)", int(Device::FovType65));
    fl->addRow("FOV :", m_fovCombo);
    root->addWidget(fg);
    root->addStretch();

    scroll->setWidget(inner);
    auto *ml = new QVBoxLayout(this);
    ml->setContentsMargins(0,0,0,0);
    ml->addWidget(scroll);

    connect(m_joystick, &JoystickWidget::speedChanged,       this, &GimbalWidget::onSpeedChanged);
    connect(m_joystick, &JoystickWidget::resetRequested,     this, &GimbalWidget::onReset);
    connect(m_joystick, &JoystickWidget::faceTargetRequested,this, &GimbalWidget::onFaceTarget);
    connect(m_joystick, &JoystickWidget::presetRequested,    this, &GimbalWidget::onPresetRequested);
    connect(m_joystick, &JoystickWidget::userTouched,        this, &GimbalWidget::userTouchedJoystick);
    connect(m_joystick, &JoystickWidget::userReleased,       this, &GimbalWidget::userReleasedJoystick);
    connect(m_zoom,     &QSlider::valueChanged, this, &GimbalWidget::onZoomChanged);
    connect(m_resetBtn, &QPushButton::clicked,  this, &GimbalWidget::onReset);
    connect(m_faceBtn,  &QPushButton::clicked,  this, &GimbalWidget::onFaceTarget);
    connect(m_invertXCb,&QCheckBox::toggled, m_joystick, &JoystickWidget::setInvertX);
    connect(m_invertYCb,&QCheckBox::toggled, m_joystick, &JoystickWidget::setInvertY);
    connect(m_compactCb,&QCheckBox::toggled, this, &GimbalWidget::onCompactToggled);
    connect(m_fovCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GimbalWidget::onFovChanged);
}

void GimbalWidget::onSpeedChanged(int p,int t)
{ m_speedLbl->setText(QString("Pan: %1  Tilt: %2").arg(p).arg(t)); }

void GimbalWidget::onZoomChanged(int v)
{
    m_zoomLbl->setText(QString("%1x").arg(v/100.0,0,'f',1));
    DeviceManager::instance().setZoom(v/100.0);
}

void GimbalWidget::onReset()   { DeviceManager::instance().gimbalReset(); }
void GimbalWidget::onFaceTarget()
{
    DeviceManager::instance().setFaceFocus(true);
    DeviceManager::instance().setAiMode(Device::AiWorkModeHuman,Device::AiSubModeCloseUp);
}
void GimbalWidget::onFovChanged(int)
{ DeviceManager::instance().setFov(
    static_cast<Device::FovType>(m_fovCombo->currentData().toInt())); }
void GimbalWidget::onCompactToggled(bool c) { m_joystick->setCompact(c); }
void GimbalWidget::onPresetRequested(int id){ PresetManager::instance().applyToDevice(id); }
void GimbalWidget::setEnabled(bool e) { QWidget::setEnabled(e); }

void GimbalWidget::onStatusUpdated(Device::CameraStatus status)
{
    int zv = 100 + int(status.tiny.zoom_ratio * 3);
    zv = qBound(100,zv,400);
    if (!m_zoom->isSliderDown()) {
        m_zoom->blockSignals(true);
        m_zoom->setValue(zv);
        m_zoomLbl->setText(QString("%1x").arg(zv/100.0,0,'f',1));
        m_zoom->blockSignals(false);
    }
}
