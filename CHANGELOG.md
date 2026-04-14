# Changelog / Journal des modifications

## [1.5.0] — 2026-04-14

### English

#### Virtual camera pipeline overhaul
- **I420 format** (yuv420p): ffmpeg now outputs I420, the native format of libwebrtc.
  NV12 and YUYV422 caused silent black frames in Firefox WebRTC — I420 is universally accepted.
- **`exclusive_caps=0`**: v4l2loopback now exposes separate OUTPUT (ffmpeg) and CAPTURE
  (WirePlumber/Firefox) interfaces, eliminating EBUSY conflicts.
- **Renamed virtual device**: `OBSBot Virtual` → `OBSBot Virtual Camera` for clarity
  in browser camera selectors.

#### Firefox integration
- **Auto-configured via package**: `media.webrtc.camera.allow-pipewire=true` set via
  Firefox enterprise policies (native deb + snap) — no manual `about:config` change needed.
- Documented manual setup for Firefox Flatpak.

#### Reliability
- **SIGTERM handler** in `main.cpp`: `pkill -f obsbot-linux` now properly terminates
  the child ffmpeg process via Qt destructor chain.
- **Orphan cleanup** at virtual cam startup: any lingering ffmpeg from a crashed
  previous session is killed before launching a new one.
- **Virtual device filter** in `startCamera()` and `startPipeWireBridge()`: prevents
  the app from accidentally selecting `/dev/video99` as the real camera.

#### Package improvements
- DEB/RPM post-install scripts now configure v4l2loopback boot parameters and
  Firefox PipeWire policy automatically.
- `.desktop` file `Exec` path corrected from hardcoded dev path to `obsbot-linux`.

---

### Français

#### Pipeline caméra virtuelle refondu
- **Format I420** (yuv420p) : ffmpeg sort maintenant en I420, format natif de libwebrtc.
  NV12 et YUYV422 produisaient un écran noir silencieux dans Firefox — I420 est universellement accepté.
- **`exclusive_caps=0`** : v4l2loopback expose désormais des interfaces OUTPUT (ffmpeg) et CAPTURE
  (WirePlumber/Firefox) séparées, éliminant les conflits EBUSY.
- **Renommage du périphérique virtuel** : `OBSBot Virtual` → `OBSBot Virtual Camera` pour plus
  de clarté dans les sélecteurs de caméra des navigateurs.

#### Intégration Firefox
- **Configuré automatiquement via le paquet** : `media.webrtc.camera.allow-pipewire=true` appliqué
  via les politiques Firefox entreprise (deb natif + snap) — aucun changement `about:config` nécessaire.
- Configuration manuelle documentée pour Firefox Flatpak.

#### Fiabilité
- **Gestionnaire SIGTERM** dans `main.cpp` : `pkill -f obsbot-linux` termine maintenant correctement
  le processus ffmpeg enfant via la chaîne de destructeurs Qt.
- **Nettoyage des orphelins** au démarrage de la cam virtuelle : tout ffmpeg restant d'une session
  précédente plantée est tué avant d'en lancer un nouveau.
- **Filtre périphérique virtuel** dans `startCamera()` et `startPipeWireBridge()` : empêche l'app
  de sélectionner accidentellement `/dev/video99` comme vraie caméra.

#### Améliorations du paquet
- Les scripts post-install DEB/RPM configurent maintenant automatiquement v4l2loopback au démarrage
  et la politique PipeWire Firefox.
- Chemin `Exec` du fichier `.desktop` corrigé (chemin absolu dev → `obsbot-linux`).

---

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
