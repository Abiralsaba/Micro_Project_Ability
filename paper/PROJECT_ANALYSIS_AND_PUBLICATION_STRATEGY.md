# Project Ability: project analysis and publication strategy

**Latest research assessment:** See the [AMP literature dossier](literature_review/REVIEW.md) for the user-selected IEEE IoT Journal protocol direction. Its recommendation is **MODIFY**: narrow the contribution and test it against established multimodal and reliable-messaging mechanisms. This source-code audit remains separate from that literature assessment.

**Audit date:** 15 September 2026  
**Scope:** All 56 existing project files, including ignored planning/context files and locally present dataset/model folders; excludes Git internals and macOS `.DS_Store` files.  
**Evidence:** Source inspection, consistency checks, Git history, and a targeted online literature/venue search. This is not a systematic literature review or a hardware validation report.

## 1. Recommendation

**Subsequent user decision:** IEEE Internet of Things Journal is now the primary target, with the Cross-Disability Remote Communication Protocol as the paper's focus. See the [focused AMP research plan](IEEE_IOTJ_PROTOCOL_RESEARCH_PLAN.md). The provisional venue comparison below records the initial audit assessment.

**Develop one focused, experimentally supported paper from the complete Ability platform. My provisional first Q1 target is IEEE Sensors Journal if the main result is a substantial sensing/fusion or integrated sensor–actuator contribution. IEEE Internet of Things Journal is the stretch target if the main result is a generalizable communication-system contribution.**

The existing documents choose IoT Journal largely because the design uses MQTT, edge processing, and encryption. That is insufficient justification: the manuscript must demonstrate what the system enables or improves beyond established components and competing implementations.

You answered “all” when asked which modules and the hub were physically built and tested. This report treats **all components as built/tested according to the project owner**. The files available here do not contain the complete deployed software, experimental measurements, or participant-study records. The older “no prototype built yet” statements in the brain and plan therefore cannot be used as the current hardware status. Conversely, your confirmation is not a substitute for measured results in a manuscript.

**Publication readiness:** Not established from the available research artifacts. This is an evidence and research-contribution gap, not a conclusion that the hardware does not work.

### Completed research brief

| Requested field | Recommended wording |
|---|---|
| Research topic | Modular edge-assisted communication across sign/gesture, Braille, gaze, speech, and text interfaces |
| Research question | Can a modular platform preserve message meaning and support successful two-way communication between users with different access needs, while reducing deployment cost and maintaining usable response times under realistic sensing and network conditions? |
| Target domain | Assistive technology; embedded/IoT systems; human–computer interaction; multimodal sensing. ML/CV are enabling methods where actually implemented and evaluated. |
| Target publication level | Q1 journal-quality original empirical research. Use **JCR JIF Q1** as the strict planning assumption until the institution's accepted ranking system is clarified. |
| Preferred literature years | 2020–15 September 2026, plus earlier foundational or directly overlapping work. A date window must not exclude prior art that affects novelty. |
| Working title | **Ability: Design and Evaluation of a Modular Edge-Assisted Platform for Cross-Modal Assistive Communication** |

Avoid putting “first,” “universal,” “full sign-language translation,” “tremor-tolerant,” or “privacy-preserving” in the title until the corresponding contribution is defined and demonstrated.

## 2. What the project is

Ability separates personal input/output devices from a shared processing and routing hub. Its proposed local system uses ESP32 and Raspberry Pi devices; a cloud MQTT broker relays messages between hubs. Text is the proposed interchange representation, with speech recognition performed at the hub for the voice device.

There are four hardware module families:

- **Glove:** flex sensing and IMU orientation, with a separate camera/vision processor; proposed sensor–vision sign/gesture recognition and visual output.
- **Braille:** a Perkins-style input design and mechanically actuated six-dot output; recent code also implements four output cells driven by eight servos.
- **Gaze:** proposed camera-based text entry with blink/dwell interaction.
- **Voice:** microphone capture and proposed synthesized speech playback.

The six profiles in the plan—Deaf/hard of hearing, blind/low vision, people with speech impairments, people with paralysis, people with Parkinson's, and hearing/sighted users—describe intended users. They are not six independent sensing technologies. The two gaze profiles reuse essentially the same input/output stack, and the sign/gesture profiles share hardware.

Six distinct labels produce **6 × 5 = 30 directed off-diagonal profile pairs**, or 15 unordered pairs. This does not demonstrate 30 independent translations, nor is it “30+” without defining additional cases. The encoder/renderer decomposition is useful engineering, but the reduction in adapter count alone is not a new algorithm or proof that all messages retain their meaning.

Treat the profiles as use cases rather than fixed assumptions about ability. Let users select suitable modalities. For example, access needs, sign-language knowledge, Braille literacy, reading ability, and reliable gaze control should be established individually.

## 3. What every part of the repository contributes

The companion [file inventory](FILE_INVENTORY.csv) lists every original file, its size, line count, SHA-256 digest, and assessment.

### 3.1 Planning and research documents

