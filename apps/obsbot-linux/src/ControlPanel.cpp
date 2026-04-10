#include "ControlPanel.h"
#include "JoystickWidget.h"
#include <obsbot/DeviceManager.h>
#include <obsbot/PresetManager.h>
#include <obsbot/FirmwareManager.h>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QKeyEvent>
#include <QDesktopServices>
#include <QUrl>

// ScrollArea qui ne capture pas les fleches clavier
class NoArrowScrollArea : public QScrollArea {
protected:
    void keyPressEvent(QKeyEvent *e) override {
        // Ne pas capturer les fleches — les laisser remonter au parent
        switch (e->key()) {
            case Qt::Key_Up: case Qt::Key_Down:
            case Qt::Key_Left: case Qt::Key_Right:
                e->ignore(); return;
            default: QScrollArea::keyPressEvent(e);
        }
    }
};

ControlPanel::ControlPanel(QWidget *parent) : QWidget(parent) { buildUi(); }

void ControlPanel::buildUi()
{
    auto *scroll = new NoArrowScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;}");
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *inner = new QWidget;
    auto *root  = new QVBoxLayout(inner);
    root->setContentsMargins(8,8,8,8);
    root->setSpacing(8);

    root->addWidget(buildGimbalSection());
    root->addWidget(buildAiSection());
    root->addWidget(buildImageSection());
    root->addWidget(buildPresetSection());
    root->addWidget(buildFirmwareSection());
    root->addStretch();

    scroll->setWidget(inner);
    auto *ml = new QVBoxLayout(this);
    ml->setContentsMargins(0,0,0,0);
    ml->addWidget(scroll);

    connect(&PresetManager::instance(), &PresetManager::presetsChanged,
        this, [this]{
            for (int i=0;i<3;i++){
                auto d = PresetManager::instance().get(i);
                m_presetLbl[i]->setText(d.valid ? d.name : QString("P%1").arg(i+1));
                m_presetLoad[i]->setEnabled(d.valid);
            }
        });

    connect(&FirmwareManager::instance(), &FirmwareManager::progress,
        this, [this](int p, const QString &msg){
            m_fwVersionLbl->setText(QString("Mise a jour: %1% — %2").arg(p).arg(msg));
        });
    connect(&FirmwareManager::instance(), &FirmwareManager::finished,
        this, [this](bool ok, const QString &msg){
            m_fwVersionLbl->setText(ok ? "OK: " + msg : "Erreur: " + msg);
            m_fwUpgradeBtn->setEnabled(true);
        });
}

