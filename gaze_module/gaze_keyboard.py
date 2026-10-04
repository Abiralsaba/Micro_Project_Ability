#!/usr/bin/env python3
"""Parkinson-friendly eye-gaze keyboard connected to Ability Chat."""

from __future__ import annotations

import argparse
from collections import deque
from dataclasses import dataclass, replace
from pathlib import Path
from statistics import median
import sys
import time
from typing import Deque, Dict, Optional, Tuple


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = PROJECT_ROOT / "Electronic_Shawon"
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))
if str(SOURCE_ROOT) not in sys.path:
    sys.path.insert(0, str(SOURCE_ROOT))

from camera_app import (  # noqa: E402
    DEFAULT_MODEL,
    FaceTracker,
    GuidedCalibration,
    draw_interface,
    draw_text,
    ensure_model,
    iris_gaze_score,
    load_calibration,
    save_calibration,
)
from camera_control import (  # noqa: E402
    CameraController,
    Mode,
    VisionSample,
)
from gaze_module.ability_cloud import AbilityGazeCloud  # noqa: E402


DEFAULT_CALIBRATION = SOURCE_ROOT / "calibration.json"
TARGETS = {
    "TO ABIR": "glove_01",
    "TO BRAILLE": "braille_01",
    "TO VOICE": "voice_01",
}
TARGET_LABELS = {device: label[3:] for label, device in TARGETS.items()}


class MedianGazeSmoother:
    """Reject short gaze spikes while preserving the calibrated scale."""

    def __init__(self, window_size: int = 7) -> None:
        if window_size < 1 or window_size % 2 == 0:
            raise ValueError("Smoothing window must be a positive odd number")
        self._scores: Deque[float] = deque(maxlen=window_size)

    def update(self, sample: VisionSample) -> VisionSample:
        if not sample.face_present:
            self._scores.clear()
            return sample
        self._scores.append(sample.gaze_score)
        return replace(sample, gaze_score=median(self._scores))


def iris_vertical_position(landmarks) -> float:
    """Return iris height inside the eyelids: 0 is up and 1 is down."""
    positions = []
    # MediaPipe right iris/upper/lower, then left iris/upper/lower.
    for iris_index, upper_index, lower_index in (
        (468, 159, 145),
        (473, 386, 374),
    ):
        top_y, bottom_y = sorted(
            (landmarks[upper_index].y, landmarks[lower_index].y)
        )
        eye_height = bottom_y - top_y
        if eye_height <= 1e-6:
            raise ValueError("Eye landmarks are too close")
        positions.append((landmarks[iris_index].y - top_y) / eye_height)
    return sum(positions) / len(positions)


class CursorFaceTracker(FaceTracker):
    """Face tracker that preserves the vertical iris value for cursor motion."""

    def __init__(self, model_path: Path):
        super().__init__(model_path)
        self.vertical_position: Optional[float] = None

    def detect(self, bgr_frame, timestamp: float) -> VisionSample:
        import cv2

        timestamp_ms = max(int(timestamp * 1000), self._last_timestamp_ms + 1)
        self._last_timestamp_ms = timestamp_ms
        rgb = cv2.cvtColor(bgr_frame, cv2.COLOR_BGR2RGB)
        image = self._mp.Image(image_format=self._mp.ImageFormat.SRGB, data=rgb)
        result = self._landmarker.detect_for_video(image, timestamp_ms)
        self.vertical_position = None

        if not result.face_landmarks or not result.face_blendshapes:
            return VisionSample(timestamp=timestamp, face_present=False)

        scores: Dict[str, float] = {
            item.category_name: float(item.score)
            for item in result.face_blendshapes[0]
        }
        if "eyeBlinkLeft" not in scores or "eyeBlinkRight" not in scores:
            return VisionSample(timestamp=timestamp, face_present=False)

        landmarks = result.face_landmarks[0]
        if len(landmarks) < 474:
            return VisionSample(timestamp=timestamp, face_present=False)
        try:
            gaze_score = iris_gaze_score(landmarks)
            self.vertical_position = iris_vertical_position(landmarks)
        except ValueError:
            return VisionSample(timestamp=timestamp, face_present=False)

        return VisionSample(
            timestamp=timestamp,
            face_present=True,
            gaze_score=gaze_score,
            blink_score=min(scores["eyeBlinkLeft"], scores["eyeBlinkRight"]),
        )


@dataclass(frozen=True)
class CursorPosition:
    x: float
    y: float
    selecting: bool = False