| File | Assessment |
|---|---|
| `PROJECT_BRAIN.md` | Useful architecture/TODO summary. Hardware status is stale relative to your confirmation and September firmware commits. Claimed costs, FPS, accuracy, latency, prevalence figures, and novelty are not recorded experimental results. |
| `plan/MASTER_PLAN.md` | Comprehensive 931-line design and roadmap. The 30 paths are specifications, even where phrased as already working. Contains conflicting compute placement, BNO085 versus BNO055 selection, bandwidth, security terminology, and latency assumptions. |
| `anti/novelty_analysis.md` | A hypothesis-generating memo, not a verified literature review. It asserts a review of 40+ papers but supplies no reproducible search log, complete bibliography, DOI table, or evidence extraction. Several absolute novelty claims are contradicted or weakened by identifiable prior work. |
| `.agents/rules/project-context.md` | Repeats the intended architecture and novelty claims. Useful project context; not independent scientific evidence. |
| Root `README.md` | Explains the ecosystem clearly, but often describes planned behavior as implemented capability. |
| `paper/README.md` | A placeholder naming IoT Journal and future LaTeX/bibliography/figures; no manuscript or experimental tables existed at audit start. |

The referenced proposal PDF and three supporting documents are absent: `Ability_Project_Proposal.pdf`, `anti/project_ability_slide_deck.md`, `anti/research_proposal_slides.md`, and `anti/sign_language_fix.md`. Only `anti/novelty_analysis.md` exists in `anti/`.

### 3.2 Braille: the most developed implementation in this checkout

| Source | Implemented behavior | Research interpretation |
|---|---|---|
| `braille_module.ino` — 354 lines | Six-dot button polling, message accumulation, MQTT send/receive, two-servo output | Integration draft with substantive correctness and timing issues below; speaker playback is absent. |
| `brailv1.ino` — 482 lines | Serial text input; alphabet encoding; calibrated independent servo angles; right-column reversal; home/test/sweep commands | Concrete single-cell controller and calibration artifact. No measured pin accuracy or reading results. |
| `v2working.ino` — 482 lines | Same behavior as `brailv1.ino` | The two files are identical after whitespace removal. Their names do not establish separate experimental versions. |
| `demomodule.ino` — 933 lines | ESP32 Wi-Fi access point, embedded browser interface, four Braille cells/eight servos, four-character batches, status and diagnostics | Most substantial integrated demo. It uses local HTTP/AP operation, not the documented hub/cloud MQTT route. |
| Braille `README.md` | Wiring, external supply guidance, alphabet reference, calibration and troubleshooting | More developed than the other hardware documentation, but the described timing and angles do not match the calibrated source. |

The demo's “four modules” means four Braille output cells. It does **not** mean that the glove, gaze, Braille, and voice module families are integrated inside that file.

No CAD, STL, dimensioned mechanism drawing, measured force, mechanical tolerance, cycle-life record, or assembly photo is present. Firmware commands reveal intended positions, not actual pin heights or tactile readability.

### 3.3 Glove

The 517-line firmware reads four ADS1115 flex channels and a fifth ESP32 ADC channel; reads BNO055 quaternion/Euler/linear-acceleration data; sends UART JSON; publishes health/environment data; and runs a threshold-based acceleration task. It forwards hub messages to the Pi with a `HUB:` prefix.

The camera processor, temporal alignment, inference, avatar player, training pipeline, and learned model are only listed as files to create. Consequently, this checkout supports analysis of **sensor acquisition firmware**, not a measured sign-recognition result. Your deployed recognition code and model need to be associated with this repository's research version.

### 3.4 Voice

The 185-line firmware initializes a microphone and attempts to publish raw PCM chunks. Voice activity detection, speaker initialization, and received-audio playback remain TODOs. There is no speech recognizer here; the recognizer is intended to run on the absent hub implementation.

### 3.5 Gaze, hub, ML, assets, tests, scripts, shared code

- Gaze and glove vision folders contain README specifications, not Python source.
- Hub routing, converters, speech engines, networking, UI, and health directories contain README specifications plus an example YAML configuration. `hub/core/main.py`, referenced by Quick Start, is absent.
- ML directories, including ignored raw-data and trained-model folders, contain README files only. There are no weights, samples, split manifests, learning curves, confusion matrices, or evaluation logs locally.
- Shared cryptography and Braille utilities are README specifications. AMP has a real JSON schema but no validator/serializer implementation.
- Avatar/audio asset folders contain README files only.
- Unit, integration, and end-to-end test folders contain README files only. No executable test suite was found.
- Setup, deployment, and tooling folders contain README specifications only.
- Hardware/API docs mostly specify future documents; the substantive Braille wiring guide is in the module README.
- `requirements.txt` uses broad minimum versions rather than a reproducible environment lock. Board-specific Arduino dependencies and build configurations are not pinned.
- `mosquitto.conf` is a real configuration template, with TLS, authentication, ACL and persistence paths. It does not prove a deployed or validated relay.
- `.gitignore` excludes the brain, master plan, agent context, model/data directories, and binary assets. Ignored local content was included in this audit. Exporting a paper artifact requires an explicit manifest and release location.