// ── Section Gimbal ─────────────────────────────────────────────────────────────
QWidget* ControlPanel::buildGimbalSection()
{
    auto *gb = new QGroupBox("Gimbal & Zoom");
    auto *root = new QVBoxLayout(gb);
    root->setSpacing(6);

    // Options en une ligne
    {
        auto *row = new QHBoxLayout;
        m_invertXCb = new QCheckBox("Inv.X");
        m_invertYCb = new QCheckBox("Inv.Y");
        m_invertXCb->setToolTip("Inverser gauche/droite");
        m_invertYCb->setToolTip("Inverser haut/bas");
        auto *hint = new QLabel("Clic sur joystick → clavier actif (ZQSD/fleches)");
        hint->setStyleSheet("color:#45475a;font-size:10px;font-style:italic;");
        row->addWidget(m_invertXCb);
        row->addWidget(m_invertYCb);
        row->addStretch();
        row->addWidget(hint);
        root->addLayout(row);
    }

    // Joystick
    m_joystick = new JoystickWidget;
    m_joystick->setFixedSize(200, 200);
    root->addWidget(m_joystick, 0, Qt::AlignCenter);

    m_speedLbl = new QLabel("Pan: 0  Tilt: 0");
    m_speedLbl->setAlignment(Qt::AlignCenter);
    m_speedLbl->setStyleSheet("color:#585b70;font-size:10px;");
    root->addWidget(m_speedLbl);

    // Boutons Reset + Home + Cibler
    {
        auto *row = new QHBoxLayout;
        auto *resetBtn = new QPushButton("⟳ Reset");
        auto *homeBtn  = new QPushButton("🏠");
        auto *faceBtn  = new QPushButton("👤 Visage");
        homeBtn->setToolTip("Mémoriser le zoom actuel (retour ici si suivi perdu)");
        homeBtn->setMaximumHeight(28);
        connect(homeBtn, &QPushButton::clicked, this, [this]{
            float yaw = 0, pitch = 0;
            DeviceManager::instance().getGimbalAngle(yaw, pitch);
            m_homeYaw   = yaw;
            m_homePitch = pitch;
            m_homeZoom  = m_zoom->value() / 100.0f;
            m_aiStatusLbl->setText(
                QString("Accueil: %1x  Pan:%2°  Tilt:%3°")
                    .arg(double(m_homeZoom), 0, 'f', 1)
                    .arg(double(m_homeYaw),  0, 'f', 0)
                    .arg(double(m_homePitch),0, 'f', 0));
            QTimer::singleShot(3000, this, [this]{
                if (!m_aiActive) m_aiStatusLbl->setText("IA inactive");
            });
        });
        faceBtn->setStyleSheet(
            "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;"
            "border-radius:5px;font-weight:bold;padding:3px 10px;}"
            "QPushButton:hover{background:#b4d0fa;}"
            "QPushButton:disabled{background:#313244;color:#585b70;}");
        resetBtn->setMaximumHeight(28);
        faceBtn->setMaximumHeight(28);
        connect(resetBtn, &QPushButton::clicked, this, &ControlPanel::onReset);
        connect(faceBtn,  &QPushButton::clicked, this, &ControlPanel::onFaceTarget);
        row->addWidget(resetBtn); row->addWidget(homeBtn); row->addWidget(faceBtn);
        root->addLayout(row);
    }

    // Zoom
    {
        auto *row = new QHBoxLayout;
        row->addWidget(new QLabel("1x"));
        m_zoom = new QSlider(Qt::Horizontal);
        m_zoom->setRange(100,400); m_zoom->setValue(100);
        m_zoomLbl = new QLabel("1.0x"); m_zoomLbl->setFixedWidth(34);
        row->addWidget(m_zoom,1);
        row->addWidget(new QLabel("4x"));
        row->addWidget(m_zoomLbl);
        root->addLayout(row);
    }

    // FOV
    {
        auto *row = new QHBoxLayout;
        row->addWidget(new QLabel("FOV:"));
        m_fovCombo = new QComboBox;
        m_fovCombo->addItem("86°", int(Device::FovType86));
        m_fovCombo->addItem("78°", int(Device::FovType78));
        m_fovCombo->addItem("65°", int(Device::FovType65));
        m_fovCombo->setMaximumWidth(80);
        row->addWidget(m_fovCombo);
        row->addStretch();
        root->addLayout(row);
    }

    connect(m_joystick, &JoystickWidget::speedChanged, this,
        [this](int p,int t){ m_speedLbl->setText(QString("Pan:%1 Tilt:%2").arg(p).arg(t)); });
    connect(m_joystick, &JoystickWidget::resetRequested,     this, &ControlPanel::onReset);
    connect(m_joystick, &JoystickWidget::faceTargetRequested,this, &ControlPanel::onFaceTarget);
    connect(m_joystick, &JoystickWidget::presetRequested,    this, &ControlPanel::onPresetLoad);
    connect(m_joystick, &JoystickWidget::userTouched, this, [this]{
        if (m_aiActive && !m_aiSuspended) {
            m_aiSuspended    = true;
            m_noTargetFrames = 0;
            DeviceManager::instance().cancelAiMode();
            m_aiStatusLbl->setText("IA suspendue");
            m_aiStatusLbl->setStyleSheet("color:#f9e2af;font-size:10px;");
        }
        emit joystickTouched();
    });
    connect(m_joystick, &JoystickWidget::userReleased, this, [this]{
        if (m_aiActive && m_aiSuspended) {
            QTimer::singleShot(500, this, [this]{
                if (!m_aiActive || !m_aiSuspended) return;
                m_aiSuspended = false;
                DeviceManager::instance().setAiMode(m_savedMode, m_savedSub);
                m_aiStatusLbl->setText("IA active");
                m_aiStatusLbl->setStyleSheet("color:#a6e3a1;font-size:10px;font-weight:bold;");
            });
        }
        emit joystickReleased();
    });
    connect(m_invertXCb, &QCheckBox::toggled, m_joystick, &JoystickWidget::setInvertX);
    connect(m_invertYCb, &QCheckBox::toggled, m_joystick, &JoystickWidget::setInvertY);
    connect(m_zoom, &QSlider::valueChanged,   this, &ControlPanel::onZoomChanged);
    connect(m_fovCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onFovChanged);

    return gb;
}