class GazeCursorMapper:
    """Map the existing calibration and live iris height to screen space."""

    VERTICAL_TOP = 0.20
    VERTICAL_BOTTOM = 0.80

    def __init__(self, calibration, window_size: int = 7) -> None:
        self.calibration = calibration
        self._x: Deque[float] = deque(maxlen=window_size)
        self._y: Deque[float] = deque(maxlen=window_size)
        self._last: Optional[CursorPosition] = None

    def reset(self) -> None:
        self._x.clear()
        self._y.clear()
        self._last = None

    def update(
        self,
        sample: VisionSample,
        vertical_position: Optional[float],
    ) -> Optional[CursorPosition]:
        if not sample.face_present:
            self.reset()
            return None

        selecting = sample.blink_score >= self.calibration.blink_threshold
        # Closing the eyes distorts the iris/eyelid geometry. Keep the last
        # stable point so a calibrated blink clicks what the user was viewing.
        if selecting and self._last is not None:
            return replace(self._last, selecting=True)

        corrected = sample.gaze_score * self.calibration.gaze_direction
        neutral = (
            self.calibration.left_threshold
            + self.calibration.right_threshold
        ) / 2.0
        left_extent = 2.0 * self.calibration.left_threshold - neutral
        right_extent = 2.0 * self.calibration.right_threshold - neutral
        horizontal_span = max(right_extent - left_extent, 1e-6)
        x = (corrected - left_extent) / horizontal_span

        if vertical_position is None:
            y = self._last.y if self._last is not None else 0.5
        else:
            y = (
                vertical_position - self.VERTICAL_TOP
            ) / (self.VERTICAL_BOTTOM - self.VERTICAL_TOP)

        self._x.append(max(0.0, min(1.0, x)))
        self._y.append(max(0.0, min(1.0, y)))
        self._last = CursorPosition(
            x=median(self._x),
            y=median(self._y),
            selecting=selecting,
        )
        return self._last


