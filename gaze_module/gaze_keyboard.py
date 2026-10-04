#!/usr/bin/env python3
"""Parkinson-friendly eye-gaze keyboard connected to Ability Chat."""

from __future__ import annotations

import argparse
from collections import deque
from dataclasses import replace
from pathlib import Path
from statistics import median
import sys
import time
from typing import Deque, Optional


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

    tracker = FaceTracker(args.model)
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
