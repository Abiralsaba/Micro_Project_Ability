"""Language mappings shared by Ability Chat accessibility modules.

Braille integers use the same layout as the ESP32 firmware: dot 1 is bit 0,
dot 2 is bit 1, through dot 6 at bit 5.  Bengali entries follow the Standard
Bharati Braille Unicode mapping (Grade 1).  A character may produce more than
one cell, so every table value is a tuple.
"""

from __future__ import annotations

import unicodedata
from typing import Dict, Iterable, Tuple


SUPPORTED_LANGUAGES = {"en", "bn"}


def dots(*numbers: int) -> int:
    """Convert Braille dot numbers (1..6) to the firmware bit mask."""
    pattern = 0
    for number in numbers:
        if number < 1 or number > 6:
            raise ValueError(f"invalid Braille dot number: {number}")
        pattern |= 1 << (number - 1)
    return pattern


def _cells(*groups: Iterable[int]) -> Tuple[int, ...]:
    return tuple(dots(*group) for group in groups)


ENGLISH_BRAILLE: Dict[str, Tuple[int, ...]] = {
    letter: (dots(*dot_numbers),)
    for letter, dot_numbers in {
        "a": (1,), "b": (1, 2), "c": (1, 4), "d": (1, 4, 5),
        "e": (1, 5), "f": (1, 2, 4), "g": (1, 2, 4, 5),
        "h": (1, 2, 5), "i": (2, 4), "j": (2, 4, 5),
        "k": (1, 3), "l": (1, 2, 3), "m": (1, 3, 4),
        "n": (1, 3, 4, 5), "o": (1, 3, 5), "p": (1, 2, 3, 4),
        "q": (1, 2, 3, 4, 5), "r": (1, 2, 3, 5),
        "s": (2, 3, 4), "t": (2, 3, 4, 5), "u": (1, 3, 6),
        "v": (1, 2, 3, 6), "w": (2, 4, 5, 6),
        "x": (1, 3, 4, 6), "y": (1, 3, 4, 5, 6),
        "z": (1, 3, 5, 6),
    }.items()
}


# Bengali letters, dependent vowel signs, signs, and common additions.  The
# independent/dependent vowel pairs intentionally share the same cells.
BENGALI_BRAILLE: Dict[str, Tuple[int, ...]] = {
    "ঁ": _cells((3,)), "ং": _cells((5, 6)), "ঃ": _cells((6,)),
    "অ": _cells((1,)), "আ": _cells((3, 4, 5)), "ই": _cells((2, 4)),
    "ঈ": _cells((3, 5)), "উ": _cells((1, 3, 6)),
    "ঊ": _cells((1, 2, 5, 6)), "ঋ": _cells((5,), (1, 2, 3, 5)),
    "ঌ": _cells((5,), (1, 2, 3)), "এ": _cells((1, 5)),
    "ঐ": _cells((3, 4)), "ও": _cells((1, 3, 5)),
    "ঔ": _cells((2, 4, 6)),
    "ক": _cells((1, 3)), "খ": _cells((4, 6)),
    "গ": _cells((1, 2, 4, 5)), "ঘ": _cells((1, 2, 6)),
    "ঙ": _cells((3, 4, 6)), "চ": _cells((1, 4)),
    "ছ": _cells((1, 6)), "জ": _cells((2, 4, 5)),
    "ঝ": _cells((3, 5, 6)), "ঞ": _cells((2, 5)),
    "ট": _cells((2, 3, 4, 5, 6)), "ঠ": _cells((2, 4, 5, 6)),
    "ড": _cells((1, 2, 4, 6)), "ঢ": _cells((1, 2, 3, 4, 5, 6)),
    "ণ": _cells((3, 4, 5, 6)), "ত": _cells((2, 3, 4, 5)),
    "থ": _cells((1, 4, 5, 6)), "দ": _cells((1, 4, 5)),
    "ধ": _cells((2, 3, 4, 6)), "ন": _cells((1, 3, 4, 5)),
    "প": _cells((1, 2, 3, 4)), "ফ": _cells((1, 2, 4)),
    "ব": _cells((1, 2)), "ভ": _cells((4, 5)),
    "ম": _cells((1, 3, 4)), "য": _cells((1, 3, 4, 5, 6)),
    "র": _cells((1, 2, 3, 5)), "ল": _cells((1, 2, 3)),
    "শ": _cells((1, 4, 6)), "ষ": _cells((1, 2, 3, 4, 6)),
    "স": _cells((2, 3, 4)), "হ": _cells((1, 2, 5)),
    "়": _cells((5,)), "ঽ": _cells((2,)),
    "া": _cells((3, 4, 5)), "ি": _cells((2, 4)),
    "ী": _cells((3, 5)), "ু": _cells((1, 3, 6)),
    "ূ": _cells((1, 2, 5, 6)), "ৃ": _cells((5,), (1, 2, 3, 5)),
    "ৄ": _cells((6,), (1, 2, 3, 5)), "ে": _cells((1, 5)),
    "ৈ": _cells((3, 4)), "ো": _cells((1, 3, 5)),
    "ৌ": _cells((2, 4, 6)), "্": _cells((4,)),
    "ৎ": _cells((4,), (2, 3, 4, 5), (1, 2, 3, 5)),
    "ড়": _cells((1, 2, 4, 5, 6)), "ঢ়": _cells((5,), (1, 2, 4, 6)),
    "য়": _cells((2, 6)),
}


NUMBER_SIGN = dots(3, 4, 5, 6)
DIGIT_CELLS = {
    "0": dots(2, 4, 5), "1": dots(1), "2": dots(1, 2),
    "3": dots(1, 4), "4": dots(1, 4, 5), "5": dots(1, 5),
    "6": dots(1, 2, 4), "7": dots(1, 2, 4, 5),
    "8": dots(1, 2, 5), "9": dots(2, 4),
}
BENGALI_DIGITS = str.maketrans("০১২৩৪৫৬৭৮৯", "0123456789")
COMMON_PUNCTUATION = {
    ",": dots(2), ";": dots(2, 3), ":": dots(2, 5),
    ".": dots(2, 5, 6), "?": dots(2, 6), "!": dots(2, 3, 5),
    "-": dots(3, 6), "'": dots(3), "।": dots(2, 5, 6),
}


def normalize_language(language: str | None) -> str:
    value = str(language or "en").lower().strip()
    return "bn" if value in {"bn", "bd", "bangla", "bengali", "বাংলা"} else "en"


def text_to_braille(text: str, language: str = "en") -> list[int]:
    """Translate Unicode text into firmware-ready six-dot cell bit masks."""
    language = normalize_language(language)
    normalized = unicodedata.normalize("NFC", str(text or ""))
    table = BENGALI_BRAILLE if language == "bn" else ENGLISH_BRAILLE
    patterns: list[int] = []
    in_number = False

    for original in normalized:
        character = original.translate(BENGALI_DIGITS)
        if character.isdigit() and character in DIGIT_CELLS:
            if not in_number:
                patterns.append(NUMBER_SIGN)
            patterns.append(DIGIT_CELLS[character])
            in_number = True
            continue
        in_number = False

        if character.isspace():
            patterns.append(0)
            continue
        if character in COMMON_PUNCTUATION:
            patterns.append(COMMON_PUNCTUATION[character])
            continue

        lookup = character if language == "bn" else character.lower()
        cells = table.get(lookup)
        # Bangla conversations commonly contain English names/acronyms.
        if cells is None and language == "bn":
            cells = ENGLISH_BRAILLE.get(character.lower())
        if cells:
            patterns.extend(cells)

    return patterns
