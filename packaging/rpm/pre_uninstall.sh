#!/bin/sh

# ── udev rule ─────────────────────────────────────────────────────────────────
rm -f /etc/udev/rules.d/99-obsbot.rules
udevadm control --reload-rules 2>/dev/null || true

# ── v4l2loopback ──────────────────────────────────────────────────────────────
rm -f /etc/modprobe.d/v4l2loopback.conf
rm -f /etc/modules-load.d/v4l2loopback.conf

# ── Firefox policies ──────────────────────────────────────────────────────────
if [ -f /etc/firefox/policies/policies.json ] && \
   grep -q "allow-pipewire" /etc/firefox/policies/policies.json 2>/dev/null; then
    rm -f /etc/firefox/policies/policies.json
    rmdir /etc/firefox/policies 2>/dev/null || true
    rmdir /etc/firefox 2>/dev/null || true
fi
