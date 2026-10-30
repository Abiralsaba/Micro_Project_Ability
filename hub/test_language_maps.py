import unittest

from hub.language_maps import dots, normalize_language, text_to_braille


class LanguageMapTests(unittest.TestCase):
    def test_english_mapping_matches_firmware_bit_order(self):
        self.assertEqual(text_to_braille("Ab z", "en"), [dots(1), dots(1, 2), 0, dots(1, 3, 5, 6)])

    def test_bangla_chart_examples(self):
        self.assertEqual(
            text_to_braille("অআইঈউঊ", "bn"),
            [dots(1), dots(3, 4, 5), dots(2, 4), dots(3, 5), dots(1, 3, 6), dots(1, 2, 5, 6)],
        )
        self.assertEqual(
            text_to_braille("কখগঘঙ", "bn"),
            [dots(1, 3), dots(4, 6), dots(1, 2, 4, 5), dots(1, 2, 6), dots(3, 4, 6)],
        )

    def test_bangla_word_keeps_vowel_signs_and_virama(self):
        self.assertEqual(
            text_to_braille("বাংলা", "bn"),
            [dots(1, 2), dots(3, 4, 5), dots(5, 6), dots(1, 2, 3), dots(3, 4, 5)],
        )

    def test_multicell_vocalic_r_and_bangla_digits(self):
        self.assertEqual(text_to_braille("ঋ ১২", "bn"), [dots(5), dots(1, 2, 3, 5), 0, dots(3, 4, 5, 6), dots(1), dots(1, 2)])

    def test_language_aliases(self):
        self.assertEqual(normalize_language("bd"), "bn")
        self.assertEqual(normalize_language("বাংলা"), "bn")
        self.assertEqual(normalize_language("eng"), "en")


if __name__ == "__main__":
    unittest.main()