class ParkinsonKeyboardController(CameraController):
    """Communication-only keyboard with eye-selectable cloud recipients."""

    KEYS = list("ABCDEFGHIJKLMNOPQRSTUVWXYZ") + [
        "SPACE",
        "BACK",
        "CLEAR",
        "SEND",
        "TO ABIR",
        "TO BRAILLE",
        "TO VOICE",
        "HELP",
        "WATER",
    ]
    KEYBOARD_COLUMNS = 7

    def __init__(self, *args, target_device: str = "glove_01", **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.target_device = target_device
        self._outgoing_text: Optional[str] = None

    @property
    def target_label(self) -> str:
        return TARGET_LABELS.get(self.target_device, self.target_device.upper())

    def pop_outgoing_text(self) -> Optional[str]:
        outgoing = self._outgoing_text
        self._outgoing_text = None
        return outgoing

    def _activate_selected(self, now: float) -> Optional[str]:
        key = self.selected_key
        if len(key) == 1:
            self.text += key
        elif key == "SPACE":
            if self.text and not self.text.endswith(" "):
                self.text += " "
        elif key == "BACK":
            self.text = self.text[:-1]
        elif key == "CLEAR":
            self.text = ""
        elif key == "SEND":
            self._outgoing_text = self.text.strip() or None
        elif key in TARGETS:
            self.target_device = TARGETS[key]
        elif key == "HELP":
            self.text = "I NEED HELP"
            self._outgoing_text = self.text
            return self.text
        elif key == "WATER":
            self.text = "I NEED WATER"
            self._outgoing_text = self.text
            return self.text
        return None


def draw_ability_status(frame, controller, cloud, message: str) -> None:
    height, width = frame.shape[:2]
    cloud_text = "CLOUD ONLINE" if cloud and cloud.connected.is_set() else "CLOUD OFFLINE"
    cloud_color = (80, 220, 80) if cloud and cloud.connected.is_set() else (40, 40, 240)
    draw_text(
        frame,
        f"{cloud_text}  |  TO: {controller.target_label}",
        (max(15, width - 500), 115),
        scale=0.55,
        color=cloud_color,
    )
    if cloud:
        incoming = cloud.latest_incoming()
        if incoming and time.time() - incoming.timestamp < 30:
            draw_text(
                frame,
                f"INCOMING {incoming.source}: {incoming.text}"[:90],
                (15, max(190, height - 235)),
                scale=0.58,
                color=(255, 210, 80),
            )


def run(args) -> int:
    import cv2

    ensure_model(args.model)
    calibration = load_calibration(args.calibration)
    if calibration is None:
        print(f"No valid calibration found at {args.calibration}", file=sys.stderr)

    controller = (
        ParkinsonKeyboardController(
            calibration,
            invert_horizontal=True,
            target_device=args.target,
        )
        if calibration
        else None
    )
    calibrator = None if calibration else GuidedCalibration(time.monotonic())
    smoother = MedianGazeSmoother(args.smoothing_window)
    message = "CURRENT CALIBRATION LOADED" if calibration else calibrator.instruction

    cloud = None if args.offline else AbilityGazeCloud("gaze_01")
    if cloud:
        cloud.start()

    tracker = CursorFaceTracker(args.model)
    cursor_mapper = GazeCursorMapper(calibration, args.smoothing_window) if calibration else None
    camera = cv2.VideoCapture(args.camera)
    camera.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
    camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
    camera.set(cv2.CAP_PROP_FPS, 30)
    if not camera.isOpened():
        tracker.close()
        if cloud:
            cloud.close()
        print("Cannot open camera. Check macOS camera permission.", file=sys.stderr)
        return 2

    last_presence = 0.0
    last_cursor_publish = 0.0
    window = "Ability Parkinson Eye-Gaze Keyboard"
    cv2.namedWindow(window, cv2.WINDOW_NORMAL)

    try:
        while True:
            ok, frame = camera.read()
            now = time.monotonic()
            if not ok or frame is None:
                time.sleep(0.02)
                continue

            raw_sample = tracker.detect(frame, now)
            sample = smoother.update(raw_sample)

            if cloud and now - last_cursor_publish >= 0.05:
                cursor = (
                    cursor_mapper.update(sample, tracker.vertical_position)
                    if cursor_mapper is not None
                    else None
                )
                cloud.publish_cursor(
                    cursor.x if cursor else 0.5,
                    cursor.y if cursor else 0.5,
                    cursor is not None,
                    sample.blink_score if sample.face_present else 0.0,
                    cursor.selecting if cursor else False,
                )
                last_cursor_publish = now

            if calibrator is not None:
                completed, message = calibrator.update(raw_sample, now)
                if completed is not None:
                    calibration = completed
                    save_calibration(args.calibration, completed)
                    controller = ParkinsonKeyboardController(
                        completed,
                        invert_horizontal=True,
                        target_device=args.target,
                    )
                    calibrator = None
                    smoother = MedianGazeSmoother(args.smoothing_window)
                    cursor_mapper = GazeCursorMapper(completed, args.smoothing_window)
            elif controller is not None:
                output = controller.update(sample)
                if output.reason:
                    message = output.reason
                elif not sample.face_present:
                    message = "FACE NOT FOUND"
                else:
                    message = f"SMOOTHED GAZE {output.gaze.value}"

                outgoing = controller.pop_outgoing_text()
                if outgoing:
                    if cloud and cloud.publish_text(outgoing, controller.target_device):
                        message = f"SENT TO {controller.target_label}: {outgoing}"
                        controller.clear_text()
                    else:
                        message = "CLOUD OFFLINE - MESSAGE KEPT; SELECT SEND TO RETRY"

            draw_interface(frame, sample, controller, message)
            if controller is not None:
                draw_ability_status(frame, controller, cloud, message)

            if cloud and time.monotonic() - last_presence >= 10.0:
                target = controller.target_device if controller else args.target
                cloud.publish_presence(controller is not None, target)
                last_presence = time.monotonic()

            cv2.imshow(window, frame)
            key = cv2.waitKey(1) & 0xFF
            if key in (ord("q"), 27):
                break
            if key == ord("c"):
                controller = None
                cursor_mapper = None
                calibrator = GuidedCalibration(now)
                smoother = MedianGazeSmoother(args.smoothing_window)
                message = calibrator.instruction
            elif key == ord("r") and controller is not None:
                controller.clear_text()
            elif key == ord("i") and controller is not None:
                controller.toggle_horizontal(now)
    finally:
        camera.release()
        tracker.close()
        if cloud:
            cloud.close()
        cv2.destroyAllWindows()
    return 0


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--camera", type=int, default=0)
    parser.add_argument("--model", type=Path, default=DEFAULT_MODEL)
    parser.add_argument("--calibration", type=Path, default=DEFAULT_CALIBRATION)
    parser.add_argument(
        "--target",
        choices=tuple(TARGET_LABELS),
        default="glove_01",
        help="initial recipient device; recipient can also be changed by eye",
    )
    parser.add_argument("--smoothing-window", type=int, default=7)
    parser.add_argument("--offline", action="store_true")
    return parser.parse_args()


def main() -> int:
    try:
        return run(parse_args())
    except (OSError, RuntimeError, ValueError) as error:
        print(f"Gaze keyboard failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
