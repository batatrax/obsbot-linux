# CLAUDE.md — obsbot-linux

## Présentation du projet
Application Linux de contrôle de la caméra **OBSBOT Tiny 2 Lite** via USB.
Interface Qt6 avec panneau de contrôle et fenêtre vidéo indépendante.

## Structure du dépôt
```
/home/batatrax/Documents/obsbot-linux/
├── apps/obsbot-linux/src/   ← SOURCES RÉELLES compilées par cmake
├── core/                    ← Wrapper SDK (DeviceManager, PresetManager, etc.)
├── sdk/                     ← SDK OBSBOT natif (headers + libs)
├── build/                   ← Répertoire de build cmake
└── obsbot-linux/            ← Copie modifiée par Gemini (NON compilée, ignorer)
```

> ⚠️ Ne jamais éditer `obsbot-linux/apps/...` — cmake utilise `apps/obsbot-linux/src/`

## Commandes utiles
```bash
# Build
cd /home/batatrax/Documents/obsbot-linux/build && cmake --build . --target obsbot-linux

# Lancer
cd /home/batatrax/Documents/obsbot-linux/build/apps/obsbot-linux && ./obsbot-linux &

# Redémarrer
pkill -f obsbot-linux; sleep 1 && ./obsbot-linux &
```

## Fichiers clés
| Fichier | Rôle |
|---|---|
| `apps/obsbot-linux/src/MainWindow.cpp` | Fenêtre principale, connexions signaux |
| `apps/obsbot-linux/src/ControlPanel.cpp` | Tous les contrôles (gimbal, IA, image, presets, firmware) |
| `apps/obsbot-linux/src/VideoWindow.cpp` | Fenêtre vidéo indépendante avec HUD |
| `core/src/DeviceManager.cpp` | Wrapper SDK — toutes les commandes caméra |
| `core/include/obsbot/DeviceManager.h` | API DeviceManager |

## Architecture
- **ControlPanel** : panneau scrollable monolithique (pas d'onglets — exprès)
- **VideoWindow** : fenêtre séparée, always-on-top, 480×320
- **JoystickWidget** : contrôle pan/tilt avec suspension IA intégrée
- **DeviceManager** : singleton Qt wrappant le SDK OBSBOT

## SDK — champs CameraStatus utiles
```cpp
status.tiny.zoom_ratio   // zoom 0-100 → 1x-4x
status.tiny.ai_target    // non-zero = cible trackée
status.tiny.ai_mode      // mode IA actif (0 = off)
```

## Comportements firmware connus
- Le firmware OBSBOT peut activer l'IA tout seul → on force `cancelAiMode()` à la connexion et en continu dans `onStatusUpdated`
- Quand le suivi est perdu, la caméra "cherche" en balayant (comportement natif) → notre app retourne à la position d'accueil après 3s
- L'IA peut dezoom automatiquement → on appelle `setAiAutoZoom(false)` lors d'un zoom manuel

## Fonctionnalités implémentées
- Contrôle gimbal : joystick virtuel, ZQSD/flèches, reset
- Zoom : slider 1x-4x synchronisé avec le statut caméra
- IA : activation/désactivation, suspension automatique pendant contrôle manuel
- **Position d'accueil** (bouton 🏠) : mémorise pan/tilt/zoom via `aiGetGimbalStateR()`, retour auto si suivi perdu ~3s
- Presets : 3 slots save/load
- Photo : capture JPEG dans ~/Images
- Firmware : mise à jour depuis fichier .bin
- **Caméra virtuelle** (bouton "Cam Virtuelle") : ffmpeg → /dev/video99 via v4l2loopback (YUYV422 1080p30), partage le flux avec OBS/Firefox/Teams. Vérification automatique de `/dev/video99` et `ffmpeg` avec message d'aide si absent
- **Miroir horizontal** (bouton ⟺ dans HUD vidéo) : flip H du preview et des captures. Mémorisé via QSettings
- **Retournement vertical** (bouton ⇅ dans HUD vidéo) : flip V du preview et des captures. Mémorisé via QSettings
- **Overlay zoom** : affichage du zoom actuel (ex: `2.3×`) en bas à gauche du preview, mis à jour en temps réel depuis le status SDK. Visible uniquement quand la caméra est active

## Problèmes connus / limites
- `PresetManager::captureFromDevice()` ne lit pas les angles actuels (TODO)
- La caméra virtuelle nécessite `v4l2loopback` chargé avec `video_nr=99` (message d'aide intégré si absent)
- La qualité vidéo applique `setCameraFormat()` avant `start()` — fonctionne sur V4L2 Linux, comportement peut varier selon le backend Qt

## QSettings persistants (obsbot-linux / VideoWindow)
- `mirror/enabled` (bool, défaut: true) — flip horizontal
- `flip/vertical`  (bool, défaut: false) — retournement vertical

## Historique commits importants
- `94b880c` : release initiale v1.0.0
- `56b4bc7` : fix IA vs contrôle manuel + position d'accueil personnalisée
