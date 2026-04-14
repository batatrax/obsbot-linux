#!/bin/sh
# Install udev rule for OBSBOT USB access / Installer la règle udev pour l'accès USB OBSBOT
install -m 0644 /usr/share/obsbot-linux/99-obsbot.rules /etc/udev/rules.d/
udevadm control --reload-rules 2>/dev/null || true
udevadm trigger 2>/dev/null || true
