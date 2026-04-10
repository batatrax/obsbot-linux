#include "StatusBar.h"
#include <QLabel>
#include <QHBoxLayout>

StatusBar::StatusBar(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(28);
    setStyleSheet("background:#11111b; border-top:1px solid #313244;");
    auto *l = new QHBoxLayout(this);
    l->setContentsMargins(12, 0, 12, 0);
    m_dot     = new QLabel("●");
    m_name    = new QLabel("En attente de la caméra…");
    m_version = new QLabel();
    m_dot->setStyleSheet("color:#585b70; font-size:10px;");
    m_name->setStyleSheet("color:#a6adc8; font-size:11px;");
    m_version->setStyleSheet("color:#585b70; font-size:11px;");
    l->addWidget(m_dot);
    l->addWidget(m_name);
    l->addStretch();
    l->addWidget(m_version);
}

void StatusBar::setStatus(bool connected, const QString &name, const QString &version)
{
    m_dot->setStyleSheet(connected ? "color:#a6e3a1; font-size:10px;" : "color:#f38ba8; font-size:10px;");
    m_name->setText(connected ? name : "Déconnecté");
    m_version->setText(connected ? "FW " + version : QString());
}
