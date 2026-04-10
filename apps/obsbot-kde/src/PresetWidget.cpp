#include "PresetWidget.h"
#include <obsbot/PresetManager.h>
#include <obsbot/DeviceManager.h>
#include <QLabel>
#include <QPushButton>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QScrollArea>

PresetWidget::PresetWidget(QWidget *parent) : QWidget(parent) { buildUi(); }

QFrame* PresetWidget::buildPresetCard(int id)
{
    auto *card = new QFrame;
    card->setStyleSheet("QFrame{background:#313244;border-radius:8px;padding:4px;}");
    auto *cl = new QVBoxLayout(card);

    auto *top = new QHBoxLayout;
    m_nameLbl[id] = new QLabel(QString("Preset %1").arg(id+1));
    m_nameLbl[id]->setStyleSheet("font-weight:bold; color:#cdd6f4; font-size:14px;");
    top->addWidget(m_nameLbl[id]);
    top->addStretch();

    auto *status = new QLabel("Vide");
    status->setObjectName(QString("status_%1").arg(id));
    status->setStyleSheet("color:#585b70; font-size:11px;");
    top->addWidget(status);
    cl->addLayout(top);

    auto *btnRow = new QHBoxLayout;
    m_saveBtn[id] = new QPushButton("💾 Sauvegarder");
    m_loadBtn[id] = new QPushButton("▶ Appliquer");
    m_loadBtn[id]->setProperty("accent", true);
    m_delBtn[id]  = new QPushButton("🗑");
    m_delBtn[id]->setFixedWidth(36);
    m_delBtn[id]->setProperty("danger", true);
    btnRow->addWidget(m_saveBtn[id]);
    btnRow->addWidget(m_loadBtn[id]);
    btnRow->addWidget(m_delBtn[id]);
    cl->addLayout(btnRow);

    auto *bootBtn = new QPushButton("Définir comme position initiale");
    bootBtn->setObjectName(QString("boot_%1").arg(id));
    bootBtn->setStyleSheet("font-size:11px; color:#a6adc8;");
    cl->addWidget(bootBtn);

    connect(m_saveBtn[id], &QPushButton::clicked, this, [this,id]{ onSave(id); });
    connect(m_loadBtn[id], &QPushButton::clicked, this, [this,id]{ onLoad(id); });
    connect(m_delBtn[id],  &QPushButton::clicked, this, [this,id]{ onDelete(id); });
    connect(bootBtn, &QPushButton::clicked, this, [this,id]{ onSetBoot(id); });

    return card;
}

void PresetWidget::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16);
    root->setSpacing(12);

    auto *title = new QLabel("Gestion des Presets");
    title->setStyleSheet("font-size:16px; font-weight:bold; color:#89b4fa;");
    root->addWidget(title);

    auto *sub = new QLabel("Sauvegardez jusqu'a 3 configurations completes (position, zoom, IA, camera)");
    sub->setProperty("dim", true);
    sub->setWordWrap(true);
    root->addWidget(sub);

    for (int i = 0; i < 3; i++)
        root->addWidget(buildPresetCard(i));

    root->addStretch();
    connect(&PresetManager::instance(), &PresetManager::presetsChanged, this, &PresetWidget::refresh);
    refresh();
}

void PresetWidget::refresh()
{
    for (int i = 0; i < 3; i++) {
        auto d = PresetManager::instance().get(i);
        m_nameLbl[i]->setText(d.valid ? d.name : QString("Preset %1").arg(i+1));
        m_loadBtn[i]->setEnabled(d.valid);
        m_delBtn[i]->setEnabled(d.valid);
        auto *s = findChild<QLabel*>(QString("status_%1").arg(i));
        if (s) s->setText(d.valid ? "Configuré ✓" : "Vide");
    }
}

void PresetWidget::onSave(int id)
{
    bool ok;
    QString name = QInputDialog::getText(this, "Nommer le preset",
        "Nom du preset :", QLineEdit::Normal,
        QString("Preset %1").arg(id+1), &ok);
    if (!ok) return;
    PresetManager::instance().captureFromDevice(id, name);
}

void PresetWidget::onLoad(int id)
{ PresetManager::instance().applyToDevice(id); }

void PresetWidget::onDelete(int id)
{ PresetManager::instance().remove(id); }

void PresetWidget::onRename(int id) { onSave(id); }

void PresetWidget::onSetBoot(int id)
{
    auto d = PresetManager::instance().get(id);
    if (!d.valid) return;
    DeviceManager::instance().gimbalSetAngle(d.yaw, d.pitch);
}

void PresetWidget::setEnabled(bool e) { QWidget::setEnabled(e); }
