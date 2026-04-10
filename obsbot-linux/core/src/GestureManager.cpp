#include "obsbot/GestureManager.h"
#include "obsbot/DeviceManager.h"

GestureManager &GestureManager::instance()
{ static GestureManager inst; return inst; }

GestureManager::GestureManager(QObject *parent) : QObject(parent) {}

void GestureManager::setEnabled(int gesture, bool enabled)
{
    if (gesture < 0 || gesture >= 4) return;
    m_states[gesture] = enabled;
    DeviceManager::instance().setGesture(gesture, enabled);
    emit gestureChanged(gesture, enabled);
}

bool GestureManager::isEnabled(int gesture) const
{
    if (gesture < 0 || gesture >= 4) return false;
    return m_states[gesture];
}