## 4. Technical findings that affect paper credibility

These observations apply to the files in this checkout. If your hardware runs newer versions, archive those versions and rerun the relevant checks before attributing these issues to the deployed system.

### 4.1 Definite source-level problems

**A. Networked Braille input mapping is inconsistent for 25 of 26 letters.**

At `braille_module.ino:116`, the decoder does not invert the alphabet encoder in the serial/web demo. For example, the demo encodes `b` as `0x03`, while the networked decoder returns a comma at that index; `h` is `0x13`, which decodes as `?`. A source-derived check found only `a` round-trips correctly. A shared, language-specific encoding specification and independent reference tests are necessary.

**B. Braille chord capture samples too early and repeats held input.**

At `braille_module.ino:182`, the code reads a button pattern, then waits 300 ms, then decodes the original pattern without sampling again. The delay cannot collect fingers pressed later. There is no complete chord/release state machine; a held chord or space button can be appended repeatedly. Evaluate with actual staggered key presses and corrections, not just simultaneous electrical inputs.

**C. Four complete sketches occupy one Arduino sketch directory.**

All four Braille `.ino` files define `setup()` and `loop()` and reuse globals/functions. Arduino concatenates `.ino` files in a sketch directory, so the normal directory build encounters duplicate definitions. Each alternative must be its own sketch/build target. This conclusion follows from the [Arduino build specification](https://docs.arduino.cc/arduino-cli/sketch-build-process).

**D. The declared ESP32-S3 target conflicts with several GPIO choices.**

The glove uses GPIO22 for I²C; the networked Braille controller uses GPIO25; the voice controller uses GPIO22/25. These are not ESP32-S3 GPIOs. Several other pins require checking against the exact module's flash/PSRAM wiring. The Wi-Fi Braille demo's pin set instead resembles a classic ESP32 allocation. Record the actual board model and correct its configuration using the [Espressif GPIO documentation](https://docs.espressif.com/projects/esp-idf/en/v5.1/esp32s3/api-reference/peripherals/gpio.html).

**E. Voice bandwidth is 256 kb/s for the specified raw format.**

At 16,000 samples/s × 16 bits × one channel, PCM requires **256,000 bits/s = 32,000 bytes/s**, before networking overhead. The documents confuse 32 kB/s with 32 kb/s. No compression codec is implemented here.

The 1,024-sample buffer contains 2,048 bytes. The MQTT buffer is also set to 2,048 bytes, leaving no space for topic/header overhead when a full block is published. The publish return value is ignored. The [PubSubClient implementation](https://github.com/knolleary/pubsubclient/blob/master/src/PubSubClient.cpp) checks the total packet allocation, so full blocks exceed this limit. Capture attempted bytes, accepted publishes, received bytes, dropped blocks, and recognizer output.

**F. Sensor timing does not support the claimed 100 Hz configuration.**

Four sequential single-ended conversions at 250 samples/s need at least approximately **16 ms**, before additional IMU reads, serialization, and other work; a full 100 Hz frame permits only 10 ms. UART at 115,200 baud with 8N1 permits approximately **115.2 bytes/frame at 100 Hz**. Measure actual serialized length, frame timestamps, missed deadlines, and sampling skew. The fifth finger also uses a different ADC resolution and needs its own normalization/calibration.

**G. Calibration/home behavior differs across the Braille versions.**

In the calibrated versions, the left zero-dot angle is 32°, but startup and homing write 22°. This may be intentional mechanical clearance, but the distinction is not documented. The networked version uses a different regular angle table and does not include the demo's right-column bit reversal. A working serial demonstration therefore does not establish correct networked actuation on the same hardware.

### 4.2 Blocking behavior and missing feedback

- Networked Braille performs 1.5 seconds of holding plus 0.3 seconds of gap **inside the MQTT callback for each character**. Receiving/processing other messages and button input is delayed. A 20-character message occupies at least 36 seconds of these delays.
- The Wi-Fi demo calls `server.handleClient()` only in its outer loop. Display/test/sweep operations block it for seconds or longer, preventing the status endpoint from serving progress during those operations.
- With all four cells active, configured delays give about 6.08 seconds for a four-character batch including drive/hold/home delays, plus 3 seconds between batches. Long-text pacing is about 9.08 seconds per four characters, before other execution overhead. **These are schedule calculations, not measured refresh speed or human reading speed.**
- `Servo.attached()` is used as `moduleOK`; it reports software attachment, not successful pin movement, correct wiring, or tactile output. Do not call it physical actuator verification.
- All four cells use the same calibration arrays. Manufacturing variation requires per-cell verification and potentially separate calibration.
- The browser renders the first four entered characters immediately; it does not receive actual pin positions or current batch contents. It is a command preview, not hardware telemetry.
- Status JSON is manually concatenated with submitted text; quotes/backslashes can invalidate it. The UI's log uses `innerHTML` with user input. The demo also handles commands through the same text field. These need cleaning before a participant-facing deployment.
- The demo converts uppercase to lowercase and supports letters/spaces; it does not implement complete literary Braille, number/capital indicators, punctuation, contractions, or Bangla Braille. Unsupported characters are skipped or left inactive depending on the version.

### 4.3 Hub/protocol/security gaps

The AMP schema is parseable JSON and identifies sender/receiver hubs/modules, content and time. It does not define a complete secure messaging protocol. Missing research-relevant details include capability negotiation, recipient selection, message ordering, acknowledgement of rendering, application-level deduplication, retry behavior, expiry enforcement, and congestion/backpressure from slow tactile outputs.

The encryption flag can coexist with plaintext content; there is no defined ciphertext envelope, nonce, authentication tag, key identifier, replay protection, or authenticated pairing implementation. The plan's phrase “AES ... receiver hub's public key” conflates symmetric encryption with key establishment. ECDH can establish a shared secret; AES-GCM then uses a derived symmetric key.

The hub is a plaintext processing endpoint in the design. Describe the intended boundary as **hub-to-hub content protection from the relay**, unless module-to-module security is also implemented. The cloud can still observe routing metadata, and disability labels need not travel in plaintext. Persistence can store ciphertext; the claim that the broker never stores content conflicts with offline queuing if interpreted literally.

MQTT delivery QoS is not evidence that a recipient actually received the message in an accessible form. The ESP32 PubSubClient stack also does not implement the advertised MQTT v5/QoS2 publishing path: its [official README](https://github.com/knolleary/pubsubclient) documents QoS0 publishing and MQTT 3.1.1/3.1 support. The plan's own-topic-only ACL description needs reconciling with publishing to another hub's inbox. Record the actual ACL and session settings rather than inferring them from comments.

### 4.4 ML, gaze and language claims

**Landmark coverage is not proof of complete sign-language understanding.** Capturing hands, body and face does not establish correct linguistic interpretation, continuous segmentation, grammar, coarticulation, or generalization across signers. Five flex values do not uniquely determine all handshapes. Define the language, vocabulary, one/two-hand coverage, signing conditions and task: isolated sign recognition, fingerspelling, constrained phrases, or continuous translation.

Likewise, English word-to-clip lookup does not establish grammatical sign-language output. Have fluent users evaluate comprehension and naturalness. Modern sign-production work explicitly models holistic motion; [SignAvatars, ECCV 2024](https://www.ecva.net/papers/eccv_2024/papers_ECCV/papers/00653.pdf) is useful context, not proof that your avatar renderer exists or is comprehensible.

**MediaPipe Iris does not directly estimate where a user looks on a screen.** Google's own [Iris description](https://research.google/blog/mediapipe-iris-real-time-iris-tracking-depth-estimation/) distinguishes iris landmarks from gaze inference. A calibrated mapping, head-pose handling, drift assessment, and target-selection evaluation are still needed. RGB-model performance under NoIR/850 nm illumination must be measured.

Do not infer effective Parkinson's support merely from Gaussian smoothing. Establish the actual input difficulty with participants and relevant expertise; test smoothing and adaptive dwell against alternatives. No clinical efficacy, tremor reduction, or universal gaze suitability follows from the supplied code. The same caution applies to the unvalidated health subsystem: `spo2 = 0` is a placeholder and the acceleration threshold is not a validated fall detector.

The plan's 30–60-frame window at 5–8 FPS spans approximately **3.75–12 seconds**. A BiLSTM needs the selected window's future context before output. A claimed “under 500 ms end-to-end” result must state whether it begins at sign onset, sign completion, model output, or network send. Separate these timings rather than combining incompatible definitions.

## 5. Novelty reassessment against identifiable prior work

The following are concrete comparators, not a claim to have searched all literature. Descriptions below distinguish a paper's reported design from independently replicated performance.

| Prior work | Overlap and consequence for Ability |
|---|---|
| **Aryan, Rajkumar E., and Shivam Doshi (2021), “Refreshable Braille Module Using Cam Actuated Mechanism,” IOP Conference Series: Materials Science and Engineering 1123, 012028.** [DOI](https://doi.org/10.1088/1757-899X/1123/1/012028); [accessible paper text](https://www.researchgate.net/publication/351539848_Refreshable_Braille_Module_Using_Cam_Actuated_Mechanism) | Reports two servo-driven camshafts, each controlling three pins, and a gear transmission. It also describes an app, Firebase integration, and several communication cases. This directly precedes the broad two-actuator and multimodal-platform ideas. Ability's proposed **linear rack** differs from rotary camshafts; prove the geometric and functional difference with CAD and comparative measurements. Do not treat the earlier paper's broad claims as proof of a fully evaluated remote platform. |
| **“Sign Language Recognition with Multimodal Sensors and Deep Learning Methods” (2023), Electronics 12(23), 4827.** [Publisher paper](https://www.mdpi.com/2079-9292/12/23/4827) | Already combines bending sensors, MediaPipe keypoints, and CNN–BiLSTM recognition; discusses occlusion and lightweight representation. “Glove + camera + BiLSTM” is not a sufficient novelty claim. An advance could concern independently validated robustness, synchronization, uncertainty handling or measured edge efficiency, if demonstrated against strong baselines. |
| **Pérez-Aguirre et al. (2024), “Bidirectional Braille-speech Communication System for Deafblind Students,” Journal on Efficiency and Responsibility in Education and Science 17(2), 164–177.** [Publisher paper](https://www.eriesjournal.com/index.php/eries/article/download/1192/291/5858) | Describes two-way speech/Finger-Braille interaction with buttons and vibrotactile output. This differs from refreshable raised pins, but establishes that bidirectional accessible communication is not itself new. Compare output type, vocabulary, interaction tasks and evaluation population rather than juxtaposing headline accuracies. |
| **Yu et al. (2024), “SignAvatars: A Large-scale 3D Sign Language Holistic Motion Dataset and Benchmark,” ECCV.** [Paper](https://www.ecva.net/papers/eccv_2024/papers_ECCV/papers/00653.pdf) | Relevant prior work for sign production and holistic motion representation. A collection of clips is an output component, not equivalent to solving sign-language translation. |
| **Tang and Piper (2026), “Designing for Collective Access: In Search of a Solution to Accessible Communication in a Mixed-Ability Non-Profit.”** [Preprint](https://arxiv.org/abs/2605.10085) | A six-month qualitative study of communication in a mixed-ability organization. Useful for designing evaluation around differing preferences, communication norms and access trade-offs. It is not a competing sensor benchmark; publication status beyond the consulted preprint was not verified. |
| **Keller and Owens (2001), “Enhancing email text production for users with motor impairments.”** [Original proceedings record](https://aisel.aisnet.org/acis2001/52/) | An older cross-disability email system with accessible keyboards and word prediction. It does not replicate Ability, but illustrates why “cross-disability remote communication never existed” is too broad. |

A closely related non-journal engineering example also documents **two servos and linear cams with eight states per three-pin column**, using a Scotch-yoke transmission: [Refreshable Braille Display Project, described as completed December 2024](https://bekheet.ca/projects/refreshable-braille-character.html). Treat its reported metrics as project claims. Include it in mechanical prior-art screening; the specific rack-and-pinion geometry still requires direct comparison.

### Verdict on the five contributions in `anti/`

| Existing claim | Revised assessment |
|---|---|
| First remote cross-disability protocol | **Not established.** A JSON envelope and MQTT routing are implementation choices. Identify an unmet requirement and evaluate a new solution. |
| Six profiles / 30 paths | **Coverage specification**, not standalone scientific novelty. Demonstrate distinct user needs and actual path results. |
| Three-tier modular architecture | **Plausible systems contribution**, but not inherently new topology. Evaluate trade-offs and establish why existing systems are inadequate. |
| Tremor-tolerant gaze keyboard | **Research hypothesis.** Needs implemented adaptive behavior and an appropriate participant evaluation. |
| Edge sensor–vision fusion | **Closest ML route**, but close prior art exists. The improvement must survive participant-independent tests and compute/power comparison. |

## 6. Choose the paper's central contribution

The complete platform can remain the context and demonstration. The paper should answer one central research question, supported by two or three connected contributions.

### Route A — sensing and fusion: provisional first Q1 route

**Question:** Does the implemented fusion method improve recognition across unseen signers and degraded sensing conditions within a measured embedded compute/energy budget?

Possible contributions, only if present or subsequently implemented:

1. A precisely specified synchronization/fusion method with measurable benefit over single modalities and a strong conventional fusion baseline.
2. A deployed implementation with measured accuracy–latency–energy trade-offs on the actual device.
3. Demonstrated communication through physical Braille and other outputs, including message comprehension and failure recovery.

**Target:** IEEE Sensors Journal. Retain the broader ecosystem in the system overview. A plain CNN/BiLSTM combination, high training accuracy or naming more disability groups will not substantiate this route.

### Route B — communication middleware: full-system stretch route

**Question:** Can delivery that accounts for receiver capabilities and output pace reduce lost, duplicated, stale or unusable messages under network interruptions and mixed output speeds?

For example, a Braille receiver may need seconds to render text while a sender generates messages much faster. A defensible systems contribution could explicitly manage acknowledgement, ordering, expiry, queue pressure and accessible error recovery. Compare it with ordinary FIFO MQTT forwarding using the same endpoints. This is a **proposed research direction**, not a claim about existing AMP.

Possible contributions: formal message/rendering semantics; a tested delivery policy; end-to-end network and participant evidence. Two independently connected hubs and quantified failure conditions matter more than a remote demonstration alone.

**Target:** IEEE Internet of Things Journal, if the generalizable advance is strong. Otherwise an applied integration journal may be more appropriate.

### Route C — tactile hardware and interaction

**Question:** Does the actual rack/cam mechanism offer a worthwhile trade-off in tactile reliability, refresh time, energy, manufacturability and cost compared with existing mechanisms?

The four-cell demo makes this route concrete in the checkout. Distinguish geometric innovation from implementation and calibration. Compare a single cell and four cells; report standard versus enlarged geometry honestly. A mechanism can be affordable yet too wide or too slow for practical line reading.

**Targets:** IEEE Sensors Journal if the integrated device contribution is within scope; IEEE Transactions on Haptics for a substantial tactile-interface/perception result if strict JCR Q1 is not mandatory. The latter is a strong specialist venue, but the verified 2024 category is Q2.

### Route D — user-centered accessibility or rehabilitation

If the strongest result concerns independence, communication success or gaze-based access in a particular population, prioritize those findings over network details.

**Targets:** ACM Transactions on Accessible Computing for accessible interaction research; IEEE Transactions on Neural Systems and Rehabilitation Engineering for a substantial rehabilitation/assistive-interface contribution supported by appropriate evaluation. Naming Parkinson's or ALS in the system description does not establish a rehabilitation contribution.

## 7. Publication shortlist and ranking corrections

Quartiles belong to a **database, metric, subject category and year**. JCR JIF quartiles, JCR AIS/JCI quartiles, CiteScore quartiles and SCImago SJR quartiles are not interchangeable. Below, 2024 means the metric year, not a promised 2026 ranking. Some public official pages still show older metrics; the record must be rechecked for the submission year's institutional rule.

| Priority / route | Venue | Verified ranking evidence | Why target it / condition |
|---|---|---|---|
| **1 — provisional practical Q1** | **IEEE Sensors Journal** | JCR JIF **Q1, Instruments & Instrumentation, 2024**; other 2024 categories include Q2. [Institutional record](https://zaguan.unizar.es/record/130017). Official page displays IF 4.5 without a current metric year beside that value. | Scope covers sensor systems, fusion, interfaces and actuators. Needs a substantive device/sensing contribution with controlled experiments. [Official scope](https://ieee-sensors.org/ieee-sensors-journal/). |
| **2 — full-platform stretch** | **IEEE Internet of Things Journal** | JCR JIF **Q1, Telecommunications, 2024** in the [institution-hosted JCR table](https://cercetare.ubbcluj.ro/wp-content/uploads/2018/02/JCR_iunie_2025.pdf). Official page explicitly reports **8.9 for 2024**, rather than the brain's 8.2 for 2023. | Good topical match for architecture, protocols and testbeds; needs a generalizable contribution beyond assembling MQTT and translation libraries. [Official scope](https://ieee-iotj.org/). |
| **3 — rehabilitation route** | **IEEE Transactions on Neural Systems and Rehabilitation Engineering** | JCR JIF **Q1, Rehabilitation and Engineering, Biomedical, 2024**. [Hospital-library JCR table](https://chospab.es/biblioteca/DOCUMENTOS/factor_impacto/2024.pdf). | Appropriate if improved assistive access or rehabilitation engineering is central and rigorously evaluated. [Editorial scope](https://www.embs.org/tnsre/for-reviewers/editorial-policy/). |
| **Specialist accessibility option** | **ACM Transactions on Accessible Computing** | JCR JIF **Q2, Computer Science, Interdisciplinary Applications, 2024**, in the [JCR table, PDF page 135](https://cercetare.ubbcluj.ro/wp-content/uploads/2018/02/JCR_iunie_2025.pdf#page=135). | Strong audience match for user-centered cross-modal communication. Not a verified strict-JCR-Q1 fallback. [Journal](https://dl.acm.org/journal/taccess). |
| **Specialist tactile option** | **IEEE Transactions on Haptics** | JCR **Q2, Computer Science, Cybernetics, 2024**. [University library record](https://topj.lib.whu.edu.cn/show.asp?cat=sci&id=4275). | Relevant for tactile displays, device control and haptic perception; requires a real haptics contribution. [Official scope](https://www.ieee-ras.org/publications/toh/). |
| **Applied systems fallback** | **IEEE Access** | University records list JCR **Q2** and SJR **Q1**, including 2025. [Institutional record](https://www.iit.comillas.edu/publicacion/info_revista/en/457/IEEE_Access). | Broad applied systems fit after complete validation. It is not interchangeable with strict JCR Q1. [Official scope](https://ieeeaccess.ieee.org/). |
| **Applied sensing fallback** | **Sensors (MDPI)** | Publisher currently lists JCR **Q2** in its three categories and CiteScore **Q1**; current page associates IF 4.0 with 2025. [Publisher statistics](https://www.mdpi.com/journal/sensors/stats), [journal page](https://www.mdpi.com/journal/Sensors). | Relevant to an evaluated assistive sensing system, but fails a strict JCR-Q1 requirement on the listed categories. |

Ranking evidence is public publisher/institutional reporting, not a direct authenticated Clarivate export for every journal. In particular, do not label all rows “current Q1.” The appropriate final ranking certificate is the institution's accepted database record.

The old file's blanket Q1 labels for Scientific Reports and Assistive Technology are not adopted here. Neither is automatically a suitable fallback; scope, category and year must be checked against the final manuscript. Conference quality is also separate from journal quartiles: ACM ASSETS is a relevant accessibility venue, but it does not satisfy a requirement for a Q1 journal article.

### Fees and timing

Do not choose on advertised review speed or assume acceptance within a stated median. IoT Journal's current [author page](https://ieee-iotj.org/guidelines-for-authors/) offers traditional publication without an OA APC, or optional paid open access; other charges can apply. IEEE Access is fully OA and currently lists **US$2,160 plus applicable taxes** on its [APC page](https://ieeeaccess.ieee.org/about/article-processing-charges/). Obtain the applicable fee quote and funding/waiver terms before final selection; eligibility has not been assumed.

## 8. Experiments that would make the paper reviewable

This is a proposed evaluation plan, not a report of completed experiments. Adapt it to the already-built system and the central contribution chosen above.

### 8.1 Freeze the research artifact first

Archive the firmware actually running on each board, all Pi/hub software, exact board/OS/library versions, model weights, label maps, calibration files, CAD, wiring diagrams and BOM. Record a commit/release identifier. Separate calibration experiments from final held-out evaluation. Add a capability matrix stating what is demonstrated, tested or out of scope.

### 8.2 End-to-end communication

Measure:

- Input onset → message accepted; input completion → message accepted.
- Message accepted → output starts; output starts → output completes.
- User task completion, correct message comprehension, correction requests and failed attempts.
- Delivery/duplication/ordering errors, reconnect recovery, offline expiry and maximum queue size.
- Median, p95/p99 where enough samples exist, variability and confidence intervals; provide trial counts.

Use local and real two-hub remote setups. Test multiple message lengths and output speeds, network delay/loss/disconnection, and increasing numbers of active modules. Label simulated clients separately from real devices. For cross-host timestamps, document synchronization error or use a suitable round-trip/externally observed timing method.

**Baseline:** conventional MQTT text forwarding using identical encoders/renderers. Add ablations that remove the proposed delivery feature. An artificial broken baseline would not support the contribution. “Exactly once at MQTT” must be distinguished from “once rendered correctly.”

### 8.3 Glove/vision recognition

- Specify the actual sign language and task; do not mix ASL, Bangla Sign Language and arbitrary gestures in a single unlabeled “accuracy.”
- Compare glove-only, vision-only, conventional fusion and the proposed method under the same vocabulary/data budget.
- Split by **participant and recording session before window extraction**, preventing overlapping windows or one person's recordings from leaking across train/test sets.
- Test held-out signers, motion speed, lighting, occlusion, sensor drift/dropout and irrelevant/no-sign input.
- Report macro-F1, per-class recall, confusion matrix, false activations, uncertainty/rejection behavior and calibration burden. Continuous translation needs sequence-level evaluation and human comprehension, beyond classification accuracy.
- Measure full pipeline latency, FPS, RAM, model size, power and thermal behavior on the real edge board; benchmark model inference separately from feature extraction.

### 8.4 Braille hardware and reading

- Verify all **64 commanded six-dot patterns** and relevant transitions in every cell; include repeated trials under finger loading and after wear.
- Measure raised/retracted pin height, spacing, holding force, backlash, repeatability, settling time, unintended intermediate states and failure rates.
- Compare one-cell and four-cell operation, per-cell calibration versus shared calibration, fixed pacing versus reader-controlled pacing if available.
- Report energy per update, hold/idle power, peak simultaneous current, enclosure size, component/manufacturing cost and serviceability.
- Evaluate letters, words and realistic messages with Braille readers. Report reading accuracy, speed, comprehension, fatigue and preferences. A sighted person recognizing a visual dot pattern is not a tactile reading evaluation.
- Include a relevant two-actuator mechanical baseline and a functionally comparable conventional display where accessible. Do not compare a large, slow single cell with a commercial multi-cell display on price alone.

### 8.5 Gaze and voice

Gaze: quantify target-selection error, accidental activation, text-entry rate, corrected/uncorrected errors, calibration time and drift. Compare fixed dwell/no smoothing with each claimed adaptation. Record lighting/head-motion conditions. A combined smoothing-plus-dwell comparison alone cannot isolate the cause of improvement.

Voice: measure received audio integrity, speech recognition word error rate, ambient-noise/accent conditions, endpointing delay and TTS intelligibility. Use the exact language/accent/model deployed. A transcription result must not be presented as a sign-language translation result.

### 8.6 Participant-study design

For a cross-disability communication claim, include communication partners with relevant access needs and test both directions. All six design profiles can remain in the architecture, but participant claims must match the groups actually evaluated. Include people with Parkinson's only in outcome claims supported by that evaluation.

Choose sample size from the primary outcome, expected variability, recruitment constraints and appropriate statistical design. **Five participants across six profiles is not a universal journal acceptance threshold**, and repeated trials are not independent extra participants. A small exploratory study can be useful if clearly labeled and not generalized beyond its evidence.

Use participant-appropriate consent and the institution's research-ethics process before prospective participant testing. Report the actual approval/exemption and protocol accurately; existing informal demonstrations are not automatically a completed approved study. Provide accessible instructions, breaks and compensation. Combine objective task outcomes with interviews; SUS/NASA-TLX alone cannot establish communication effectiveness.

### 8.7 Cost and resource comparison

Use receipts or dated supplier quotations for the actual hardware, including camera, display, power, enclosure, cabling, manufacturing and shared infrastructure. Report:

`per-user deployed cost = personal module + allocated hub/infrastructure cost + allocated recurring cost`

Show the allocation as a function of users per hub. A remote pair using separate hubs has different economics from a classroom sharing one. Commercial comparison should state cell count, size, warranty/support, input features and intended use. The old dollar estimates and claimed low servo holding consumption are hypotheses until measured.

## 9. Manuscript structure

1. **Introduction:** A concrete communication barrier, a bounded gap, and two or three contributions.
2. **Related work:** Closest accessible communication systems, two-actuator Braille mechanisms, multimodal recognition, and the selected contribution's baselines.
3. **Requirements and system:** Users/modalities, trust boundaries, deployment assumptions, implemented hardware/software and language support.
4. **Proposed contribution:** Algorithm, mechanism, protocol or interaction method with enough detail to reproduce.
5. **Experimental methods:** Participants/datasets, splits, comparators, hardware, measurement tools and analysis plan.
6. **Results:** Component performance followed by end-to-end communication and participant outcomes, including failures.
7. **Discussion:** What the findings establish, where they fail, deployment cost, accessibility trade-offs and limitations.
8. **Conclusion:** Findings supported by the reported results.

Recommended figures: actual architecture with trust boundaries; photographed prototype and dimensioned mechanism; method diagram; latency breakdown; accuracy/energy trade-off; communication-task results. Recommended tables: closest-work comparison, reproducible implementation/BOM, modality-path coverage, and evaluated outcomes. Do not fill these with targets disguised as results.

## 10. Next actions, in order

1. **Synchronize the research checkout with the working hardware.** The source reviewed here is incomplete relative to the owner-reported build.
2. **Resolve or document the source findings** against those deployed versions: Braille mapping/chord capture, build targets/board pins, audio packet sizing, timing, calibration and message delivery.
3. **Select the central contribution from existing evidence.** Start with Sensors Journal only if sensing/device experiments support it; choose IoT Journal only if a meaningful systems advance exists.
4. **Build the reproducible evidence package:** code/CAD/model versions, raw anonymized measurements, analysis scripts, ethics documentation and protocol.
5. **Run contribution-specific baselines and participant tasks.** Derive claims from results.
6. **Rewrite the novelty and journal sections of the brain/plan/anti memo** after agreement on the evidence. This audit leaves the original files intact.
7. **Draft and internally review the manuscript**, verify the institution's accepted Q1 category/year, and submit to the best-fitting venue. No submission or contact with a journal was performed.

## 11. Verification and limits of this audit

Machine-readable results are in [AUDIT_CHECKS.json](AUDIT_CHECKS.json); the [inventory](FILE_INVENTORY.csv) identifies the inspected file versions.

- Enumerated and read all 56 original project files, including ignored planning/model/data README files; 45 Markdown files, six Arduino sketches, and five other configuration/dependency files.
- Compared the serial/demo alphabet to the network decoder: **25/26 mismatches**.
- Compared the two calibrated single-cell versions after whitespace removal: **equivalent**.
- Confirmed four independent pairs of Arduino entry points in one sketch folder.
- Calculated audio payload rate/size, ADC conversion lower bound, UART capacity and configured rendering delays directly from the source.
- Parsed the AMP JSON; schema parsing alone is not behavioral or security validation.
- Found no Python implementation, executable tests, datasets, trained models, CAD or measured research results in the local project.
- Neither `arduino-cli` nor PlatformIO is installed in the inspected environment. **No firmware compilation, flashing, hardware testing, ML benchmark or participant evaluation was performed.**
- Existing user changes were present before this audit (`paper/` untracked and two tracked deletions); they were preserved. No firmware or original planning document was edited.
- Public literature search established relevant overlap; it does not prove an exhaustive novelty gap. Retain search strings, database exports and inclusion decisions in the eventual formal literature review.

The search used the public web on the audit date, with queries including `remote communication deaf blind multimodal braille sign language assistive system`, `two servo cam refreshable braille display mechanism`, `cross-disability communication system remote`, and `hybrid sign language glove vision fusion 2024 2025`, followed by exact-title and publisher searches. Journal checks used official scope/author pages and public institutional ranking records. This process did not directly search authenticated IEEE Xplore, Scopus or Web of Science databases; it does not reproduce the earlier memo's claimed 40-paper review.

**Final judgment:** Ability is a credible platform for an assistive-technology research program. Its strongest paper will come from a measured improvement in sensing, communication, tactile interaction or user access. The current five broad novelty assertions should not be carried into a Q1 manuscript unchanged.
