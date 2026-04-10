/**
 * @file MainWindow.h
 * @brief Fenetre principale — panneau de controles.
 *
 * Layout :
 *   - Barre de statut (36px, fixe)
 *   - ControlPanel (scrollable, tout les controles)
 *
 * La video est dans VideoWindow (fenetre independante).
 * Fermeture : eteint la camera sans dialogue.
 */
#pragma once
#include <QMainWindow>
#include <dev/dev.hpp>

class QLabel;
class QPushButton;
class ControlPanel;
class VideoWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

private slots:
    void onDeviceConnected(const QString &sn);
    void onDeviceDisconnected(const QString &sn);
    void onStatusUpdated(Device::CameraStatus status);

private:
    void buildUi();
    void updateConnectionState(bool connected);

    QWidget      *m_statusBar  = nullptr;
    QLabel       *m_devLabel   = nullptr;
    QPushButton  *m_videoBtn   = nullptr;  // bouton afficher/masquer fenetre video
    ControlPanel *m_panel      = nullptr;
    VideoWindow  *m_videoWin   = nullptr;
};