// ── Section IA ────────────────────────────────────────────────────────────────
QWidget* ControlPanel::buildAiSection()
{
    auto *gb   = new QGroupBox("IA & Suivi");
    auto *root = new QVBoxLayout(gb);
    root->setSpacing(5);

    // Mode + bouton sur la meme ligne
    {
        auto *row = new QHBoxLayout;
        m_aiModeCombo = new QComboBox;
        m_aiModeCombo->addItem("Humain - Corps",   0);
        m_aiModeCombo->addItem("Humain - Buste",   1);
        m_aiModeCombo->addItem("Humain - Gros plan",2);
        m_aiModeCombo->addItem("Groupe",            3);
        m_aiModeCombo->addItem("Main",              4);
        m_aiBtn = new QPushButton("Activer IA");
        m_aiBtn->setFixedHeight(26);
        m_aiBtn->setStyleSheet(
            "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;"
            "border-radius:5px;font-weight:bold;font-size:11px;padding:2px 10px;}"
            "QPushButton:hover{background:#b4d0fa;}"
            "QPushButton:disabled{background:#313244;color:#585b70;}");
        row->addWidget(m_aiModeCombo,1);
        row->addWidget(m_aiBtn);
        root->addLayout(row);
    }

    m_aiStatusLbl = new QLabel("IA inactive");
    m_aiStatusLbl->setStyleSheet("color:#585b70;font-size:10px;font-style:italic;");
    root->addWidget(m_aiStatusLbl);

    // Options
    {
        auto *row = new QHBoxLayout;
        m_faceFocusCb = new QCheckBox("MAP visage");
        m_zoomLockCb  = new QCheckBox("Lock zoom");
        m_zoomLockCb->setToolTip("Empeche le dezoom automatique de l'IA");
        row->addWidget(m_faceFocusCb);
        row->addWidget(m_zoomLockCb);
        row->addStretch();
        root->addLayout(row);
    }

    // Gestes en ligne
    {
        auto *row = new QHBoxLayout;
        const char *glabels[] = {"Cible","Zoom","Photo"};
        for (int i=0;i<3;i++){
            m_gesture[i] = new QCheckBox(glabels[i]);
            m_gesture[i]->setStyleSheet("font-size:10px;");
            row->addWidget(m_gesture[i]);
            connect(m_gesture[i],&QCheckBox::toggled,this,[this,i](bool e){ onGestureChanged(i,e); });
        }
        row->addStretch();
        root->addLayout(row);
    }

    connect(m_aiBtn, &QPushButton::clicked, this, &ControlPanel::onAiToggle);
    return gb;
}

