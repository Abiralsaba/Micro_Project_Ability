import unittest
from types import SimpleNamespace

from gaze_module.gaze_keyboard import (
    GazeCursorMapper,
    MedianGazeSmoother,
    ParkinsonKeyboardController,
    VisionSample,
    iris_vertical_position,
)
from camera_control import Calibration


CALIBRATION = Calibration(
    left_threshold=-0.2,
    right_threshold=0.2,
    blink_threshold=0.5,
)


class SmootherTests(unittest.TestCase):
    def test_median_rejects_single_gaze_spike(self):
        smoother = MedianGazeSmoother(5)
        scores = [0.01, 0.01, 0.90, 0.02, 0.01]
        result = None
        for index, score in enumerate(scores):
            result = smoother.update(VisionSample(index, True, score, 0.0))
        self.assertAlmostEqual(result.gaze_score, 0.01)

    def test_face_loss_resets_history(self):
        smoother = MedianGazeSmoother(3)
        smoother.update(VisionSample(0.0, True, 0.5, 0.0))
        smoother.update(VisionSample(0.1, False))
        result = smoother.update(VisionSample(0.2, True, -0.4, 0.0))
        self.assertAlmostEqual(result.gaze_score, -0.4)


class KeyboardTests(unittest.TestCase):
    def test_typed_text_is_queued_by_send_key(self):
        controller = ParkinsonKeyboardController(CALIBRATION)
        controller.text = "HELLO"
        controller.selected_index = controller.KEYS.index("SEND")
        controller._activate_selected(0.0)
        self.assertEqual(controller.pop_outgoing_text(), "HELLO")
        self.assertIsNone(controller.pop_outgoing_text())

    def test_recipient_is_eye_selectable(self):
        controller = ParkinsonKeyboardController(CALIBRATION)
        controller.selected_index = controller.KEYS.index("TO BRAILLE")
        controller._activate_selected(0.0)
        self.assertEqual(controller.target_device, "braille_01")

    def test_help_is_an_immediate_cloud_phrase(self):
        controller = ParkinsonKeyboardController(CALIBRATION)
        controller.selected_index = controller.KEYS.index("HELP")
        spoken = controller._activate_selected(0.0)
        self.assertEqual(spoken, "I NEED HELP")
        self.assertEqual(controller.pop_outgoing_text(), "I NEED HELP")


class CursorTests(unittest.TestCase):
    def test_vertical_iris_position_averages_both_eyes(self):
        landmarks = [SimpleNamespace(x=0.0, y=0.0) for _ in range(474)]
        for iris, upper, lower in ((468, 159, 145), (473, 386, 374)):
            landmarks[upper].y = 0.2
            landmarks[lower].y = 0.8
            landmarks[iris].y = 0.5
        self.assertAlmostEqual(iris_vertical_position(landmarks), 0.5)

    def test_existing_calibration_maps_to_full_cursor(self):
        mapper = GazeCursorMapper(CALIBRATION, window_size=1)
        top_left = mapper.update(
            VisionSample(0.0, True, gaze_score=-0.4, blink_score=0.0),
            0.2,
        )
        bottom_right = mapper.update(
            VisionSample(0.1, True, gaze_score=0.4, blink_score=0.0),
            0.8,
        )
        self.assertAlmostEqual(top_left.x, 0.0)
        self.assertAlmostEqual(top_left.y, 0.0)
        self.assertAlmostEqual(bottom_right.x, 1.0)
        self.assertAlmostEqual(bottom_right.y, 1.0)

    def test_calibrated_blink_holds_last_stable_cursor(self):
        mapper = GazeCursorMapper(CALIBRATION, window_size=1)
        stable = mapper.update(VisionSample(0.0, True, 0.0, 0.0), 0.5)
        blink = mapper.update(VisionSample(0.1, True, 0.4, 0.8), 0.8)
        self.assertEqual((blink.x, blink.y), (stable.x, stable.y))
        self.assertTrue(blink.selecting)


if __name__ == "__main__":
    unittest.main()
