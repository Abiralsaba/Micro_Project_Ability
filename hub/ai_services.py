"""Local Whisper transcription and Gemini text services for Ability Chat."""

from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import tempfile
from typing import Dict, Iterable, List, Optional
import urllib.error
import urllib.request


PROJECT_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_WHISPER_ROOT = (
    Path.home()
    / "Library"
    / "Application Support"
    / "NationX"
    / "whisper.cpp"
)
ALLOWED_AUDIO_SUFFIXES = {".webm", ".ogg", ".mp4", ".m4a", ".wav"}


class AIServiceError(RuntimeError):
    """A user-facing AI service failure without credential disclosure."""


def load_local_environment(path: Path = PROJECT_ROOT / ".env") -> None:
    """Load known local settings without overriding exported environment."""
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except OSError:
        return
    allowed = {
        "GEMINI_API_KEY",
        "GEMINI_MODEL",
        "WHISPER_CPP_ROOT",
        "WHISPER_MODEL",
        "FFMPEG_PATH",
    }
    for raw_line in lines:
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        if key not in allowed or key in os.environ:
            continue
        value = value.strip().strip('"').strip("'")
        os.environ[key] = value


load_local_environment()


class WhisperCppTranscriber:
    """Run the user's existing whisper.cpp binary and multilingual model."""

    def __init__(self) -> None:
        root = Path(os.getenv("WHISPER_CPP_ROOT", str(DEFAULT_WHISPER_ROOT)))
        self.root = root.expanduser()
        self.binary = self.root / "build" / "bin" / "whisper-cli"
        self.ffmpeg = Path(os.getenv("FFMPEG_PATH", "/opt/homebrew/bin/ffmpeg"))
        self.model = self._select_model(os.getenv("WHISPER_MODEL"))
        self.bengali_model = self.root / "models" / "ggml-bengali-medium-q4_0.bin"

    def _select_model(self, configured: Optional[str]) -> Path:
        if configured:
            return Path(configured).expanduser()
        candidates = (
            self.root / "models" / "ggml-large-v3-turbo.bin",
            self.root / "models" / "ggml-medium.bin",
            self.root / "models" / "ggml-bengali-medium.bin",
        )
        return next((path for path in candidates if path.is_file()), candidates[0])

    @property
    def available(self) -> bool:
        return (
            self.binary.is_file()
            and os.access(self.binary, os.X_OK)
            and self.model.is_file()
            and self.ffmpeg.is_file()
        )

    def status(self) -> Dict[str, object]:
        return {
            "available": self.available,
            "model": self.model.name if self.model.is_file() else None,
            "bengali_model": (
                self.bengali_model.name if self.bengali_model.is_file() else None
            ),
            "engine": "whisper.cpp",
            "languages": ["auto", "bn", "en"],
        }

    def model_for(self, language: str) -> Path:
        if language == "bn" and self.bengali_model.is_file():
            return self.bengali_model
        return self.model

    def transcribe(
        self,
        audio: bytes,
        suffix: str = ".webm",
        language: str = "auto",
    ) -> str:
        if not self.available:
            raise AIServiceError(
                "Local Whisper is unavailable. Check whisper.cpp, its model, and FFmpeg."
            )
        if not audio:
            raise AIServiceError("No microphone audio was received.")
        suffix = suffix.lower() if suffix.lower() in ALLOWED_AUDIO_SUFFIXES else ".webm"
        language = language if language in {"auto", "bn", "en"} else "auto"
        selected_model = self.model_for(language)

        with tempfile.TemporaryDirectory(prefix="ability-whisper-") as directory:
            temp_dir = Path(directory)
            source = temp_dir / f"source-upload{suffix}"
            wave = temp_dir / "recording.wav"
            source.write_bytes(audio)

            conversion = subprocess.run(
                [
                    str(self.ffmpeg),
                    "-nostdin",
                    "-hide_banner",
                    "-loglevel",
                    "error",
                    "-y",
                    "-i",
                    str(source),
                    "-ar",
                    "16000",
                    "-ac",
                    "1",
                    "-c:a",
                    "pcm_s16le",
                    str(wave),
                ],
                capture_output=True,
                text=True,
                timeout=60,
                check=False,
            )
            if conversion.returncode != 0 or not wave.is_file():
                raise AIServiceError("The recorded audio could not be decoded.")

            command = [
                str(self.binary),
                "--model",
                str(selected_model),
                "--file",
                str(wave),
                "--language",
                language,
                "--no-timestamps",
                "--no-prints",
                "--threads",
                "6",
            ]
            result = subprocess.run(
                command,
                capture_output=True,
                text=True,
                timeout=180,
                check=False,
                env={**os.environ, "GGML_METAL_LOG_LEVEL": "0"},
            )
            if result.returncode != 0:
                raise AIServiceError("Local Whisper could not transcribe this recording.")

            text = " ".join(line.strip() for line in result.stdout.splitlines() if line.strip())
            if not text:
                raise AIServiceError("No speech was detected. Please try again closer to the microphone.")
            return text


