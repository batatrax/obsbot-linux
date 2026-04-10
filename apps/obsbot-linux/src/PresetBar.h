/**
 * @file PresetBar.h
 * @brief Barre de presets toujours visible (3 emplacements).
 *
 * Chaque preset sauvegarde la configuration complete :
 * position gimbal, zoom, mode IA, focus, FOV, balance des blancs.
 *
 * Les presets sont persistes dans :
 *   ~/.local/share/obsbot-linux/presets.json
 *
 * Boutons par preset :
 *   - Appliquer : rappelle la configuration sauvegardee
 *   - Sauver    : capture la configuration actuelle
 */
#pragma once
#include <QWidget>
class QPushButton; class QLabel;

class PresetBar : public QWidget {
    Q_OBJECT
public:
    explicit PresetBar(QWidget *parent = nullptr);
public slots:
    void setEnabled(bool e);
    void refresh();
private slots:
    void onLoad(int id);
    void onSave(int id);
private:
    void buildUi();
    QPushButton *m_loadBtn[3] = {};
    QPushButton *m_saveBtn[3] = {};
    QLabel      *m_nameLbl[3] = {};
};
