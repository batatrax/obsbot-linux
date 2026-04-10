#include "FirmwareWidget.h"
#include <obsbot/DeviceManager.h>
#include <obsbot/FirmwareManager.h>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>

FirmwareWidget::FirmwareWidget(QWidget *parent) : QWidget(parent) { buildUi(); }

void FirmwareWidget::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16,16,16,16);
    root->setSpacing(16);

    // Version actuelle
    auto *gbInfo = new QGroupBox("Version actuelle");
    auto *il = new QVBoxLayout(gbInfo);
    m_versionLbl = new QLabel("–");
    m_versionLbl->setStyleSheet("font-size:18px; font-weight:bold; color:#89b4fa;");
    il->addWidget(m_versionLbl);
    root->addWidget(gbInfo);

    // Mise à jour manuelle
    auto *gbUpgrade = new QGroupBox("Mise à jour firmware");
    auto *ul = new QVBoxLayout(gbUpgrade);

    auto *info = new QLabel(
        "1. Telechargez le firmware depuis obsbot.com\n"
        "2. Selectionnez le fichier .bin ci-dessous\n"
        "3. Cliquez sur Mettre a jour\n\n"
    info->setProperty("dim", true);
    info->setWordWrap(true);
    ul->addWidget(info);

    auto *downloadBtn = new QPushButton("🌐  Télécharger le firmware (obsbot.com)");
    downloadBtn->setStyleSheet("color:#89b4fa;");
    ul->addWidget(downloadBtn);

    auto *fileRow = new QHBoxLayout;
    m_fileLbl = new QLabel("Aucun fichier sélectionné");
    m_fileLbl->setStyleSheet("color:#585b70; font-size:11px;");
    m_browseBtn = new QPushButton("📂  Parcourir…");
    fileRow->addWidget(m_fileLbl, 1);
    fileRow->addWidget(m_browseBtn);
    ul->addLayout(fileRow);

    m_upgradeBtn = new QPushButton("⬆  Mettre à jour le firmware");
    m_upgradeBtn->setProperty("accent", true);
    m_upgradeBtn->setMinimumHeight(42);
    m_upgradeBtn->setEnabled(false);
    ul->addWidget(m_upgradeBtn);

    m_progress = new QProgressBar;
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_progress->setVisible(false);
    ul->addWidget(m_progress);

    m_statusLbl = new QLabel();
    m_statusLbl->setAlignment(Qt::AlignCenter);
    m_statusLbl->setVisible(false);
    ul->addWidget(m_statusLbl);

    root->addWidget(gbUpgrade);
    root->addStretch();

    auto &fm = FirmwareManager::instance();
    connect(downloadBtn, &QPushButton::clicked, this, []{
        QDesktopServices::openUrl(QUrl("https://www.obsbot.com/download/obsbot-tiny-2-lite"));
    });
    connect(m_browseBtn, &QPushButton::clicked, this, &FirmwareWidget::onBrowse);
    connect(m_upgradeBtn, &QPushButton::clicked, this, [this]{
        FirmwareManager::instance().upgradeFromFile(m_selectedFile);
    });
    connect(&fm, &FirmwareManager::progress,  this, &FirmwareWidget::onProgress);
    connect(&fm, &FirmwareManager::finished,  this, &FirmwareWidget::onFinished);
    connect(&DeviceManager::instance(), &DeviceManager::deviceConnected, this, [this]{
        m_versionLbl->setText(DeviceManager::instance().deviceVersion());
    });
}

void FirmwareWidget::onBrowse()
{
    QString f = QFileDialog::getOpenFileName(this, "Sélectionner le firmware",
        QDir::homePath(), "Firmware OBSBOT (*.bin);;Tous les fichiers (*)");
    if (f.isEmpty()) return;
    m_selectedFile = f;
    m_fileLbl->setText(QFileInfo(f).fileName());
    m_fileLbl->setStyleSheet("color:#cdd6f4; font-size:11px;");
    m_upgradeBtn->setEnabled(true);
}

void FirmwareWidget::onProgress(int p, const QString &msg)
{
    m_progress->setVisible(true);
    m_progress->setValue(p);
    m_statusLbl->setText(msg);
    m_statusLbl->setStyleSheet("color:#89b4fa;");
    m_statusLbl->setVisible(true);
    m_upgradeBtn->setEnabled(false);
}

void FirmwareWidget::onFinished(bool ok, const QString &msg)
{
    m_statusLbl->setText(ok ? "✓  " + msg : "✗  " + msg);
    m_statusLbl->setStyleSheet(ok ? "color:#a6e3a1; font-weight:bold;" : "color:#f38ba8; font-weight:bold;");
    m_progress->setVisible(false);
    m_upgradeBtn->setEnabled(true);
}

void FirmwareWidget::setEnabled(bool e) { QWidget::setEnabled(e); }
