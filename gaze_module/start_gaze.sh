#!/usr/bin/env bash
set -e

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
GAZE_PYTHON="$PROJECT_DIR/Electronic_Shawon/.venv-mac/bin/python"

if [ ! -x "$GAZE_PYTHON" ]; then
  echo "Gaze Python environment not found: $GAZE_PYTHON" >&2
  echo "Create Electronic_Shawon/.venv-mac and install its requirements first." >&2
  exit 1
fi

cd "$PROJECT_DIR"
exec "$GAZE_PYTHON" -m gaze_module.gaze_keyboard "$@"
