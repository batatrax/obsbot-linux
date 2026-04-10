#include "GestureWidget.h"
#include <obsbot/GestureManager.h>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>

GestureWidget::GestureWidget(QWidget *parent) : QWidget(parent) { buildUi(); }

void GestureWidget::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16);
    root->setSpacing(16);

    auto *gb = new QGroupBox("Gesture Control 2.0");
    auto *gl = new QVBoxLayout(gb);

    struct GestureDef { const char *label; const char *desc; int id; };
    GestureDef defs[] = {
        {"✋  Sélection de cible",     "Levez la main pour sélectionner une cible", 0},
        {"🤏  Zoom par geste",          "Pincez pour zoomer/dézoomer",               1},
        {"📸  Déclenchement photo",     "Geste pour prendre une photo",              2},
    };

    for (auto &d : defs) {
        auto *row = new QHBoxLayout;
        auto *col = new QVBoxLayout;
        m_gestures[d.id] = new QCheckBox(d.label);
        m_gestures[d.id]->setStyleSheet("font-size:13px;");
        auto *desc = new QLabel(d.desc);
        desc->setProperty("dim", true);
        col->addWidget(m_gestures[d.id]);
        col->addWidget(desc);
        row->addLayout(col);
        gl->addLayout(row);

        connect(m_gestures[d.id], &QCheckBox::toggled, this,
                [id=d.id](bool checked) {
                    GestureManager::instance().setEnabled(id, checked);
                });
    }
    root->addWidget(gb);

    auto *info = new QLabel("Les gestes fonctionnent uniquement quand\nle suivi IA est activ\u00e9.");
    info->setProperty("dim", true);
    info->setWordWrap(true);
    root->addWidget(info);
    root->addStretch();
}

void GestureWidget::setEnabled(bool e) { QWidget::setEnabled(e); }
