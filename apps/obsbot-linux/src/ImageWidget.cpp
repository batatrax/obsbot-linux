#include "ImageWidget.h"
#include <obsbot/DeviceManager.h>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>

ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent) { buildUi(); }

void ImageWidget::buildUi()
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;}");
    auto *inner = new QWidget;
    auto *root  = new QVBoxLayout(inner);
    root->setContentsMargins(12,10,12,10);
    root->setSpacing(10);

    auto *gb1 = new QGroupBox("HDR");
    auto *fl1 = new QFormLayout(gb1);
    m_hdrCombo = new QComboBox;
    m_hdrCombo->addItem("Desactive",  int(Device::DevWdrModeNone));
    m_hdrCombo->addItem("HDR 2-en-1", int(Device::DevWdrModeDol2TO1));
    fl1->addRow("Mode :", m_hdrCombo);
    root->addWidget(gb1);

    auto *gb2 = new QGroupBox("Balance des blancs");
    auto *fl2 = new QFormLayout(gb2);
    m_wbCombo = new QComboBox;
    m_wbCombo->addItem("Automatique",     int(Device::DevWhiteBalanceAuto));
    m_wbCombo->addItem("Manuel",          int(Device::DevWhiteBalanceManual));
    m_wbCombo->addItem("Lumiere du jour", int(Device::DevWhiteBalanceDaylight));
    m_wbCombo->addItem("Nuageux",         int(Device::DevWhiteBalanceCloudy));
    m_wbCombo->addItem("Tungstene",       int(Device::DevWhiteBalanceTungsten));
    m_wbCombo->addItem("Fluorescent",     int(Device::DevWhiteBalanceFluorescent));
    fl2->addRow("Mode :", m_wbCombo);
    auto *tempW = new QWidget;
    auto *tl = new QHBoxLayout(tempW);
    tl->setContentsMargins(0,0,0,0);
    m_wbTempSlider = new QSlider(Qt::Horizontal);
    m_wbTempSlider->setRange(2000,10000);
    m_wbTempSlider->setValue(5000);
    m_wbTempSlider->setEnabled(false);
    m_wbTempLbl = new QLabel("5000 K");
    m_wbTempLbl->setFixedWidth(58);
    tl->addWidget(m_wbTempSlider);
    tl->addWidget(m_wbTempLbl);
    fl2->addRow("Temperature :", tempW);
    root->addWidget(gb2);

    auto *gb3 = new QGroupBox("Focus manuel");
    auto *fl3 = new QFormLayout(gb3);
    auto *frow = new QWidget;
    auto *frl  = new QHBoxLayout(frow);
    frl->setContentsMargins(0,0,0,0);
    m_focusSlider = new QSlider(Qt::Horizontal);
    m_focusSlider->setRange(0,100);
    m_focusLbl = new QLabel("0");
    m_focusLbl->setFixedWidth(28);
    frl->addWidget(m_focusSlider);
    frl->addWidget(m_focusLbl);
    fl3->addRow("Valeur :", frow);
    root->addWidget(gb3);
    root->addStretch();

    scroll->setWidget(inner);
    auto *ml = new QVBoxLayout(this);
    ml->setContentsMargins(0,0,0,0);
    ml->addWidget(scroll);

    connect(m_hdrCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImageWidget::onHdrChanged);
    connect(m_wbCombo,   QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImageWidget::onWbChanged);
    connect(m_wbTempSlider, &QSlider::valueChanged, this, &ImageWidget::onWbTempChanged);
    connect(m_focusSlider,  &QSlider::valueChanged, this, &ImageWidget::onFocusChanged);
}

void ImageWidget::onHdrChanged(int)
{ DeviceManager::instance().setHdr(m_hdrCombo->currentData().toInt()); }

void ImageWidget::onWbChanged(int)
{
    auto wb = static_cast<Device::DevWhiteBalanceType>(m_wbCombo->currentData().toInt());
    m_wbTempSlider->setEnabled(wb == Device::DevWhiteBalanceManual);
    DeviceManager::instance().setWhiteBalance(wb, m_wbTempSlider->value());
}

void ImageWidget::onWbTempChanged(int v)
{
    m_wbTempLbl->setText(QString("%1 K").arg(v));
    if (m_wbCombo->currentData().toInt() == int(Device::DevWhiteBalanceManual))
        DeviceManager::instance().setWhiteBalance(Device::DevWhiteBalanceManual, v);
}

void ImageWidget::onFocusChanged(int v)
{
    m_focusLbl->setText(QString::number(v));
    DeviceManager::instance().setFocusAbsolute(v);
}

void ImageWidget::onStatusUpdated(Device::CameraStatus) {}
void ImageWidget::setEnabled(bool e) { QWidget::setEnabled(e); }
