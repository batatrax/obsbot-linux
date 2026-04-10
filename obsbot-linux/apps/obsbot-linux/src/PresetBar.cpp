#include "PresetBar.h"
#include <obsbot/PresetManager.h>
#include <QPushButton>
#include <QLabel>
#include <QInputDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>

PresetBar::PresetBar(QWidget *parent) : QWidget(parent) { buildUi(); }

void PresetBar::buildUi()
{
    setStyleSheet("background:#181825; border-top:1px solid #313244;");
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8,4,8,6);
    root->setSpacing(4);

    auto *title = new QLabel("Presets");
    title->setStyleSheet("color:#89b4fa;font-weight:bold;font-size:11px;");
    root->addWidget(title);

    auto *row = new QHBoxLayout;
    row->setSpacing(6);

    for (int i = 0; i < 3; i++) {
        auto *card = new QFrame;
        card->setStyleSheet(
            "QFrame{background:#24243e;border:1px solid #45475a;border-radius:6px;}");
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(6,4,6,4);
        cl->setSpacing(3);

        m_nameLbl[i] = new QLabel(QString("Preset %1").arg(i+1));
        m_nameLbl[i]->setStyleSheet("color:#585b70;font-size:11px;");
        m_nameLbl[i]->setAlignment(Qt::AlignCenter);

        m_loadBtn[i] = new QPushButton("Appliquer");
        m_loadBtn[i]->setStyleSheet(
            "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;"
            "border-radius:4px;font-size:10px;padding:3px 6px;font-weight:bold;}"
            "QPushButton:hover{background:#b4d0fa;}"
            "QPushButton:disabled{background:#313244;color:#585b70;}");

        m_saveBtn[i] = new QPushButton("Sauver");
        m_saveBtn[i]->setStyleSheet(
            "QPushButton{background:#313244;color:#a6adc8;border:1px solid #45475a;"
            "border-radius:4px;font-size:10px;padding:3px 6px;}"
            "QPushButton:hover{background:#45475a;color:#cdd6f4;}");

        cl->addWidget(m_nameLbl[i]);
        cl->addWidget(m_loadBtn[i]);
        cl->addWidget(m_saveBtn[i]);
        row->addWidget(card);

        connect(m_loadBtn[i], &QPushButton::clicked, this, [this,i]{ onLoad(i); });
        connect(m_saveBtn[i], &QPushButton::clicked, this, [this,i]{ onSave(i); });
    }
    root->addLayout(row);

    connect(&PresetManager::instance(), &PresetManager::presetsChanged,
            this, &PresetBar::refresh);
    refresh();
}

void PresetBar::refresh()
{
    for (int i = 0; i < 3; i++) {
        auto d = PresetManager::instance().get(i);
        m_nameLbl[i]->setText(d.valid ? d.name : QString("Preset %1").arg(i+1));
        m_loadBtn[i]->setEnabled(d.valid);
        m_nameLbl[i]->setStyleSheet(d.valid
            ? "color:#a6e3a1;font-size:11px;font-weight:bold;"
            : "color:#585b70;font-size:11px;");
    }
}

void PresetBar::onLoad(int id) { PresetManager::instance().applyToDevice(id); }
void PresetBar::onSave(int id)
{
    bool ok;
    QString name = QInputDialog::getText(this, "Nommer le preset", "Nom :",
        QLineEdit::Normal, QString("Preset %1").arg(id+1), &ok);
    if (ok) PresetManager::instance().captureFromDevice(id, name);
}
void PresetBar::setEnabled(bool e) { QWidget::setEnabled(e); }
