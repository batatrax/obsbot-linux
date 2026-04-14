# Changelog / Journal des modifications

## [1.1.0] — 2026-04-14

### English

#### New features
- **Virtual camera** (`/dev/video99` via v4l2loopback + ffmpeg): share the OBSBOT stream in OBS, Firefox, Teams, etc.
- **Independent video window**: always-on-top preview with HUD controls, auto-positioned to the right of the control panel
- **Horizontal mirror / vertical flip**: persistent via QSettings (saved per session)
- **Zoom overlay**: real-time zoom indicator (e.g. `2.3×`) in the video window corner
- **Custom home position**: save pan/tilt/zoom as home; camera returns here automatically when AI loses its target for ~3 s

#### Improvements
- Full English UI — all user-visible strings translated
- Bilingual DEB and RPM packages (English + French package descriptions)
- udev rule (`99-obsbot.rules`) installed automatically by DEB/RPM packages
- Firmware section collapsed by default to reduce panel height
- Image + Presets sections displayed side-by-side to reduce scrolling
- Joystick suspends AI tracking during manual control, resumes 0.5 s after release

#### Bug fixes
- AI mode forced off at connection (firmware can activate it silently)
- Zoom lock prevents firmware from overriding manual zoom
- `_exit(0)` on close avoids hang caused by OBSBOT SDK native threads

---

### Français

#### Nouvelles fonctionnalités
- **Caméra virtuelle** (`/dev/video99` via v4l2loopback + ffmpeg) : partage le flux dans OBS, Firefox, Teams, etc.
- **Fenêtre vidéo indépendante** : aperçu always-on-top avec HUD, positionnée automatiquement à droite du panneau
- **Miroir horizontal / retournement vertical** : persistants via QSettings
- **Overlay zoom** : indicateur de zoom en temps réel (ex : `2.3×`) dans le coin de la fenêtre vidéo
- **Position d'accueil personnalisée** : mémorise pan/tilt/zoom ; la caméra revient ici automatiquement si l'IA perd sa cible pendant ~3 s

#### Améliorations
- Interface entièrement en anglais — toutes les chaînes utilisateur traduites
- Paquets DEB et RPM bilingues (descriptions en anglais + français)
- Règle udev (`99-obsbot.rules`) installée automatiquement par les paquets DEB/RPM
- Section Firmware réduite par défaut pour diminuer la hauteur du panneau
- Sections Image et Presets côte à côte pour réduire le défilement
- Le joystick suspend le suivi IA pendant le contrôle manuel, reprend 0,5 s après relâchement

#### Corrections
- Mode IA forcé OFF à la connexion (le firmware peut l'activer silencieusement)
- Lock zoom empêche le firmware d'écraser le zoom manuel
- `_exit(0)` à la fermeture évite le blocage dû aux threads natifs du SDK OBSBOT

---

## [1.0.0] — 2025-xx-xx

Initial release / Version initiale.
