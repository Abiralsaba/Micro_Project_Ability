# Sources and BDT comparison basis

Public pages checked **30 September 2026**. All presentation amounts are in **Bangladeshi taka**. Original foreign list prices are retained here so the conversions can be checked.

## Currency basis

[Bangladesh Bank](https://www.bb.org.bd/en/) showed an interbank weighted average USD/BDT rate of **123.0300**, updated **30 September 2026**. The deck uses the rounded presentation conversion **USD 1 ≈ ৳123**. These are approximate converted sticker prices, not Bangladesh import quotes; taxes, shipping and bank fees are excluded.

## Selected premium references

| Product and official source | Observed foreign price | At ৳123 per USD | Scope difference |
|---|---:|---:|---|
| [Rokoko Smartgloves II](https://store.rokoko.com/products/smartgloves-ii) | USD 1,995 | ৳245,385 ≈ **৳2.45 lakh** | Professional motion-capture pair, versus Ability’s single gesture-glove estimate of ৳4,000–5,000. Different purpose, sensor capability, quantity and support. |
| [HumanWare Brailliant BI 40X](https://store.humanware.com/hus/brailliant-bi-40x-braille-display.html) | USD 3,759 | ৳462,357 ≈ **৳4.62 lakh** | Commercial 40-cell reader, versus Ability’s four-cell, six-dot prototype target of ৳3,500. |
| [Orbit Reader 20](https://www.orbitresearch.com/products/blindness-products/braille-devices/orbit-reader-20/) | USD 799 | ৳98,277 ≈ **৳98,300** | Lower-price commercial context; twenty eight-dot cells. This prevents implying all market devices cost several lakh. |
| [OpenBCI Cyton eight-channel board](https://shop.openbci.com/products/cyton-biosensing-board-8-channel) | USD 1,249 | ৳153,627 ≈ **৳1.54 lakh** | EEG acquisition board only, electrodes excluded. Not a wheelchair price. The ৳12,000 Ability controller target does not include this board. |

The opened HumanWare product and category pages showed **USD 3,759**. A search snippet still showed an older **USD 3,299** promotion; the deck uses the live opened listing. These are selected premium references, not representative market averages or claims that Ability has equivalent performance. No percentage savings is calculated across these unequal products.

## Ability budget basis

| Module | BDT figure | Basis |
|---|---:|---|
| Glove | ৳4,500 working midpoint | Owner supplied ৳4,000–5,000 estimate; individual line items are budget allocations. |
| Four-cell Braille | ৳3,500 | Proposed low-cost target; supplier quotes pending. |
| Voice | ৳1,500 | Proposed microphone/amplifier/speaker/ESP32 target. |
| Gaze | ৳3,500 | Proposed economy USB camera interface; existing screen and Pi hub reused. |
| Shared Pi hub | ৳15,000 | Lower-memory or reused Pi 5 target, including power/storage/cooling; not a new 8GB kit quote. |
| Wheelchair controller add-on | ৳12,000 | Proposed control interface only; EEG, electrodes, powered chair, motors, brakes, battery and validation excluded. |

**Core:** ৳23,000. **Communication build with proposed voice/gaze:** ৳28,000. **With 10% reserve:** ৳30,800. Mobility remains separate. Parts-only targets exclude software development, model training, labor, tax, shipping and production/clinical validation. Small local variations of roughly ৳50–100 may occur; they do not turn a target into a receipt. Component-level rows are in budget.csv.

## Technical and novelty references

- [Orbit Chat](https://www.orbitresearch.com/products/blindness-products/apps/orbit-chat-an-app-for-face-to-face-communication-with-people-who-are-deafblind/): prior accessible communication product, relevant to bounding novelty.
- [Hybrid EEG/EOG wheelchair research, 2014](https://pmc.ncbi.nlm.nih.gov/articles/PMC4155067/): brain/eye wheelchair control has precedents. No published accuracy was transferred to Ability.
- [MediaPipe Iris documentation](https://chuoling.github.io/mediapipe/solutions/iris.html): landmarks do not themselves infer gaze position; calibrated gaze estimation is still required.
- [Parkinson’s Foundation: vision changes](https://www.parkinson.org/understanding-parkinsons/non-movement-symptoms/vision): vision and eye coordination can be affected, so gaze suitability needs individual assessment.

The custom AI/ML hub and chat interface are presented as the intended platform design. No trained model, chat implementation or model accuracy report was available in this checkout. The pictured conversation is an original interface concept, not actual device telemetry.
