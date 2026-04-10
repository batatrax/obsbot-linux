#pragma once
#include <QWidget>
#include <dev/dev.hpp>
class QComboBox; class QSlider; class QLabel; class QCheckBox; class QPushButton;

class CameraWidget : public QWidget {
    Q_OBJECT
public:
    explicit CameraWidget(QWidget *parent = nullptr);
public slots:
    void setEnabled(bool e);
    void onStatusUpdated(Device::CameraStatus status);
private slots:
    void onFovChanged(int i);
    void onHdrChanged(int i);
    void onWbChanged(int i);
    void onWbTempChanged(int v);
    void onFaceFocusToggled(bool c);
    void onFocusChanged(int v);
    void onWakeup(); void onSleep();
private:
    void buildUi();
    QComboBox *m_fovCombo, *m_hdrCombo, *m_wbCombo;
    QSlider *m_wbTempSlider, *m_focusSlider;
    QLabel  *m_wbTempLbl, *m_focusLbl;
    QCheckBox *m_faceFocusCb;
    QPushButton *m_wakeBtn, *m_sleepBtn;
};
