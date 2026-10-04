#!/usr/bin/env bash
set -euo pipefail

# Main launcher for the Parkinson eye-gaze keyboard.
# Cursor control was intentionally removed; this starts only the calibrated
# camera keyboard and its Ability Chat cloud connection.
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GAZE_PYTHON="$PROJECT_DIR/Electronic_Shawon/.venv-mac/bin/python"

if [[ ! -x "$GAZE_PYTHON" ]]; then
  echo "Gaze Python environment not found: $GAZE_PYTHON" >&2
  echo "Install Electronic_Shawon/requirements.txt into .venv-mac first." >&2
  exit 1
fi

if [[ ! -f "$PROJECT_DIR/Electronic_Shawon/calibration.json" ]]; then
  echo "No saved calibration found; guided calibration will start." >&2
fi

cd "$PROJECT_DIR"
exec "$GAZE_PYTHON" -m gaze_module.gaze_keyboard "$@"