// ── Section Image ─────────────────────────────────────────────────────────────
QWidget* ControlPanel::buildImageSection()
{
    auto *gb   = new QGroupBox("Image");
    auto *fl   = new QFormLayout(gb);
    fl->setSpacing(5);

    // HDR
    m_hdrCombo = new QComboBox;
    m_hdrCombo->addItem("HDR off",    int(Device::DevWdrModeNone));
    m_hdrCombo->addItem("HDR 2-en-1", int(Device::DevWdrModeDol2TO1));
    fl->addRow("HDR:", m_hdrCombo);

    // Balance blancs
    m_wbCombo = new QComboBox;
    m_wbCombo->addItem("Auto",       int(Device::DevWhiteBalanceAuto));
    m_wbCombo->addItem("Manuel",     int(Device::DevWhiteBalanceManual));
    m_wbCombo->addItem("Jour",       int(Device::DevWhiteBalanceDaylight));
    m_wbCombo->addItem("Nuageux",    int(Device::DevWhiteBalanceCloudy));
    m_wbCombo->addItem("Tungstene",  int(Device::DevWhiteBalanceTungsten));
    m_wbCombo->addItem("Fluo",       int(Device::DevWhiteBalanceFluorescent));
    fl->addRow("WB:", m_wbCombo);

    // Temperature WB
    {
        auto *row = new QHBoxLayout;
        m_wbTempSlider = new QSlider(Qt::Horizontal);
        m_wbTempSlider->setRange(2000,10000); m_wbTempSlider->setValue(5000);
        m_wbTempSlider->setEnabled(false);
        m_wbTempLbl = new QLabel("5000K"); m_wbTempLbl->setFixedWidth(50);
        row->addWidget(m_wbTempSlider); row->addWidget(m_wbTempLbl);
        auto *w = new QWidget; w->setLayout(row);
        fl->addRow("Temp:", w);
    }

    // Focus
    {
        auto *row = new QHBoxLayout;
        m_focusSlider = new QSlider(Qt::Horizontal);
        m_focusSlider->setRange(0,100);
        m_focusLbl = new QLabel("0"); m_focusLbl->setFixedWidth(24);
        row->addWidget(m_focusSlider); row->addWidget(m_focusLbl);
        auto *w = new QWidget; w->setLayout(row);
        fl->addRow("Focus:", w);
    }

    connect(m_hdrCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onHdrChanged);
    connect(m_wbCombo,   QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ControlPanel::onWbChanged);
    connect(m_wbTempSlider, &QSlider::valueChanged, this, &ControlPanel::onWbTempChanged);
    connect(m_focusSlider,  &QSlider::valueChanged, this, &ControlPanel::onFocusChanged);
    return gb;
}

// ── Section Presets ───────────────────────────────────────────────────────────
QWidget* ControlPanel::buildPresetSection()
{
    auto *gb   = new QGroupBox("Presets");
    auto *root = new QHBoxLayout(gb);
    root->setSpacing(4);

    for (int i=0;i<3;i++){
        auto *col = new QVBoxLayout;
        m_presetLbl[i] = new QLabel(QString("P%1").arg(i+1));
        m_presetLbl[i]->setAlignment(Qt::AlignCenter);
        m_presetLbl[i]->setStyleSheet("color:#585b70;font-size:10px;");

        m_presetLoad[i] = new QPushButton("▶");
        m_presetLoad[i]->setFixedHeight(22);
        m_presetLoad[i]->setEnabled(false);
        m_presetLoad[i]->setToolTip(QString("Appliquer preset %1").arg(i+1));
        m_presetLoad[i]->setStyleSheet(
            "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;"
            "border-radius:4px;font-weight:bold;font-size:11px;}"
            "QPushButton:hover{background:#b4d0fa;}"
            "QPushButton:disabled{background:#313244;color:#585b70;}");

        m_presetSave[i] = new QPushButton("💾");
        m_presetSave[i]->setFixedHeight(22);
        m_presetSave[i]->setToolTip(QString("Sauvegarder preset %1").arg(i+1));
        m_presetSave[i]->setStyleSheet(
            "QPushButton{background:#313244;color:#a6adc8;border:1px solid #45475a;"
            "border-radius:4px;font-size:11px;}"
            "QPushButton:hover{background:#45475a;}");

        col->addWidget(m_presetLbl[i]);
        col->addWidget(m_presetLoad[i]);
        col->addWidget(m_presetSave[i]);
        root->addLayout(col);

        connect(m_presetLoad[i],&QPushButton::clicked,this,[this,i]{ onPresetLoad(i); });
        connect(m_presetSave[i],&QPushButton::clicked,this,[this,i]{ onPresetSave(i); });
    }
    return gb;
}

