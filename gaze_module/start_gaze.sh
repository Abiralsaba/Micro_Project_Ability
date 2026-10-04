#!/usr/bin/env bash
set -euo pipefail

# Backwards-compatible path. The main launcher now lives at project/gaze.sh.
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "$PROJECT_DIR/gaze.sh" "$@"
