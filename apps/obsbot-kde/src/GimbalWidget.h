#pragma once
#include <QWidget>
class QSlider; class QLabel; class QPushButton; class QDial;

class GimbalWidget : public QWidget {
    Q_OBJECT
public:
    explicit GimbalWidget(QWidget *parent = nullptr);
public slots:
    void setEnabled(bool e);
private slots:
    void onPanChanged(int v);
    void onTiltChanged(int v);
    void onZoomChanged(int v);
    void onReset();
    void onSpeedMode(bool checked);
private:
    void buildUi();
    QSlider *m_pan, *m_tilt, *m_zoom;
    QLabel  *m_panLbl, *m_tiltLbl, *m_zoomLbl;
    QPushButton *m_resetBtn, *m_speedBtn;
    bool m_continuousMode = false;
};
