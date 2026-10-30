// Bangla Unicode bridge for the toolkit's existing calibrated A-Z animations.
// This does not modify the avatar rig or its calibrated joint movements.

export const banglaToLatin = {
  "ঁ": "N", "ং": "NG", "ঃ": "H",
  "অ": "A", "আ": "A", "ই": "I", "ঈ": "I", "উ": "U", "ঊ": "U",
  "ঋ": "R", "ঌ": "L", "এ": "E", "ঐ": "OI", "ও": "O", "ঔ": "OU",
  "ক": "K", "খ": "KH", "গ": "G", "ঘ": "GH", "ঙ": "NG",
  "চ": "CH", "ছ": "CH", "জ": "J", "ঝ": "JH", "ঞ": "NY",
  "ট": "T", "ঠ": "TH", "ড": "D", "ঢ": "DH", "ণ": "N",
  "ত": "T", "থ": "TH", "দ": "D", "ধ": "DH", "ন": "N",
  "প": "P", "ফ": "PH", "ব": "B", "ভ": "BH", "ম": "M",
  "য": "J", "র": "R", "ল": "L", "শ": "SH", "ষ": "SH",
  "স": "S", "হ": "H", "ড়": "R", "ঢ়": "RH", "য়": "Y", "ৎ": "T",
  "া": "A", "ি": "I", "ী": "I", "ু": "U", "ূ": "U", "ৃ": "R",
  "ে": "E", "ৈ": "OI", "ো": "O", "ৌ": "OU", "্": "",
  "়": "", "ৗ": "OU"
};

export const banglaKnownWords = {
  "হ্যালো": "HELLO",
  "নমস্কার": "HELLO",
  "ধন্যবাদ": "THANK_YOU",
  "সময়": "TIME",
  "সময়": "TIME",
  "বাড়ি": "HOME",
  "বাড়ি": "HOME",
  "ঘর": "HOME",
  "মানুষ": "PERSON",
  "ব্যক্তি": "PERSON",
  "তুমি": "YOU",
  "আপনি": "YOU",
  "তুই": "YOU"
};

export function banglaSignTokens(value) {
  const words = String(value || "").normalize("NFC").match(/[\u0980-\u09FF]+|[A-Za-z]+/g) || [];
  const tokens = [];

  words.forEach(word => {
    const knownAnimation = banglaKnownWords[word];
    if (knownAnimation) {
      tokens.push({ animation: knownAnimation, label: `${word} `, sequence: "" });
      return;
    }

    if (/^[A-Za-z]+$/.test(word)) {
      [...word.toUpperCase()].forEach((character, index) => {
        tokens.push({
          animation: null,
          label: index === word.length - 1 ? `${character} ` : character,
          sequence: character
        });
      });
      return;
    }

    [...word].forEach((character, index) => {
      tokens.push({
        animation: null,
        label: index === [...word].length - 1 ? `${character} ` : character,
        sequence: banglaToLatin[character] || ""
      });
    });
  });

  return tokens;
}
