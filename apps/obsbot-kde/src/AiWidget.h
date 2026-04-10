#pragma once
#include <QWidget>
#include <dev/dev.hpp>
class QComboBox; class QPushButton; class QLabel;
class AiWidget : public QWidget {
    Q_OBJECT
public:
    explicit AiWidget(QWidget *parent = nullptr);
public slots:
    void setEnabled(bool e);
private slots:
    void onActivate(); void onCancel();
private:
    void buildUi();
    QComboBox *m_modeCombo, *m_subCombo;
    QPushButton *m_activateBtn, *m_cancelBtn;
    QLabel *m_statusLabel;
};
