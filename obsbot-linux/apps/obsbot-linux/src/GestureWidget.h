#pragma once
#include <QWidget>
class QCheckBox; class QLabel;
class GestureWidget : public QWidget {
    Q_OBJECT
public:
    explicit GestureWidget(QWidget *parent = nullptr);
public slots:
    void setEnabled(bool e);
private:
    void buildUi();
    QCheckBox *m_gestures[3];
};
