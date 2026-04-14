#!/bin/sh

# ── udev rule — OBSBOT USB access without root ────────────────────────────────
install -m 0644 /usr/share/obsbot-linux/99-obsbot.rules /etc/udev/rules.d/
udevadm control --reload-rules 2>/dev/null || true
udevadm trigger 2>/dev/null || true

# ── v4l2loopback — create /dev/video99 at boot ───────────────────────────────
mkdir -p /etc/modprobe.d /etc/modules-load.d
echo 'options v4l2loopback video_nr=99 card_label="OBSBot Virtual Camera" exclusive_caps=0' \
    > /etc/modprobe.d/v4l2loopback.conf
echo 'v4l2loopback' > /etc/modules-load.d/v4l2loopback.conf

# ── Firefox — enable PipeWire camera backend ─────────────────────────────────
FIREFOX_POLICY='{
  "policies": {
    "Preferences": {
      "media.webrtc.camera.allow-pipewire": {
        "Value": true,
        "Status": "default"
      }
    }
  }
}'

if [ -f /usr/bin/firefox ] || [ -d /usr/lib/firefox ] || [ -d /usr/lib64/firefox ]; then
    mkdir -p /etc/firefox/policies
    printf '%s\n' "$FIREFOX_POLICY" > /etc/firefox/policies/policies.json
fi
