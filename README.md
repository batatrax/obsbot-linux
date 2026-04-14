# OBSBOT Linux Controller

<div align="center">

![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux-green.svg)
![Qt](https://img.shields.io/badge/Qt-6-blue.svg)
![Version](https://img.shields.io/badge/version-1.2.0-brightgreen.svg)

**The only native Linux controller for OBSBOT cameras**  
**Le seul contrôleur Linux natif pour les caméras OBSBOT**

[English](#english) | [Français](#français)

</div>

---

## English

### Why this project?

OBSBOT provides excellent cameras but no official Linux software. This project fills that gap with a full-featured native controller built on the official OBSBOT SDK — gimbal, AI tracking, virtual camera, firmware update, and more. All without Wine, all without a VM.

### Features

- 🎥 **Gimbal Control** — Pan/Tilt/Zoom with virtual joystick, keyboard shortcuts (WASD/arrows), and direct SDK commands
- 🤖 **AI Tracking** — Human, group, hand tracking with auto framing; AI can be suspended during manual control
- 📌 **Presets** — 3 saveable configurations (position + AI + camera settings)
- ✋ **Gestures** — Gesture Control 2.0 (selection, zoom, photo)
- ⚙ **Camera** — FOV, HDR, White balance, Focus, Quality selection (1080p/720p/4K, MJPEG/H264)
- 🔄 **Firmware** — Manual update from .bin file
- 📺 **Live Preview** — Independent always-on-top video window with HUD, mirror/flip, zoom overlay
- 📡 **Virtual Camera** — Share the OBSBOT stream to OBS, Firefox, Teams, Zoom, Chrome via `/dev/video99`
- 🏠 **Home Position** — Custom pan/tilt/zoom saved as home; camera returns automatically when AI loses its target

### Two variants

| | `obsbot-linux` | `obsbot-kde` |
|---|---|---|
| **Toolkit** | Qt6 pure | KDE Frameworks 6 |
| **Desktop** | GNOME, KDE, XFCE, i3, Sway... | KDE Plasma |
| **Extras** | — | System tray, Plasma notifications |

---

### Virtual Camera

The virtual camera feature shares your OBSBOT stream to any application that supports V4L2 or PipeWire (OBS, Firefox, Chrome, Teams, Zoom...).

**How it works:**
```
OBSBOT (/dev/video0)
  → ffmpeg (MJPEG → I420)
  → /dev/video99  (v4l2loopback)
  → WirePlumber   (PipeWire node)
  → Firefox / OBS / Teams / Chrome
```

**What the package installs automatically:**
- `v4l2loopback` module configured at boot (`/dev/video99`, `exclusive_caps=0`)
- Firefox policy enabling PipeWire camera backend (`media.webrtc.camera.allow-pipewire=true`)

**Manual setup (if not using the package):**
```bash
# 1. Install v4l2loopback
sudo apt install v4l2loopback-dkms ffmpeg

# 2. Load the module
sudo modprobe v4l2loopback video_nr=99 card_label='OBSBot Virtual Camera' exclusive_caps=0

# 3. Persist across reboots
echo 'options v4l2loopback video_nr=99 card_label="OBSBot Virtual Camera" exclusive_caps=0' \
    | sudo tee /etc/modprobe.d/v4l2loopback.conf
echo 'v4l2loopback' | sudo tee /etc/modules-load.d/v4l2loopback.conf

# 4. Enable PipeWire camera in Firefox (native deb)
sudo mkdir -p /etc/firefox/policies
sudo tee /etc/firefox/policies/policies.json << 'EOF'
{
  "policies": {
    "Preferences": {
      "media.webrtc.camera.allow-pipewire": { "Value": true, "Status": "default" }
    }
  }
}
EOF

# 5. Firefox Snap
sudo mkdir -p /var/snap/firefox/common/distribution
sudo cp /etc/firefox/policies/policies.json \
    /var/snap/firefox/common/distribution/policies.json
```

**Firefox Flatpak** — Policies are not supported. Set manually in `about:config`:  
`media.webrtc.camera.allow-pipewire` → `true`

---

### Installation

#### DEB / RPM packages (recommended)

```bash
# Ubuntu / Debian / Kubuntu
sudo apt install ./obsbot-linux_1.2.0_amd64.deb

# Fedora / openSUSE
sudo rpm -i obsbot-linux-1.2.0.x86_64.rpm
```

The package automatically installs:
- udev rule (USB access without root)
- v4l2loopback boot configuration
- Firefox PipeWire policy

Then plug in your OBSBOT and launch **OBSBOT Linux** from your application menu.

#### Build from source

**Prerequisites:**
```bash
# Ubuntu / Debian
sudo apt install qt6-base-dev qt6-multimedia-dev \
    libqt6opengl6-dev cmake build-essential ninja-build \
    v4l2loopback-dkms ffmpeg

# For obsbot-kde only:
sudo apt install libkf6-framework-dev extra-cmake-modules
```

**Build:**
```bash
git clone https://github.com/batatrax/obsbot-linux
cd obsbot-linux

# Add the OBSBOT SDK (request from OBSBOT or use your existing copy)
mkdir -p sdk
unzip libdev_v2_1_0_8.zip -d /tmp/sdk_tmp
cp -r /tmp/sdk_tmp/libdev_v2.1.0_8/* sdk/

# obsbot-linux only (recommended)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_OBSBOT_KDE=OFF
cmake --build build

# Both variants
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

#### Flatpak
```bash
# obsbot-linux
flatpak-builder --install --force-clean build-flatpak \
    flatpak/com.github.obsbot-linux.obsbot-linux.json

# obsbot-kde
flatpak-builder --install --force-clean build-flatpak \
    flatpak/com.github.obsbot-linux.obsbot-kde.json
```

---

### Credits

This project was built collaboratively by:

- **batatrax** — project owner, hardware testing, integration
- **[Claude](https://claude.ai) (Anthropic)** — architecture, C++ implementation, virtual camera pipeline, debugging
- **[Gemini](https://gemini.google.com) (Google)** — analysis, diagnostics, cross-validation
- **Boubou** (qwen2.5:7b via [Ollama](https://ollama.ai)) — local AI assistant, live testing, format experiments

> Four minds, one camera. The virtual camera pipeline (MJPEG → I420 → v4l2loopback → PipeWire → Firefox) was the result of deep collaboration between all four, involving kernel module analysis, WebRTC log inspection, and systematic format testing across multiple reboots.

### License

GPL v3 — See [LICENSE](LICENSE)

### Contributing

PRs welcome! See [CONTRIBUTING.md](docs/CONTRIBUTING.md)

---

## Français

### Pourquoi ce projet ?

OBSBOT propose d'excellentes caméras mais aucun logiciel Linux officiel. Ce projet comble ce manque avec un contrôleur natif complet basé sur le SDK officiel OBSBOT — gimbal, suivi IA, caméra virtuelle, mise à jour firmware, et bien plus. Sans Wine. Sans VM.

### Fonctionnalités

- 🎥 **Contrôle Gimbal** — Pan/Tilt/Zoom avec joystick virtuel, raccourcis clavier (ZQSD/flèches) et commandes SDK directes
- 🤖 **IA Tracking** — Suivi humain, groupe, main avec cadrage automatique ; l'IA se suspend pendant le contrôle manuel
- 📌 **Presets** — 3 configurations sauvegardables (position + IA + réglages caméra)
- ✋ **Gestes** — Gesture Control 2.0 (sélection, zoom, photo)
- ⚙ **Caméra** — FOV, HDR, Balance des blancs, Mise au point, sélection qualité (1080p/720p/4K, MJPEG/H264)
- 🔄 **Firmware** — Mise à jour manuelle depuis fichier .bin
- 📺 **Aperçu en direct** — Fenêtre vidéo indépendante always-on-top avec HUD, miroir/retournement, overlay zoom
- 📡 **Caméra virtuelle** — Partage le flux OBSBOT vers OBS, Firefox, Teams, Zoom, Chrome via `/dev/video99`
- 🏠 **Position d'accueil** — Pan/tilt/zoom personnalisés sauvegardés ; retour automatique quand l'IA perd sa cible

### Caméra virtuelle

La caméra virtuelle partage le flux OBSBOT vers toute application supportant V4L2 ou PipeWire.

**Installation manuelle (si pas via paquet) :**
```bash
sudo apt install v4l2loopback-dkms ffmpeg
sudo modprobe v4l2loopback video_nr=99 card_label='OBSBot Virtual Camera' exclusive_caps=0
echo 'options v4l2loopback video_nr=99 card_label="OBSBot Virtual Camera" exclusive_caps=0' \
    | sudo tee /etc/modprobe.d/v4l2loopback.conf
echo 'v4l2loopback' | sudo tee /etc/modules-load.d/v4l2loopback.conf
```

**Firefox (deb natif)** — configuré automatiquement par le paquet.  
**Firefox Flatpak** — dans `about:config` : `media.webrtc.camera.allow-pipewire` → `true`

### Installation

#### Paquets DEB / RPM (recommandé)

```bash
sudo apt install ./obsbot-linux_1.2.0_amd64.deb     # Ubuntu / Debian / Kubuntu
sudo rpm -i obsbot-linux-1.2.0.x86_64.rpm           # Fedora / openSUSE
```

Le paquet installe automatiquement la règle udev, la configuration v4l2loopback et la politique Firefox. Branchez la caméra et lancez **OBSBOT Linux** depuis le menu des applications.

### Crédits

Ce projet est né d'une collaboration entre :

- **batatrax** — porteur du projet, tests matériels, intégration
- **[Claude](https://claude.ai) (Anthropic)** — architecture, implémentation C++, pipeline caméra virtuelle, débogage
- **[Gemini](https://gemini.google.com) (Google)** — analyse, diagnostics, validation croisée
- **Boubou** (qwen2.5:7b via [Ollama](https://ollama.ai)) — IA locale, tests en direct, expérimentations de formats

### Licence

GPL v3 — Voir [LICENSE](LICENSE)
