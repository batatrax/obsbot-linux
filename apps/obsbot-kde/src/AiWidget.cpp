#include "AiWidget.h"
#include <obsbot/DeviceManager.h>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

AiWidget::AiWidget(QWidget *parent) : QWidget(parent) { buildUi(); }

void AiWidget::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16);
    root->setSpacing(16);

    auto *gb = new QGroupBox("Suivi automatique (IA)");
    auto *gl = new QFormLayout(gb);
    m_modeCombo = new QComboBox;
    m_modeCombo->addItem("Humain",  int(Device::AiWorkModeHuman));
    m_modeCombo->addItem("Groupe",  int(Device::AiWorkModeGroup));
    m_modeCombo->addItem("Main",    int(Device::AiWorkModeHand));
    gl->addRow("Mode :", m_modeCombo);

    m_subCombo = new QComboBox;
    m_subCombo->addItem("Normal",         int(Device::AiSubModeNormal));
    m_subCombo->addItem("Buste",          int(Device::AiSubModeUpperBody));
    m_subCombo->addItem("Gros plan",      int(Device::AiSubModeCloseUp));
    m_subCombo->addItem("Corps entier",   int(Device::AiSubModeNormal));
    gl->addRow("Cadrage :", m_subCombo);
    root->addWidget(gb);

    m_statusLabel = new QLabel("IA inactive");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color:#585b70; font-style:italic; padding:8px; "
        "background:#24243e; border-radius:6px;");
    root->addWidget(m_statusLabel);

    auto *row = new QHBoxLayout;
    m_activateBtn = new QPushButton("▶  Activer l'IA");
    m_activateBtn->setProperty("accent", true);
    m_activateBtn->setMinimumHeight(40);
    m_cancelBtn = new QPushButton("■  Désactiver");
    m_cancelBtn->setEnabled(false);
    m_cancelBtn->setMinimumHeight(40);
    row->addWidget(m_activateBtn);
    row->addWidget(m_cancelBtn);
    root->addLayout(row);
    root->addStretch();

    connect(m_activateBtn, &QPushButton::clicked, this, &AiWidget::onActivate);
    connect(m_cancelBtn,   &QPushButton::clicked, this, &AiWidget::onCancel);
}

void AiWidget::onActivate()
{
    auto mode = static_cast<Device::AiWorkModeType>(m_modeCombo->currentData().toInt());
    auto sub  = static_cast<Device::AiSubModeType>(m_subCombo->currentData().toInt());
    DeviceManager::instance().setAiMode(mode, sub);
    m_statusLabel->setText("✓  IA active — " + m_modeCombo->currentText() + " / " + m_subCombo->currentText());
    m_statusLabel->setStyleSheet("color:#a6e3a1; font-weight:bold; padding:8px; background:#24243e; border-radius:6px;");
    m_activateBtn->setEnabled(false); m_cancelBtn->setEnabled(true);
}

void AiWidget::onCancel()
{
    DeviceManager::instance().cancelAiMode();
    m_statusLabel->setText("IA inactive");
    m_statusLabel->setStyleSheet("color:#585b70; font-style:italic; padding:8px; background:#24243e; border-radius:6px;");
    m_activateBtn->setEnabled(true); m_cancelBtn->setEnabled(false);
}

void AiWidget::setEnabled(bool e) { QWidget::setEnabled(e); }
