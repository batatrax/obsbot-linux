#pragma once
#include <QObject>

class GestureManager : public QObject
{
    Q_OBJECT
public:
    static GestureManager &instance();
    void setEnabled(int gesture, bool enabled);
    bool isEnabled(int gesture) const;

signals:
    void gestureChanged(int gesture, bool enabled);

private:
    explicit GestureManager(QObject *parent = nullptr);
    bool m_states[4] = {false, false, false, false};
};
