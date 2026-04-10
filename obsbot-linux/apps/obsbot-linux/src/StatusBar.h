#pragma once
#include <QWidget>
class QLabel;

class StatusBar : public QWidget {
    Q_OBJECT
public:
    explicit StatusBar(QWidget *parent = nullptr);
    void setStatus(bool connected, const QString &name, const QString &version);
private:
    QLabel *m_dot, *m_name, *m_version;
};
