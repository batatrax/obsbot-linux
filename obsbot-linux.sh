#!/bin/bash
# Lance OBSBOT Linux sans messages parasites MangOHUD/XNVCtrl
export MANGOHUD=0
export MANGOHUD_CONFIG=""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY="$SCRIPT_DIR/build/apps/obsbot-linux/obsbot-linux"
[ ! -f "$BINARY" ] && BINARY="$(find "$SCRIPT_DIR" -name 'obsbot-linux' -type f 2>/dev/null | head -1)"
[ ! -f "$BINARY" ] && echo "Erreur: obsbot-linux introuvable" && exit 1

exec "$BINARY" "$@" 2> >(grep -v -E "MANGOHUD|XNVCtrl|Authorization required|nvctrl|overlay_params|ffmpeg" >&2)