// ── Section Firmware (deroulant discret) ──────────────────────────────────────
QWidget* ControlPanel::buildFirmwareSection()
{
    auto *container = new QWidget;
    auto *vl = new QVBoxLayout(container);
    vl->setContentsMargins(0,0,0,0);
    vl->setSpacing(0);

    // En-tete cliquable
    auto *header = new QWidget;
    header->setStyleSheet(
        "QWidget{background:#24243e;border-radius:5px;}"
        "QWidget:hover{background:#313244;}");
    header->setFixedHeight(28);
    header->setCursor(Qt::PointingHandCursor);
    auto *hl = new QHBoxLayout(header);
    hl->setContentsMargins(8,0,8,0);

    m_fwVersionLbl = new QLabel("Firmware");
    m_fwVersionLbl->setStyleSheet("color:#a6adc8;font-size:11px;");
    m_fwToggleBtn  = new QPushButton("▶");
    m_fwToggleBtn->setFixedSize(20,20);
    m_fwToggleBtn->setStyleSheet(
        "QPushButton{background:transparent;color:#585b70;border:none;font-size:10px;}"
        "QPushButton:hover{color:#89b4fa;}");
    hl->addWidget(m_fwVersionLbl,1);
    hl->addWidget(m_fwToggleBtn);
    vl->addWidget(header);

    // Panneau repliable
    m_firmwarePanel = new QWidget;
    m_firmwarePanel->setVisible(false);
    m_firmwarePanel->setStyleSheet("background:#1e1e2e;border:1px solid #313244;border-radius:5px;");
    auto *fl = new QVBoxLayout(m_firmwarePanel);
    fl->setContentsMargins(8,6,8,6);
    fl->setSpacing(4);

    auto *info = new QLabel(
        "1. Telechargez le firmware depuis obsbot.com\n"
        "2. Selectionnez le fichier .bin\n"
        "3. Cliquez Mettre a jour");
    info->setStyleSheet("color:#585b70;font-size:10px;");
    fl->addWidget(info);

    auto *dlBtn = new QPushButton("Telecharger (obsbot.com)");
    dlBtn->setStyleSheet("font-size:10px;color:#89b4fa;");
    connect(dlBtn,&QPushButton::clicked,this,[]{
        QDesktopServices::openUrl(QUrl("https://www.obsbot.com/download/obsbot-tiny-2-lite"));
    });
    fl->addWidget(dlBtn);

    auto *fileRow = new QHBoxLayout;
    m_fwFileLbl = new QLabel("Aucun fichier");
    m_fwFileLbl->setStyleSheet("color:#585b70;font-size:10px;");
    auto *browseBtn = new QPushButton("Parcourir");
    browseBtn->setFixedHeight(24);
    browseBtn->setStyleSheet("font-size:10px;");
    connect(browseBtn,&QPushButton::clicked,this,&ControlPanel::onFirmwareBrowse);
    fileRow->addWidget(m_fwFileLbl,1); fileRow->addWidget(browseBtn);
    fl->addLayout(fileRow);

    m_fwUpgradeBtn = new QPushButton("Mettre a jour le firmware");
    m_fwUpgradeBtn->setFixedHeight(28);
    m_fwUpgradeBtn->setEnabled(false);
    m_fwUpgradeBtn->setStyleSheet(
        "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;border-radius:5px;"
        "font-weight:bold;font-size:11px;}"
        "QPushButton:disabled{background:#313244;color:#585b70;}");
    connect(m_fwUpgradeBtn,&QPushButton::clicked,this,&ControlPanel::onFirmwareUpgrade);
    fl->addWidget(m_fwUpgradeBtn);

    vl->addWidget(m_firmwarePanel);

    auto toggleFn = [this]{
        m_firmwareExpanded = !m_firmwareExpanded;
        m_firmwarePanel->setVisible(m_firmwareExpanded);
        m_fwToggleBtn->setText(m_firmwareExpanded ? "▼" : "▶");
        // Afficher la version si on a un device
        if (!m_firmwareExpanded && DeviceManager::instance().isConnected())
            m_fwVersionLbl->setText(
                "Firmware v" + DeviceManager::instance().deviceVersion());
    };
    connect(m_fwToggleBtn, &QPushButton::clicked, this, &ControlPanel::onFirmwareToggle);

    return container;
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void ControlPanel::onReset()   { DeviceManager::instance().gimbalReset(); }
void ControlPanel::onFaceTarget()
{
    DeviceManager::instance().setFaceFocus(true);
    DeviceManager::instance().setAiMode(Device::AiWorkModeHuman,Device::AiSubModeCloseUp);
}

void ControlPanel::onZoomChanged(int v)
{
    m_zoomLbl->setText(QString("%1x").arg(v/100.0,0,'f',1));
    DeviceManager::instance().setZoom(v/100.0);
    // Empêcher l'IA de dezoom immédiatement après un zoom manuel
    if (m_aiActive)
        DeviceManager::instance().setAiAutoZoom(false);
}

void ControlPanel::onFovChanged(int)
{ DeviceManager::instance().setFov(
    static_cast<Device::FovType>(m_fovCombo->currentData().toInt())); }

void ControlPanel::onAiToggle()
{
    if (!m_aiActive) {
        DeviceManager::instance().setFaceFocus(m_faceFocusCb->isChecked());
        DeviceManager::instance().setAiAutoZoom(!m_zoomLockCb->isChecked());
        int idx = m_aiModeCombo->currentIndex();
        switch(idx){
            case 0: m_savedMode=Device::AiWorkModeHuman;m_savedSub=Device::AiSubModeNormal;    break;
            case 1: m_savedMode=Device::AiWorkModeHuman;m_savedSub=Device::AiSubModeUpperBody; break;
            case 2: m_savedMode=Device::AiWorkModeHuman;m_savedSub=Device::AiSubModeCloseUp;   break;
            case 3: m_savedMode=Device::AiWorkModeGroup;m_savedSub=Device::AiSubModeNormal;    break;
            case 4: m_savedMode=Device::AiWorkModeHand; m_savedSub=Device::AiSubModeNormal;    break;
        }
        DeviceManager::instance().setAiMode(m_savedMode,m_savedSub);
        m_aiActive=true; m_aiSuspended=false;
        m_aiBtn->setText("Desact. IA");
        m_aiBtn->setStyleSheet(
            "QPushButton{background:#f38ba8;color:#1e1e2e;border:none;"
            "border-radius:5px;font-weight:bold;font-size:11px;padding:2px 10px;}");
        m_aiStatusLbl->setText("IA active");
        m_aiStatusLbl->setStyleSheet("color:#a6e3a1;font-size:10px;font-weight:bold;");
    } else {
        DeviceManager::instance().cancelAiMode();
        DeviceManager::instance().setAiAutoZoom(true);
        m_aiActive=false; m_aiSuspended=false;
        m_aiBtn->setText("Activer IA");
        m_aiBtn->setStyleSheet(
            "QPushButton{background:#89b4fa;color:#1e1e2e;border:none;"
            "border-radius:5px;font-weight:bold;font-size:11px;padding:2px 10px;}"
            "QPushButton:hover{background:#b4d0fa;}"
            "QPushButton:disabled{background:#313244;color:#585b70;}");
        m_aiStatusLbl->setText("IA inactive");
        m_aiStatusLbl->setStyleSheet("color:#585b70;font-size:10px;font-style:italic;");
    }
}

void ControlPanel::onHdrChanged(int)
{ DeviceManager::instance().setHdr(m_hdrCombo->currentData().toInt()); }

void ControlPanel::onWbChanged(int)
{
    auto wb=static_cast<Device::DevWhiteBalanceType>(m_wbCombo->currentData().toInt());
    m_wbTempSlider->setEnabled(wb==Device::DevWhiteBalanceManual);
    DeviceManager::instance().setWhiteBalance(wb,m_wbTempSlider->value());
}

void ControlPanel::onWbTempChanged(int v)
{
    m_wbTempLbl->setText(QString("%1K").arg(v));
    if (m_wbCombo->currentData().toInt()==int(Device::DevWhiteBalanceManual))
        DeviceManager::instance().setWhiteBalance(Device::DevWhiteBalanceManual,v);
}

void ControlPanel::onFocusChanged(int v)
{
    m_focusLbl->setText(QString::number(v));
    DeviceManager::instance().setFocusAbsolute(v);
}

void ControlPanel::onPresetLoad(int id) { PresetManager::instance().applyToDevice(id); }
void ControlPanel::onPresetSave(int id)
{
    bool ok;
    QString name=QInputDialog::getText(this,"Preset","Nom:",QLineEdit::Normal,
        QString("Preset %1").arg(id+1),&ok);
    if (ok) PresetManager::instance().captureFromDevice(id,name);
}

void ControlPanel::onFirmwareToggle()
{
    m_firmwareExpanded=!m_firmwareExpanded;
    m_firmwarePanel->setVisible(m_firmwareExpanded);
    m_fwToggleBtn->setText(m_firmwareExpanded?"▼":"▶");
}

void ControlPanel::onFirmwareBrowse()
{
    QString f=QFileDialog::getOpenFileName(this,"Firmware",
        QDir::homePath(),"Firmware (*.bin);;Tous (*)");
    if (f.isEmpty()) return;
    m_fwFilePath=f;
    m_fwFileLbl->setText(QFileInfo(f).fileName());
    m_fwUpgradeBtn->setEnabled(true);
}

void ControlPanel::onFirmwareUpgrade()
{
    FirmwareManager::instance().upgradeFromFile(m_fwFilePath);
    m_fwUpgradeBtn->setEnabled(false);
}

void ControlPanel::onGestureChanged(int g,bool e)
{ DeviceManager::instance().setGesture(g,e); }

void ControlPanel::onStatusUpdated(Device::CameraStatus status)
{
    int zv=100+int(status.tiny.zoom_ratio*3);
    zv=qBound(100,zv,400);
    if (!m_zoom->isSliderDown()){
        m_zoom->blockSignals(true);
        m_zoom->setValue(zv);
        m_zoomLbl->setText(QString("%1x").arg(zv/100.0,0,'f',1));
        m_zoom->blockSignals(false);
    }
    if (DeviceManager::instance().isConnected() && !m_firmwareExpanded)
        m_fwVersionLbl->setText("Firmware v"+DeviceManager::instance().deviceVersion());

    // Le firmware peut activer l'IA tout seul → on force OFF si l'utilisateur ne l'a pas activée
    if (!m_aiActive && status.tiny.ai_mode != 0) {
        DeviceManager::instance().cancelAiMode();
        DeviceManager::instance().setAiAutoZoom(false);
    }

    // Retour à la position d'accueil si le suivi est perdu pendant ~3 secondes
    if (m_aiActive && !m_aiSuspended) {
        if (status.tiny.ai_target == 0) {
            m_noTargetFrames++;
            if (m_noTargetFrames == 30) {
                DeviceManager::instance().gimbalSetAngle(m_homeYaw, m_homePitch);
                DeviceManager::instance().setZoom(double(m_homeZoom));
                m_aiStatusLbl->setText("Suivi perdu → position d'accueil");
                m_aiStatusLbl->setStyleSheet("color:#f38ba8;font-size:10px;");
            }
        } else {
            m_noTargetFrames = 0;
        }
    } else {
        m_noTargetFrames = 0;
    }
}

void ControlPanel::setEnabled(bool e)
{
    QWidget::setEnabled(e);
    if (e) {
        // Connexion : s'assurer que le firmware n'a pas activé l'IA en douce
        DeviceManager::instance().cancelAiMode();
        DeviceManager::instance().setAiAutoZoom(false);
    } else {
        m_aiActive       = false;
        m_aiSuspended    = false;
        m_noTargetFrames = 0;
        m_aiStatusLbl->setText("Camera non connectee");
        m_fwVersionLbl->setText("Firmware — non connecte");
    }
}