class GeminiClient:
    """Small REST client that keeps API credentials on the server."""

    ENDPOINT = (
        "https://generativelanguage.googleapis.com/v1beta/models/"
        "{model}:generateContent"
    )

    def __init__(self) -> None:
        self.api_key = os.getenv("GEMINI_API_KEY", "").strip()
        self.model = os.getenv("GEMINI_MODEL", "gemini-3.8-flash").strip()

    @property
    def available(self) -> bool:
        return bool(self.api_key)

    def status(self) -> Dict[str, object]:
        return {
            "available": self.available,
            "model": self.model,
        }

    @staticmethod
    def _history_contents(history: Iterable[Dict[str, str]]) -> List[Dict[str, object]]:
        contents = []
        for item in list(history)[-20:]:
            role = item.get("role")
            text = str(item.get("text", "")).strip()
            if role not in {"user", "model"} or not text:
                continue
            contents.append({"role": role, "parts": [{"text": text[:8000]}]})
        return contents

    def generate(
        self,
        message: str,
        history: Iterable[Dict[str, str]] = (),
        system_instruction: Optional[str] = None,
        json_output: bool = False,
    ) -> str:
        if not self.available:
            raise AIServiceError(
                "Gemini is not configured. Add GEMINI_API_KEY to the project's .env file."
            )
        message = message.strip()
        if not message:
            raise AIServiceError("Enter a message for Gemini.")

        contents = self._history_contents(history)
        contents.append({"role": "user", "parts": [{"text": message[:12000]}]})
        payload: Dict[str, object] = {
            "contents": contents,
            "generationConfig": {
                "temperature": 0.35 if json_output else 0.7,
                "maxOutputTokens": 2048,
            },
        }
        if json_output:
            payload["generationConfig"]["responseMimeType"] = "application/json"
        if system_instruction:
            payload["systemInstruction"] = {"parts": [{"text": system_instruction}]}

        request = urllib.request.Request(
            self.ENDPOINT.format(model=self.model),
            data=json.dumps(payload).encode("utf-8"),
            headers={
                "Content-Type": "application/json",
                "x-goog-api-key": self.api_key,
            },
            method="POST",
        )
        try:
            with urllib.request.urlopen(request, timeout=45) as response:
                data = json.loads(response.read().decode("utf-8"))
        except urllib.error.HTTPError as error:
            try:
                detail = json.loads(error.read().decode("utf-8"))
                message_text = detail.get("error", {}).get("message", "")
            except (UnicodeDecodeError, json.JSONDecodeError):
                message_text = ""
            if error.code in (401, 403):
                raise AIServiceError("Gemini rejected the API key. Replace it in .env.") from error
            raise AIServiceError(
                message_text[:240] or f"Gemini request failed with HTTP {error.code}."
            ) from error
        except (urllib.error.URLError, TimeoutError) as error:
            raise AIServiceError("Gemini could not be reached. Check the internet connection.") from error

        try:
            parts = data["candidates"][0]["content"]["parts"]
            output = "".join(part.get("text", "") for part in parts).strip()
        except (KeyError, IndexError, TypeError):
            output = ""
        if not output:
            raise AIServiceError("Gemini returned no text response.")
        return output
