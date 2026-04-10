#include "GimbalWidget.h"
#include <obsbot/DeviceManager.h>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

GimbalWidget::GimbalWidget(QWidget *parent) : QWidget(parent) { buildUi(); }

void GimbalWidget::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16);
    root->setSpacing(16);

    auto makeRow = [](const QString &label, QSlider *&slider, QLabel *&lbl,
                      int mn, int mx, const QString &unit) {
        auto *row = new QHBoxLayout;
        auto *l = new QLabel(label);
        l->setFixedWidth(48);
        l->setStyleSheet("color:#a6adc8;");
        slider = new QSlider(Qt::Horizontal);
        slider->setRange(mn, mx);
        slider->setValue(0);
        lbl = new QLabel("0" + unit);
        lbl->setFixedWidth(52);
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        row->addWidget(l);
        row->addWidget(slider, 1);
        row->addWidget(lbl);
        return row;
    };

    auto *gb = new QGroupBox("Contrôle Gimbal");
    auto *gl = new QVBoxLayout(gb);
    gl->addLayout(makeRow("Pan",  m_pan,  m_panLbl,  -130, 130, "°"));
    gl->addLayout(makeRow("Tilt", m_tilt, m_tiltLbl, -90,  90,  "°"));
    root->addWidget(gb);

    auto *zb = new QGroupBox("Zoom");
    auto *zl = new QVBoxLayout(zb);
    zl->addLayout(makeRow("×", m_zoom, m_zoomLbl, 100, 400, "×"));
    root->addWidget(zb);

    auto *btnRow = new QHBoxLayout;
    m_resetBtn = new QPushButton("⟳ Centrer");
    m_speedBtn = new QPushButton("Vitesse continue");
    m_speedBtn->setCheckable(true);
    btnRow->addWidget(m_resetBtn);
    btnRow->addWidget(m_speedBtn);
    root->addLayout(btnRow);
    root->addStretch();

    connect(m_pan,  &QSlider::valueChanged, this, &GimbalWidget::onPanChanged);
    connect(m_tilt, &QSlider::valueChanged, this, &GimbalWidget::onTiltChanged);
    connect(m_zoom, &QSlider::valueChanged, this, &GimbalWidget::onZoomChanged);
    connect(m_resetBtn, &QPushButton::clicked,  this, &GimbalWidget::onReset);
    connect(m_speedBtn, &QPushButton::toggled,  this, &GimbalWidget::onSpeedMode);
}

void GimbalWidget::onPanChanged(int v) {
    m_panLbl->setText(QString("%1°").arg(v));
    if (!m_continuousMode)
        DeviceManager::instance().gimbalSetAngle(v, m_tilt->value());
}
void GimbalWidget::onTiltChanged(int v) {
    m_tiltLbl->setText(QString("%1°").arg(v));
    if (!m_continuousMode)
        DeviceManager::instance().gimbalSetAngle(m_pan->value(), v);
}
void GimbalWidget::onZoomChanged(int v) {
    m_zoomLbl->setText(QString("%1×").arg(v/100.0, 0, 'f', 1));
    DeviceManager::instance().setZoom(v / 100.0);
}
void GimbalWidget::onReset() {
    m_pan->setValue(0); m_tilt->setValue(0); m_zoom->setValue(100);
    DeviceManager::instance().gimbalReset();
}
void GimbalWidget::onSpeedMode(bool c) {
    m_continuousMode = c;
    m_speedBtn->setText(c ? "✓ Vitesse continue" : "Vitesse continue");
}
void GimbalWidget::setEnabled(bool e) { QWidget::setEnabled(e); }
