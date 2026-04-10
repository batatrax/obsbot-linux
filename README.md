# OBSBOT Linux Controller

<div align="center">

![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux-green.svg)
![Qt](https://img.shields.io/badge/Qt-6-blue.svg)

**Contrôleur Linux natif pour les caméras OBSBOT** | **Native Linux controller for OBSBOT cameras**

[English](#english) | [Français](#français)

</div>

---

## Français

### Description
Contrôleur Linux complet pour la caméra OBSBOT Tiny 2 Lite, utilisant le SDK officiel OBSBOT.

Deux variantes disponibles :
- **obsbot-linux** — Qt6 pur, compatible GNOME, KDE, XFCE, i3...
- **obsbot-kde** — KDE Frameworks 6, notifications Plasma, icône système

### Fonctionnalités
- 🎥 **Contrôle Gimbal** — Pan/Tilt/Zoom avec commandes directes
- 🤖 **IA Tracking** — Suivi humain, groupe, main avec cadrage automatique
- 📌 **Presets** — 3 configurations sauvegardables (position + IA + caméra)
- ✋ **Gestes** — Gesture Control 2.0 (sélection, zoom, photo)
- ⚙ **Caméra** — FOV, HDR, Balance des blancs, Mise au point
- 🔄 **Firmware** — Mise à jour manuelle depuis fichier .bin
- 📺 **Prévisualisation** — Flux vidéo en direct via Qt Multimedia

### Installation

#### Prérequis
```bash
sudo apt install qt6-base-dev qt6-multimedia-dev \
    cmake build-essential ninja-build \
    gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-libav
```

Pour obsbot-kde uniquement :
```bash
sudo apt install libkf6-framework-dev extra-cmake-modules
```

#### Compilation
```bash
git clone https://github.com/obsbot-linux/obsbot-linux
cd obsbot-linux

# Intégrer le SDK OBSBOT (demandez-le à OBSBOT)
mkdir -p sdk
unzip libdev_v2_1_0_8.zip -d /tmp/sdk_tmp
cp -r /tmp/sdk_tmp/libdev_v2.1.0_8/* sdk/

# Compiler obsbot-linux uniquement
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_OBSBOT_KDE=OFF
cmake --build build

# Compiler les deux
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

#### Permissions USB
```bash
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="2bd9", MODE="0666", GROUP="plugdev"' \
    | sudo tee /etc/udev/rules.d/99-obsbot.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

---

## English

### Description
Full-featured Linux controller for the OBSBOT Tiny 2 Lite camera, using the official OBSBOT SDK.

Two variants:
- **obsbot-linux** — Pure Qt6, works on GNOME, KDE, XFCE, i3...
- **obsbot-kde** — KDE Frameworks 6, Plasma notifications, system tray icon

### Features
- 🎥 **Gimbal Control** — Pan/Tilt/Zoom with direct commands
- 🤖 **AI Tracking** — Human, group, hand tracking with auto framing
- 📌 **Presets** — 3 saveable configurations (position + AI + camera)
- ✋ **Gestures** — Gesture Control 2.0 (selection, zoom, photo)
- ⚙ **Camera** — FOV, HDR, White balance, Focus
- 🔄 **Firmware** — Manual update from .bin file
- 📺 **Preview** — Live video stream via Qt Multimedia

### License
GPL v3 — See [LICENSE](LICENSE)

### Contributing
PRs welcome! See [CONTRIBUTING.md](docs/CONTRIBUTING.md)
