import unittest
from gaze_module.gaze_keyboard import (
    MedianGazeSmoother,
    ParkinsonKeyboardController,
    VisionSample,
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
if __name__ == "__main__":
    unittest.main()
