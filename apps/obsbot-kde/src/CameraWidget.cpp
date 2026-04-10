#include "CameraWidget.h"
#include <obsbot/DeviceManager.h>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>

CameraWidget::CameraWidget(QWidget *parent) : QWidget(parent) { buildUi(); }

void CameraWidget::buildUi()
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;}");
    auto *inner = new QWidget;
    auto *root = new QVBoxLayout(inner);
    root->setContentsMargins(16,16,16,16);
    root->setSpacing(16);

    // FOV + HDR
    auto *gb1 = new QGroupBox("Image");
    auto *fl1 = new QFormLayout(gb1);
    m_fovCombo = new QComboBox;
    m_fovCombo->addItem("Large (86°)",   int(Device::FovType86));
    m_fovCombo->addItem("Moyen (78°)",   int(Device::FovType78));
    m_fovCombo->addItem("Étroit (65°)",  int(Device::FovType65));
    fl1->addRow("Champ de vision :", m_fovCombo);
    m_hdrCombo = new QComboBox;
    m_hdrCombo->addItem("HDR désactivé", int(Device::DevWdrModeNone));
    m_hdrCombo->addItem("HDR 2-en-1",    int(Device::DevWdrModeDol2TO1));
    fl1->addRow("HDR :", m_hdrCombo);
    root->addWidget(gb1);

    // Balance des blancs
    auto *gb2 = new QGroupBox("Balance des blancs");
    auto *fl2 = new QFormLayout(gb2);
    m_wbCombo = new QComboBox;
    m_wbCombo->addItem("Automatique",      int(Device::DevWhiteBalanceAuto));
    m_wbCombo->addItem("Manuel",           int(Device::DevWhiteBalanceManual));
    m_wbCombo->addItem("Lumière du jour",  int(Device::DevWhiteBalanceDaylight));
    m_wbCombo->addItem("Nuageux",          int(Device::DevWhiteBalanceCloudy));
    m_wbCombo->addItem("Tungstène",        int(Device::DevWhiteBalanceTungsten));
    m_wbCombo->addItem("Fluorescent",      int(Device::DevWhiteBalanceFluorescent));
    fl2->addRow("Mode :", m_wbCombo);
    {
        auto *row = new QHBoxLayout;
        m_wbTempSlider = new QSlider(Qt::Horizontal);
        m_wbTempSlider->setRange(2000, 10000);
        m_wbTempSlider->setValue(5000);
        m_wbTempSlider->setEnabled(false);
        m_wbTempLbl = new QLabel("5000 K");
        m_wbTempLbl->setFixedWidth(60);
        row->addWidget(m_wbTempSlider);
        row->addWidget(m_wbTempLbl);
        fl2->addRow("Température :", new QWidget);
        gb2->layout()->addItem(row);
    }
    root->addWidget(gb2);

    // Focus
    auto *gb3 = new QGroupBox("Mise au point");
    auto *fl3 = new QFormLayout(gb3);
    m_faceFocusCb = new QCheckBox("Mise au point automatique sur le visage");
    fl3->addRow(m_faceFocusCb);
    {
        auto *row = new QHBoxLayout;
        m_focusSlider = new QSlider(Qt::Horizontal);
        m_focusSlider->setRange(0, 100);
        m_focusLbl = new QLabel("0");
        m_focusLbl->setFixedWidth(30);
        row->addWidget(m_focusSlider);
        row->addWidget(m_focusLbl);
        fl3->addRow("Manuel :", new QWidget);
        gb3->layout()->addItem(row);
    }
    root->addWidget(gb3);

    // Alimentation
    auto *gb4 = new QGroupBox("Alimentation");
    auto *pl = new QHBoxLayout(gb4);
    m_wakeBtn  = new QPushButton("⏻  Réveiller");
    m_sleepBtn = new QPushButton("💤  Veille");
    m_wakeBtn->setProperty("success", true);
    pl->addWidget(m_wakeBtn);
    pl->addWidget(m_sleepBtn);
    root->addWidget(gb4);
    root->addStretch();

    scroll->setWidget(inner);
    auto *ml = new QVBoxLayout(this);
    ml->setContentsMargins(0,0,0,0);
    ml->addWidget(scroll);

    connect(m_fovCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraWidget::onFovChanged);
    connect(m_hdrCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraWidget::onHdrChanged);
    connect(m_wbCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraWidget::onWbChanged);
    connect(m_wbTempSlider, &QSlider::valueChanged, this, &CameraWidget::onWbTempChanged);
    connect(m_faceFocusCb,  &QCheckBox::toggled,    this, &CameraWidget::onFaceFocusToggled);
    connect(m_focusSlider,  &QSlider::valueChanged, this, &CameraWidget::onFocusChanged);
    connect(m_wakeBtn, &QPushButton::clicked, this, &CameraWidget::onWakeup);
    connect(m_sleepBtn,&QPushButton::clicked, this, &CameraWidget::onSleep);
}

void CameraWidget::onFovChanged(int)
{ DeviceManager::instance().setFov(static_cast<Device::FovType>(m_fovCombo->currentData().toInt())); }
void CameraWidget::onHdrChanged(int)
{ DeviceManager::instance().setHdr(m_hdrCombo->currentData().toInt()); }
void CameraWidget::onWbChanged(int)
{
    auto wb = static_cast<Device::DevWhiteBalanceType>(m_wbCombo->currentData().toInt());
    m_wbTempSlider->setEnabled(wb == Device::DevWhiteBalanceManual);
    DeviceManager::instance().setWhiteBalance(wb, m_wbTempSlider->value());
}
void CameraWidget::onWbTempChanged(int v)
{
    m_wbTempLbl->setText(QString("%1 K").arg(v));
    if (m_wbCombo->currentData().toInt() == int(Device::DevWhiteBalanceManual))
        DeviceManager::instance().setWhiteBalance(Device::DevWhiteBalanceManual, v);
}
void CameraWidget::onFaceFocusToggled(bool c) { DeviceManager::instance().setFaceFocus(c); }
void CameraWidget::onFocusChanged(int v) { m_focusLbl->setText(QString::number(v)); DeviceManager::instance().setFocusAbsolute(v); }
void CameraWidget::onWakeup() { DeviceManager::instance().setDevRunStatus(Device::DevStatusRun); }
void CameraWidget::onSleep()  { DeviceManager::instance().setDevRunStatus(Device::DevStatusSleep); }
void CameraWidget::onStatusUpdated(Device::CameraStatus) {}
void CameraWidget::setEnabled(bool e) { QWidget::setEnabled(e); }
