#include "AiWidget.h"
#include <obsbot/DeviceManager.h>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTimer>

AiWidget::AiWidget(QWidget *parent) : QWidget(parent) { buildUi(); }

void AiWidget::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12,10,12,10);
    root->setSpacing(10);

    auto *gb1 = new QGroupBox("Mode de suivi");
    auto *fl1 = new QFormLayout(gb1);
    m_modeCombo = new QComboBox;
    m_modeCombo->addItem("Humain - Corps entier", 0);
    m_modeCombo->addItem("Humain - Buste",        1);
    m_modeCombo->addItem("Humain - Gros plan",    2);
    m_modeCombo->addItem("Groupe",                3);
    m_modeCombo->addItem("Main / Geste",          4);
    fl1->addRow("Mode :", m_modeCombo);
    m_faceFocusCb = new QCheckBox("Mise au point automatique visage");
    fl1->addRow(m_faceFocusCb);
    m_zoomLockCb = new QCheckBox("Verrouiller le zoom");
    m_zoomLockCb->setToolTip("Empeche l'IA de dezoomer automatiquement");
    fl1->addRow(m_zoomLockCb);
    m_activateBtn = new QPushButton("Activer le suivi IA");
    m_activateBtn->setMinimumHeight(38);
    m_activateBtn->setStyleSheet(
        "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;"
        "border-radius:6px;font-weight:bold;font-size:13px;}"
        "QPushButton:hover{background:#b4d0fa;}"
        "QPushButton:disabled{background:#313244;color:#585b70;}");
    fl1->addRow(m_activateBtn);
    m_statusLbl = new QLabel("IA inactive");
    m_statusLbl->setAlignment(Qt::AlignCenter);
    m_statusLbl->setStyleSheet(
        "color:#585b70;font-style:italic;padding:6px;background:#24243e;border-radius:6px;");
    fl1->addRow(m_statusLbl);
    root->addWidget(gb1);

    auto *gb2 = new QGroupBox("Gestes (Gesture Control 2.0)");
    auto *fl2 = new QFormLayout(gb2);
    struct GDef { const char *l, *t; };
    GDef gdefs[] = {
        {"Selection de cible",  "Levez la main pour selectionner"},
        {"Zoom par geste",      "Pincez pour zoomer / dezoomer"},
        {"Photo par geste",     "Geste pour declencher une photo"},
    };
    for (int i = 0; i < 3; i++) {
        m_gestureChk[i] = new QCheckBox(gdefs[i].l);
        m_gestureChk[i]->setToolTip(gdefs[i].t);
        fl2->addRow(m_gestureChk[i]);
        connect(m_gestureChk[i], &QCheckBox::toggled, this,
            [this,i](bool c){ onGestureChanged(i,c); });
    }
    root->addWidget(gb2);
    root->addStretch();

    connect(m_activateBtn, &QPushButton::clicked, this, &AiWidget::onActivateToggle);
}

void AiWidget::applySettings()
{
    DeviceManager::instance().setFaceFocus(m_faceFocusCb->isChecked());
    DeviceManager::instance().setAiAutoZoom(!m_zoomLockCb->isChecked());
    int idx = m_modeCombo->currentIndex();
    switch(idx) {
        case 0: m_savedMode=Device::AiWorkModeHuman; m_savedSub=Device::AiSubModeNormal;    break;
        case 1: m_savedMode=Device::AiWorkModeHuman; m_savedSub=Device::AiSubModeUpperBody; break;
        case 2: m_savedMode=Device::AiWorkModeHuman; m_savedSub=Device::AiSubModeCloseUp;   break;
        case 3: m_savedMode=Device::AiWorkModeGroup; m_savedSub=Device::AiSubModeNormal;    break;
        case 4: m_savedMode=Device::AiWorkModeHand;  m_savedSub=Device::AiSubModeNormal;    break;
        default: break;
    }
    DeviceManager::instance().setAiMode(m_savedMode, m_savedSub);
}

void AiWidget::onActivateToggle()
{
    if (!m_aiActive) {
        applySettings();
        m_aiActive=true; m_aiSuspended=false;
        m_activateBtn->setText("Desactiver le suivi IA");
        m_activateBtn->setStyleSheet(
            "QPushButton{background:#f38ba8;color:#1e1e2e;border:none;"
            "border-radius:6px;font-weight:bold;font-size:13px;}"
            "QPushButton:hover{background:#f5a0b8;}");
        m_statusLbl->setText("IA active");
        m_statusLbl->setStyleSheet(
            "color:#a6e3a1;font-weight:bold;padding:6px;background:#24243e;border-radius:6px;");
        emit aiActivated(m_savedMode, m_savedSub);
    } else {
        DeviceManager::instance().cancelAiMode();
        DeviceManager::instance().setAiAutoZoom(true);
        m_aiActive=false; m_aiSuspended=false;
        m_activateBtn->setText("Activer le suivi IA");
        m_activateBtn->setStyleSheet(
            "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;"
            "border-radius:6px;font-weight:bold;font-size:13px;}"
            "QPushButton:hover{background:#b4d0fa;}"
            "QPushButton:disabled{background:#313244;color:#585b70;}");
        m_statusLbl->setText("IA inactive");
        m_statusLbl->setStyleSheet(
            "color:#585b70;font-style:italic;padding:6px;background:#24243e;border-radius:6px;");
        emit aiDeactivated();
    }
}

void AiWidget::suspendAi()
{
    if (!m_aiActive || m_aiSuspended) return;
    m_aiSuspended=true;
    DeviceManager::instance().cancelAiMode();
    m_statusLbl->setText("IA suspendue (joystick actif)");
    m_statusLbl->setStyleSheet(
        "color:#f9e2af;font-style:italic;padding:6px;background:#24243e;border-radius:6px;");
}

void AiWidget::resumeAi()
{
    if (!m_aiActive || !m_aiSuspended) return;
    QTimer::singleShot(500, this, [this]{
        if (!m_aiActive || !m_aiSuspended) return;
        m_aiSuspended=false;
        DeviceManager::instance().setAiMode(m_savedMode, m_savedSub);
        m_statusLbl->setText("IA active");
        m_statusLbl->setStyleSheet(
            "color:#a6e3a1;font-weight:bold;padding:6px;background:#24243e;border-radius:6px;");
    });
}

void AiWidget::onGestureChanged(int g, bool e)
{ DeviceManager::instance().setGesture(g,e); }

void AiWidget::onStatusUpdated(Device::CameraStatus) {}
void AiWidget::setEnabled(bool e) { QWidget::setEnabled(e); }
