/**
 * @file ImageWidget.h
 * @brief Panneau de reglages image (HDR, balance des blancs, focus).
 *
 * Parametres :
 *   - HDR : desactive / 2-en-1
 *   - Balance des blancs : auto, manuel, preset (jour/nuage/tungstene...)
 *   - Temperature de couleur (2000K-10000K, mode manuel)
 *   - Focus manuel (0-100)
 */
#pragma once
#include <QWidget>
#include <dev/dev.hpp>
class QComboBox; class QSlider; class QLabel;

class ImageWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = nullptr);
public slots:
    void setEnabled(bool e);
    void onStatusUpdated(Device::CameraStatus status);
private slots:
    void onHdrChanged(int i);
    void onWbChanged(int i);
    void onWbTempChanged(int v);
    void onFocusChanged(int v);
private:
    void buildUi();
    QComboBox *m_hdrCombo     = nullptr;
    QComboBox *m_wbCombo      = nullptr;
    QSlider   *m_wbTempSlider = nullptr;
    QLabel    *m_wbTempLbl    = nullptr;
    QSlider   *m_focusSlider  = nullptr;
    QLabel    *m_focusLbl     = nullptr;
};
