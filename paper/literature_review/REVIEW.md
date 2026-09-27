# AMP: literature review, novelty assessment and publication plan

**Working title:** AMP: A Cross-Disability Remote Communication Protocol for Heterogeneous Assistive IoT Systems  
**Review cutoff:** 15 September 2026. Primary literature window: 2020–cutoff, with older directly relevant prior art.  
**Purpose:** Decide what can defensibly become an original research contribution for IEEE Internet of Things Journal. This is a structured, critical scoping review, not an exhaustive systematic review or a report of completed AMP experiments.

## 1. Search log — read before the novelty verdict

The complete retained log is [SEARCH_LOG.csv](SEARCH_LOG.csv). It records exact queries/URLs, access date, available inspection information and retained-source identities where recoverable. The original query records are preserved in [search_sessions.json](search_sessions.json). All sixteen requested clusters A–P were searched. Supplemental searches pursued AAC, remote tactile communication, output acknowledgments, standards, recovery, scheduling and journal scope.

| Search ID | Database/source | Search query | Date | Results inspected | Relevant studies retained |
|---|---|---|---|---|---|
| A | General web discovery | `"cross-disability" "communication" protocol OR system` | 2026-09-15 | Not recorded / unavailable | Per-query count not recorded; screened sources in core table |
| B | General web discovery | `deafblind remote bidirectional communication system Braille speech 2020 2026` | 2026-09-15 | Not recorded / unavailable | See core table |
| C | General web discovery | `W3C multimodal architecture distributed lifecycle DoneNotification pause resume` | 2026-09-15 | Not recorded / unavailable | See core table |
| H | General web discovery | `presentation acknowledgment displayed XMPP message markers W3C multimodal` | 2026-09-15 | Not recorded / unavailable | See core table |
| I | General web discovery | `receiver driven flow control Reactive Streams AMQP slow consumers backpressure` | 2026-09-15 | Not recorded / unavailable | See core table |
| CR series | Crossref API + DOI resolver | Exact DOI API URLs in CSV | 2026-09-15 | One metadata record per lookup; resolver attempted | Individual reference IDs in CSV |

**Database access statement.** Crossref was directly queried. IEEE Xplore, ACM DL, ScienceDirect, SpringerLink, Wiley, journal sites, arXiv, W3C, IETF/RFC, OASIS, ISO and XMPP resources were discovered through web search and/or opened as individual records. Their internal literature-search databases were not systematically searched. **Scopus, Web of Science, Google Scholar, Semantic Scholar and PubMed were not directly searched.** No claim of database-complete coverage is made. Publisher access sometimes returned 403 or an interstitial; Crossref identity checks are distinguished from full-text access.

**Reproducibility limit.** Search discovery results were not exported as a complete deduplicated screening set. Some adaptive follow-up searches were not retained verbatim. The log therefore reproduces the recorded search strategy and metadata checks, not every ranked result or a PRISMA flow. Exact total hits, exclusions and per-query retained counts are unavailable. This limits the confidence of absence-based novelty claims. Before a manuscript submission, supplement with a librarian-assisted Scopus/WoS search, backward/forward citation tracking and full-text retrieval of the closest abstract-only studies.

## 2. Screening criteria and near-match decisions

**Include:** verified sources that directly address remote assistive communication, multimodal architecture, accessibility preferences, capability discovery, semantic mediation, output lifecycle, pacing/backpressure, recovery or relevant human communication practices. Include foundational standards even when older than 2020. Keep research evidence and specifications in different ID series.

**Exclude:** diagnosis-only applications, unrelated monitoring, recognition-only models without a communication architecture, unverifiable search leads and duplicate versions. A source can remain as contextual evidence without being a closest protocol competitor. No claim about an algorithm is extracted from a title alone.

| Near match / candidate | Decision | Reason and consequence |
|---|---|---|
| MIO arXiv version / IEEE HAPTICS paper [P05](https://doi.org/10.1109/HAPTICS52432.2022.9765570) | One retained work | Published DOI verified; author manuscript supplies examined text. Do not count twice. |
| Tang and Piper arXiv / DIS 2026 [P12](https://doi.org/10.1145/3800645.3813075) | One retained work | Published DIS DOI and manuscript header agree. Cite the conference work, not an unverified peer-review status. |
| PARLOMA [P01](https://doi.org/10.5772/60416) and later cloud robotics [P19](https://doi.org/10.1007/978-3-319-94277-3_33) | Retain both, link lineage | Distinct publications, related research programme; not independent replications. |
| Adaptive Periodic Communication over MQTT for Large-Scale Cyber-Physical Systems, DOI `10.1109/CPSNA.2015.21` | Exclude from analytical core; metadata lead retained | Existence verified, but no adequate abstract/method evidence examined. Title cannot establish its control algorithm. P18 intentionally unused. |
| AAC teleintervention review [P20](https://doi.org/10.1111/dmcn.15387) | Context only | Remote training/intervention is not equivalent to interoperability between conversational endpoints. |
| Educational AAC case [P21](https://doi.org/10.36253/form-10189) | Context only | Supports participation context, not new delivery guarantees. |
| CLAID [P17](https://doi.org/10.1016/j.future.2024.05.026) | Infrastructure comparator | Edge–cloud interoperability is relevant; it does not establish an accessible conversation result. |
| Snapshotting preprint [P23](https://arxiv.org/abs/1506.08603) | Foundational mechanism comparator | Clearly labeled preprint; digital-state recovery cannot be equated with human perception. |
| Generic sign/Braille classifiers in discovery | Excluded by category | Recognition accuracy alone does not answer the protocol question; no fabricated paper-level exclusion count. |
| Unverified phrase searches such as “Designing Remote Communication Tools for AAC Users” | Not treated as publications | Search phrases are not bibliographic evidence; near-title Crossref matches were not silently adopted. |

## 3. Verified core literature

The [21-column core table](CORE_LITERATURE.csv) contains **46 sources: 26 research items and 20 standards/specifications**. The research items include a clearly labeled preprint and a workshop paper whose review procedure was not verified. This is **not 46 peer-reviewed studies**, nor 46 directly equivalent systems.

Each row includes problem, method, modalities, remote operation, capability awareness, cross-modal operation, bidirectionality, presentation tracking, pacing, recovery, human evaluation, findings, limitations, relevance, authoritative link and evidence level. [SOURCE_ASSESSMENTS.json](SOURCE_ASSESSMENTS.json) preserves annotations and evidence locators. [REFERENCES.md](REFERENCES.md) supplies full bibliographic records and uncertainty notes.

**Evidence levels:** “Full text” means relevant technical/method/result sections were examined, sometimes through an author version or publisher-rendered text. It does not mean every equation or reference was independently replicated. “Abstract only” means **assessment based on abstract only**; detailed unreported functions remain unknown. Metadata-only scheduling leads were excluded from the analytical core.

**Critical correction:** [P03](https://doi.org/10.7160/eriesj.2024.170206) did not recruit Deafblind participants. Its trials include sighted/hearing participants and simulated impairment. It is evidence for a prototype and conversion direction, not demonstrated effectiveness for Deafblind students. The matrix marks human evaluation present only with this qualification.

## 4. Fifteen closest systems and frameworks

“Missing” below means not established in examined evidence, never that the prior system is incapable. The last column evaluates a **proposed difference**, not a proven AMP result. Standards are included because they can invalidate a protocol novelty claim even without an assistive user study.

| Prior work / what it does | Shared features | Prior work has that AMP research artifacts lack | Missing relative to AMP proposal / potential difference | Scientific significance |
|---|---|---|---|---|
| PARLOMA [P01](https://doi.org/10.5772/60416): networked tracking and robotic tactile output | Remote tactile communication; hardware abstraction | Published integrated prototype and tactile evaluation | General endpoint contract and interrupted-presentation state not established | Wider adapters alone are integration; recovery semantics might matter |
| Publish-subscribe assistance [P02](https://doi.org/10.1016/j.ifacol.2016.11.145) | MQTT, remote requests/responses, heterogeneous assistance providers | Published assistance architecture | Negotiated assistive renderer state not verified from abstract | Application-domain extension is incremental unless a mechanism differs |
| Braille–speech prototype [P03](https://doi.org/10.7160/eriesj.2024.170206) | Bidirectional cross-modal communication | Published prototype trials and conversion details | Remote lifecycle, pacing and restart recovery not established | Networking alone is implementation; state consistency could be meaningful |
| AACrobat [P04](https://doi.org/10.1145/2998181.2998215) | Gaze, text/speech, distributed devices, partner coordination | User autonomy controls and participant evaluation | Durable presentation progress across failures not established | Potentially meaningful reliability extension; generic session state is not new |
| MIO [P05](https://doi.org/10.1109/HAPTICS52432.2022.9765570) | Tactile output, customization and bidirectional input/output | Deafblind participant evidence and concrete local interaction design | Remote protocol and crash-recovery contract not evaluated | Remote connection is integration; accessible recovery needs evidence |
| SymbolChat [P06](https://doi.org/10.1016/j.intcom.2012.06.003) | Remote messaging and individual access adaptation | Published field study | Detailed negotiated lifecycle not verified from abstract | Uncertain until full text obtained; customization claim overlaps strongly |
| INTER-IoT/CasAware [P13](https://doi.org/10.3233/AIS-200578) | Heterogeneous IoT, semantic mediation | Published integration architecture/case | Human presentation state and conversational recovery not reported in examined sections | Assistive specialization alone is incremental |
| Flexible-interface/URC/GPII comparison [P15](https://doi.org/10.3233/AIS-160370) | Pluggable interfaces, personal preferences | Established frameworks and comparative analysis | Joint conversational pacing/recovery not established | Capability/preference reuse is expected infrastructure |
| Multilayer edge multimodal architecture [P16](https://doi.org/10.3390/s24165199) | Edge components and multimodal applications | Published deployment architecture | Detailed presentation/recovery behavior not verified from abstract | “Runs at the edge” is implementation, not an adequate difference |
| Cloud robotics communication [P19](https://doi.org/10.1007/978-3-319-94277-3_33) | Remote assistive endpoints and conversion | Published cloud robotics design | General negotiated renderer state not verified | Endpoint breadth is integration; partial recovery remains uncertain |
| Motion-tracking communication glove [P22](https://doi.org/10.24251/HICSS.2022.259) | Glove-mediated communication and remote messaging | Published specialized glove architecture | Durable multi-output lifecycle not verified from abstract | Must retrieve full text before making a strong exclusion claim |
| EMBASSI output modeling [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf) | Capability/context-aware output planning and completion feedback | Explicit presentation-completion acknowledgment precedent | Durable partial assistive rendering across disconnections not established | Generic presentation ACK novelty fails; narrower recovery might survive |
| W3C MMI [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/) | Distributed modality components, sessions, lifecycle events | Published architecture and normative event semantics | Specific assistive checkpoint/pacing policy requires an application binding | A wrapper is integration; evaluate whether extension changes guarantees |
| AMQP 1.0 [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf) | Receiver credit, acknowledgment, expiry and recovery | Mature protocol specification for these mechanisms | Physical presentation offsets are different from transfer offsets | Strongest composable reliability baseline; high novelty threat |
| Real-time text over WebRTC [S13](https://www.rfc-editor.org/rfc/rfc8865.html) | Remote conversation, negotiation and receiver rate limits | Standardized receiver maximum character rate | Cross-renderer semantic progress and recovery not specified | Rate-awareness claim fails; broader state interaction may be incremental |

## 5. Feature matrix

The [24-feature matrix](FEATURE_MATRIX.csv) compares these fifteen sources and **AMP — PROPOSED — NOT PRIOR ART**. [Matrix interpretation](FEATURE_MATRIX.md) records important caveats.

- **✓ Confirmed:** supported in examined evidence; for a specification, this means specified functionality.
- **△ Partial / unclear:** incomplete support, broader framework support, or insufficient detail.
- **✗ Not reported:** not identified in examined evidence. It does not mean absent from every implementation.

AMP cells are △ because this review does not possess an evaluated AMP implementation. The owner reports all hardware modules and the hub built/tested; that does not supply evidence for every proposed protocol feature. No checkmark total is used as a novelty score. A composed implementation of several standards is a stronger comparator than any single row.

## 6. Twelve-theme critical synthesis

### 6.1 Accessible remote communication

PARLOMA, publish-subscribe assistance and SymbolChat establish several forms of remote accessible communication [P01](https://doi.org/10.5772/60416), [P02](https://doi.org/10.1016/j.ifacol.2016.11.145), [P06](https://doi.org/10.1016/j.intcom.2012.06.003). Their different populations and interfaces make a single universal performance comparison inappropriate. AMP must specify supported languages, representations and tasks; adding geographical distance does not establish novelty.

### 6.2 Cross-disability and mixed-ability interaction

Ability-diverse collaboration is already a substantive HCI field [P09](https://doi.org/10.1145/3613904.3641930). Virtual teamwork and collective-access studies show that access involves preferences, collaboration practices and sometimes conflicting needs [P10](https://doi.org/10.1145/3772318.3790419), [P12](https://doi.org/10.1145/3800645.3813075). Device compatibility is necessary but insufficient evidence of successful communication between people.

### 6.3 Multimodal architectures

The W3C framework separates interpretation, interaction management and presentation [S02](https://www.w3.org/TR/mmi-framework/). Its later architecture specifies distributed component lifecycles [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/). EMBASSI adds a particularly direct precedent for output completion feedback [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf). AMP should explain precisely which existing event contract fails in its target condition.

### 6.4 Assistive interoperability

URC/GPII-related work and the ISO remote-console framework address replaceable interfaces and adaptation [P15](https://doi.org/10.3233/AIS-160370), [S17](https://www.iso.org/standard/60821.html). AACrobat demonstrates interaction across a gaze device and mobile companion [P04](https://doi.org/10.1145/2998181.2998215). A larger collection of ESP32 modules is useful engineering, but interoperability research requires demonstrated compatibility boundaries and behavior under change.

### 6.5 Capabilities and preferences

WoT descriptions/discovery cover machine-facing service capabilities [S04](https://www.w3.org/TR/wot-thing-description11/), [S05](https://www.w3.org/TR/wot-discovery/). ISO personal-needs preferences and SIP language negotiation cover different parts of user-facing matching [S18](https://www.iso.org/standard/43603.html), [S14](https://www.rfc-editor.org/rfc/rfc8373.html). These should not be collapsed into diagnosis labels. A capability description does not by itself encode consent to a fallback route.

### 6.6 Representation and transformation

EMMA and semantic IoT middleware precede a shared message representation [S03](https://www.w3.org/TR/emma/), [P13](https://doi.org/10.3233/AIS-200578), [P14](https://doi.org/10.1002/ett.4544). Text can be an implementation choice, but a transcription is not automatically equivalent to a signed language or a complete semantic representation. Information-transformation research reinforces the need to examine what conversions preserve and what work they impose [P11](https://doi.org/10.1145/3772318.3790872).

### 6.7 IoT middleware and edge execution

IoT middleware research addresses heterogeneity, composition and adaptation [P25](https://doi.org/10.1109/JIOT.2016.2615180). INTER-IoT, CLAID and multimodal edge architecture supply concrete comparison domains [P13](https://doi.org/10.3233/AIS-200578), [P17](https://doi.org/10.1016/j.future.2024.05.026), [P16](https://doi.org/10.3390/s24165199). MQTT and Raspberry Pi usage establish neither a new communication theory nor measurable scientific benefit.

### 6.8 Receipt, presentation and understanding

XMPP distinguishes client receipts from displayed markers [S19](https://xmpp.org/extensions/xep-0184.html), [S09](https://xmpp.org/extensions/attic/xep-0333-1.0.0.html). MMI and EMBASSI show output lifecycle/completion precedents [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/), [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf). An AMP renderer may report that it actuated pins or finished audio; neither event proves that a person perceived or understood the message. A human acknowledgment must remain a distinct event.

### 6.9 Receiver pacing and backpressure

TCP windows, AMQP credit and Reactive Streams demand directly challenge generic receiver-pacing novelty [S15](https://www.rfc-editor.org/rfc/rfc9293.html), [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S08](https://github.com/reactive-streams/reactive-streams-jvm/blob/v1.0.4/README.md). More specifically, RFC 8865 negotiates receiver character rate for conversational text [S13](https://www.rfc-editor.org/rfc/rfc8865.html). AMP needs to test an advantage over feedback tied to measured presentation service time, not merely a fast sender with no control.

### 6.10 Queueing, deadlines and recovery

Deadline-aware allocation, MQTT expiry, disruption-tolerant bundles and checkpointing already cover substantial scheduling/recovery ground [P24](https://doi.org/10.1145/2018436.2018443), [S06](https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html), [S11](https://www.rfc-editor.org/rfc/rfc9171.html), [P23](https://arxiv.org/abs/1506.08603). The unresolved candidate is at the physical-output boundary: an interrupted tactile or spoken segment may have been partly perceived even when durable state does not establish completion. That is a proposed problem formulation, not proof that prior literature lacks solutions.

### 6.11 Conversation continuity and fallback

Chat states and language negotiation already expose conversational context [S20](https://xmpp.org/extensions/xep-0085.html), [S14](https://www.rfc-editor.org/rfc/rfc8373.html); AACrobat includes partner-oriented activity visibility [P04](https://doi.org/10.1145/2998181.2998215). Collective-access evidence cautions against assuming an available output is an acceptable replacement [P12](https://doi.org/10.1145/3800645.3813075). AMP can investigate continuity under consent-constrained changes, but cannot claim ordinary session state as new.

### 6.12 Evaluation with intended users

MIO and AACrobat provide examples of user-centered empirical work [P05](https://doi.org/10.1109/HAPTICS52432.2022.9765570), [P04](https://doi.org/10.1145/2998181.2998215). Support-person studies and teleintervention reviews supply different evidence, with different inferential limits [P07](https://doi.org/10.3233/TAD-190254), [P20](https://doi.org/10.1111/dmcn.15387). P03's surrogate trials illustrate why engineering function and effectiveness for intended users must be reported separately [P03](https://doi.org/10.7160/eriesj.2024.170206). A comparison of device-mediated and human-articulated tactile fingerspelling also reports intelligibility differences, reinforcing the need to measure comprehension separately from actuation [P27](https://doi.org/10.1080/10400435.2024.2369547).

## 7. Component novelty: N1–N14

| Component | Classification | Evidence and judgment |
|---|---|---|
| N1 Cross-disability remote communication | Previously demonstrated | Remote assistive systems and mixed-ability research predate AMP [P01](https://doi.org/10.5772/60416), [P06](https://doi.org/10.1016/j.intcom.2012.06.003), [P09](https://doi.org/10.1145/3613904.3641930). Exact modality pairs differ. |
| N2 Heterogeneous assistive endpoints | Established | Distributed AAC and remote interface frameworks [P04](https://doi.org/10.1145/2998181.2998215), [P15](https://doi.org/10.3233/AIS-160370), [S17](https://www.iso.org/standard/60821.html). |
| N3 Capability rather than diagnosis routing | Established as a principle; partially explored for exact route | Functional needs/preferences and capability descriptions already exist [S18](https://www.iso.org/standard/43603.html), [S04](https://www.w3.org/TR/wot-thing-description11/), [S14](https://www.rfc-editor.org/rfc/rfc8373.html). |
| N4 Intermediate semantic representation | Established | EMMA and semantic middleware [S03](https://www.w3.org/TR/emma/), [P13](https://doi.org/10.3233/AIS-200578), [P14](https://doi.org/10.1002/ett.4544). Plain text does not become novel by being placed in JSON. |
| N5 Cross-modal route negotiation | Partially explored | Output planning, language negotiation and service composition overlap [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf), [S14](https://www.rfc-editor.org/rfc/rfc8373.html), [P14](https://doi.org/10.1002/ett.4544). Exact assistive route handshake remains insufficiently characterized. |
| N6 User preference constraints | Established | Personal preference infrastructure and user-controlled collaboration [S18](https://www.iso.org/standard/43603.html), [P15](https://doi.org/10.3233/AIS-160370), [P04](https://doi.org/10.1145/2998181.2998215). |
| N7 Presentation-aware lifecycle | Previously demonstrated | MMI component lifecycle and display-state conventions [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/), [S09](https://xmpp.org/extensions/attic/xep-0333-1.0.0.html). |
| N8 Presentation-aware ACK | Previously demonstrated | EMBASSI completion feedback and displayed markers [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf), [S09](https://xmpp.org/extensions/attic/xep-0333-1.0.0.html). |
| N9 Receiver-paced admission/control | Established | Receiver credit/demand and negotiated text rate [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S08](https://github.com/reactive-streams/reactive-streams-jvm/blob/v1.0.4/README.md), [S13](https://www.rfc-editor.org/rfc/rfc8865.html). |
| N10 Presentation-time backlog | Insufficient evidence for a distinct new algorithm | Service-work estimates are an ordinary scheduling construction; deadlines and rate limits are established [P24](https://doi.org/10.1145/2018436.2018443), [S13](https://www.rfc-editor.org/rfc/rfc8865.html). A new estimator/control policy must earn its claim experimentally. |
| N11 Interruption/partial-presentation recovery | Partially explored; exact boundary underexplored in reviewed set | Transfer/state recovery is established [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S11](https://www.rfc-editor.org/rfc/rfc9171.html), [P23](https://arxiv.org/abs/1506.08603); cross-renderer physical-progress guarantees were not identified here. |
| N12 Approved fallback negotiation | Partially explored | Preference/output planning and access conflict evidence overlap [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf), [P15](https://doi.org/10.3233/AIS-160370), [P12](https://doi.org/10.1145/3800645.3813075). Consent-bound recovery needs a precise contract. |
| N13 Bidirectional conversational state | Established | Chat state signaling and AAC partner coordination [S20](https://xmpp.org/extensions/xep-0085.html), [P04](https://doi.org/10.1145/2998181.2998215). |
| N14 Unified protocol | Potentially novel combination; significance unproven | Integration of existing mechanisms is the default explanation until a composed baseline is defeated. |

“Underexplored” is scoped to examined evidence. It is not a claim of global absence. Neither a new acronym nor the number of combined features changes this assessment.

## 8. Combination novelty

| Combination | Closest evidence / overlap | Remaining proposed difference | Scientific significance |
|---|---|---|---|
| A: cross-modal + capabilities + remote endpoints | MMI, URC, remote tactile systems and distributed AAC [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/), [P15](https://doi.org/10.3233/AIS-160370), [P01](https://doi.org/10.5772/60416), [P04](https://doi.org/10.1145/2998181.2998215) | Unified assistive endpoint contract | Mostly integration unless compatibility results expose a general unsolved constraint |
| B: cross-modal + presentation ACK | EMBASSI output planning/completion; MMI [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf), [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/) | Durable fine-grained renderer progress | Broad combination already substantially overlaps; granularity alone is weak novelty |
| C: presentation state + pacing + heterogeneous speeds | MMI lifecycle, AMQP credit, receiver text rate [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/), [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S13](https://www.rfc-editor.org/rfc/rfc8865.html) | Policy driven jointly by remaining presentation work and recovery state | Candidate incremental mechanism; must beat tuned service-aware credit |
| D: route negotiation + preferences + fallback | Semantic composition, preference frameworks and output planning [P14](https://doi.org/10.1002/ett.4544), [P15](https://doi.org/10.3233/AIS-160370), [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf) | Explicit consent/version contract during partial presentation | Potentially meaningful behavioral guarantee; implementation-level unless difficult interaction is demonstrated |
| E: intermittency + partial accessible presentation + recovery | Bundles, AMQP resume and computational checkpoints [S11](https://www.rfc-editor.org/rfc/rfc9171.html), [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [P23](https://arxiv.org/abs/1506.08603) | Expose uncertainty after physical output and resume at aligned semantic boundaries | Strongest candidate problem in reviewed set; novelty confidence still low |
| F: all mechanisms in one bidirectional protocol | Composition of MMI + capabilities + durable messaging + credit + checkpoints | Joint contract and policy across pacing, route changes and nonrollbackable output | No directly equivalent evaluated system identified here; absence is not proof. Research value depends on an advantage over that composition. |

## 9. Research-gap table

| Research gap | Evidence | Supporting sources | Contradicting evidence / threat | Confidence | Potential AMP contribution |
|---|---|---|---|---|---|
| Consistent recovery after partly rendered assistive content | Output lifecycle and computational/transfer recovery are separately described | [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/), [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [P23](https://arxiv.org/abs/1506.08603), [P05](https://doi.org/10.1109/HAPTICS52432.2022.9765570) | Existing checkpoint+application ACK composition may already suffice | Moderate that the engineering problem matters; Low that a new protocol is necessary | Explicit uncertain state and semantic resume contract |
| Admission accounting for unconfirmed presentation work after restart | Receiver rate control exists; transfer credit does not define a tactile reading cursor | [S13](https://www.rfc-editor.org/rfc/rfc8865.html), [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S08](https://github.com/reactive-streams/reactive-streams-jvm/blob/v1.0.4/README.md) | Service-aware adaptive credit is an obvious strong alternative | Low | Joint progress/credit policy tested against equal-information baseline |
| Preference-consistent fallback while preserving progress | Preferences and conflicting access requirements are documented | [S18](https://www.iso.org/standard/43603.html), [P12](https://doi.org/10.1145/3800645.3813075), [P04](https://doi.org/10.1145/2998181.2998215) | Existing multimodal planners already use preferences | Moderate for need; Low for algorithmic novelty | Versioned consent and route transition invariants |
| Human benefit of protocol-level recovery | Accessible communication studies evaluate different settings and populations | [P05](https://doi.org/10.1109/HAPTICS52432.2022.9765570), [P07](https://doi.org/10.3233/TAD-190254), [P10](https://doi.org/10.1145/3772318.3790419) | Hardware, literacy and recognition errors may dominate protocol differences | Moderate for evaluation need; speculative for AMP benefit | Study repair effort and comprehension under controlled failures |
| Resource cost of this composed contract on the actual platform | Edge and distributed middleware are established | [P16](https://doi.org/10.3390/s24165199), [P17](https://doi.org/10.1016/j.future.2024.05.026), [P25](https://doi.org/10.1109/JIOT.2016.2615180) | A capable hub may make overhead negligible, leaving no distinctive result | Low | Memory/storage/control budget with disclosed tradeoffs |

No gap rests solely on one paper's future-work paragraph. Human/HCI evidence supports the need to investigate access and agency; it does not prove a specific networking algorithm is missing.

## 10. Novelty verdict

**Current concept: B — Mostly integration.** Most proposed components are independently established, several combinations have close architectural precedents, and AMP has not yet supplied a measured advantage or a formal new contract in the available research artifacts.

**Narrowed candidate: C — Incremental research contribution, conditional.** The most defensible direction is a joint presentation-progress/recovery/pacing mechanism for heterogeneous assistive outputs. Even C is a research hypothesis, not a demonstrated result. A D rating would require stronger prior-art exclusion and evidence that conventional composition fails on an important class of cases. There is no basis here for E.

## 11. Deliberate novelty kill test

| Question | Answer | Skeptical assessment |
|---|---|---|
| 1. Is AMP just MQTT plus metadata? | Partially | That remains a plausible account of the current proposal. A schema and adapters do not establish a scientific advance [S06](https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html), [P02](https://doi.org/10.1016/j.ifacol.2016.11.145). |
| 2. Is capability negotiation standardized? | Yes | Substantial description/discovery and language-preference functions are standardized, though not one complete AMP profile [S04](https://www.w3.org/TR/wot-thing-description11/), [S05](https://www.w3.org/TR/wot-discovery/), [S14](https://www.rfc-editor.org/rfc/rfc8373.html). |
| 3. Is modality switching solved? | Partially | Planning/adaptation are established; every assistive interruption/consent case is not established by this review [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf), [P15](https://doi.org/10.3233/AIS-160370). |
| 4. Is semantic normalization standard architecture? | Yes | Intermediate representations and semantic mediation are longstanding [S03](https://www.w3.org/TR/emma/), [P13](https://doi.org/10.3233/AIS-200578). |
| 5. Is receiver pacing ordinary flow control? | Yes, at the broad level | Credit, demand and receiver text-rate limits directly overlap [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S08](https://github.com/reactive-streams/reactive-streams-jvm/blob/v1.0.4/README.md), [S13](https://www.rfc-editor.org/rfc/rfc8865.html). |
| 6. Is presentation tracking simply an application ACK? | Partially | Completion/display acknowledgment already exists; a richer recoverable state machine needs a distinct justification [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf), [S09](https://xmpp.org/extensions/attic/xep-0333-1.0.0.html). |
| 7. Is interruption recovery covered by durable messaging? | Partially | Delivery recovery is; human-facing physical effects are a separate application boundary [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S11](https://www.rfc-editor.org/rfc/rfc9171.html). |
| 8. Does AAC already solve interoperability? | Partially | Distributed and adaptable systems exist; universal interoperability is not established here [P04](https://doi.org/10.1145/2998181.2998215), [P06](https://doi.org/10.1016/j.intcom.2012.06.003). |
| 9. Does W3C MMI cover the architecture? | Yes, substantially | Its architectural decomposition and lifecycle strongly overlap; it does not specify every AMP policy [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/), [S02](https://www.w3.org/TR/mmi-framework/). |
| 10. Does middleware combine most functions? | Partially | A composition covers much of AMP. No single-row checklist should hide this [P13](https://doi.org/10.3233/AIS-200578), [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/). |
| 11. Is cross-disability communication already studied? | Yes | Prior remote systems and ability-diverse HCI invalidate the blanket application novelty claim [P01](https://doi.org/10.5772/60416), [P09](https://doi.org/10.1145/3613904.3641930), [P12](https://doi.org/10.1145/3800645.3813075). |
| 12. Could standards reproduce the exact contribution without research innovation? | Unclear; credible threat | Build that composition first. If it provides the same guarantees and outcomes, position AMP as integration. |

## 12. Smallest defensible contribution

> Existing systems already provide remote assistive communication, multimodal component orchestration, capability/preferences infrastructure, receiver flow control and delivery recovery [P01](https://doi.org/10.5772/60416), [P04](https://doi.org/10.1145/2998181.2998215), [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/), [S04](https://www.w3.org/TR/wot-thing-description11/), [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S13](https://www.rfc-editor.org/rfc/rfc8865.html). Existing research has also investigated output-completion feedback and the practical demands of accessible collaboration [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf), [P05](https://doi.org/10.1109/HAPTICS52432.2022.9765570), [P12](https://doi.org/10.1145/3800645.3813075). However, the reviewed literature provides limited evidence for an evaluated contract that jointly preserves semantic presentation progress, respects fallback consent and regulates remaining output work when heterogeneous assistive renderers are interrupted or restarted. AMP therefore investigates whether such a contract and policy reduce duplicate or lost presentation and conversational repair compared with a composition of established lifecycle, persistence, checkpoint and receiver-credit mechanisms.

This is a **proposed contribution statement**, not a result. The qualifier “reviewed literature” is essential. The difficult point is the interaction of physical presentation with state recovery; a new `presented=true` field is insufficient.

### A concrete, reviewable mechanism to investigate

Represent each message with a stable content ID/version, sender sequence, route/renderer version, presentation epoch, semantic segment IDs, expiry and approved fallback constraints. Store separate facts for **application accepted**, **render attempted**, **render completion observed**, and **user confirmed**. Define the evidence available for each renderer: actuator command, sensor feedback, audio-buffer drain, or explicit user action. Do not label all of them “read.”

The receiver advertises a budget for remaining presentation work. An illustrative workload estimate is `W = sum(estimated remaining segment service times)`. This sum is a baseline modeling choice, **not a claimed invention**. Investigate whether using durable progress and uncertainty intervals in admission decisions adds value beyond adaptive credit with the same service-time observations.

A route change requires compatible segment alignment and a currently approved fallback. If a conversion cannot preserve segment identity—such as an unconstrained translation with reordered content—resume must return to a declared safe boundary or request user choice. Do not silently reuse character offsets across incompatible representations.

After a crash between physical rendering and logging, the device may not know whether a segment was perceived. Mark it **uncertain** and offer the defined replay/skip/confirm interaction. Generic persistence cannot guarantee exactly-once human perception: physical output and a durable log are not automatically one atomic transaction. This is a design limitation AMP should expose, not conceal.

**Proposed invariants:** no completion without the defined evidence; monotone confirmed progress within an epoch; old ACKs cannot complete a new route/version; no unauthorized fallback; expiry records any already presented prefix; bounded admission and explicit rejection; uncertainty is never silently converted to success. Model liveness only under stated assumptions about connectivity, renderer availability and eventual user response.

## 13. Research questions

- **RQ1 — Reliability:** Under interruption, restart and approved route changes, can AMP preserve and reconcile presentation progress more accurately than a composed lifecycle-plus-durable-messaging baseline?
- **RQ2 — Control:** Does joint progress-aware admission reduce tail presentation delay and unwanted replay under unequal or changing output rates, compared with tuned receiver-feedback control using the same observations and resources?
- **RQ3 — Communication:** For intended users and supported modality pairs, do these mechanisms reduce repair effort and improve task success without increasing burden or compromising user control?

RQ1 is not answered by broker delivery rate. RQ2 is not answered by a comparison against unlimited queues. RQ3 is not answered by hardware demonstrations with the researchers.

## 14. Hypotheses and falsification

| Hypothesis | Comparison / primary outcome | What would falsify the useful claim |
|---|---|---|
| H1: progress-aware recovery reduces unwanted repeated or omitted semantic units | Full AMP vs composed checkpoint/lifecycle baseline under identical fault traces | No meaningful reduction, or reduction achieved only by more frequent user interruption |
| H2: joint admission reduces p95 completion delay under changing service rates while preserving completion success | Full AMP vs tuned service-aware receiver credit with equal queues/storage and estimation inputs | Benefit disappears after fair tuning, or shifts delay into hidden sender queues/rejections |
| H3: users need fewer repair actions without worse comprehension | Counterbalanced comparison with the strongest usable baseline | More burden, poorer comprehension, or no practically meaningful benefit |

These are proposed directional hypotheses. Preregister primary endpoints and a meaningful effect/margin after formative work; do not invent a noninferiority margin or participant count now. Exploratory qualitative questions are appropriate for fallback comfort, autonomy and uncertainty presentation. Report null and adverse results.

## 15. Strong baselines and ablations

| Baseline | Required implementation | Role |
|---|---|---|
| B1: conventional MQTT | Supported QoS/session settings, sensible bounded queues and standard reconnect behavior | Engineering reference; insufficient as sole scientific comparator |
| B2: durable application messaging | B1 + persistence, stable IDs, application ACK, deduplication, sequence handling and expiry | Removes benefits explainable by basic reliable messaging |
| B3: receiver feedback | B2 + tuned adaptive credit/window, measured renderer service rate and ACK tied to completion; durable progress checkpoints | Main control/recovery competitor; must be competently implemented |
| B4: multimodal framework composition | Capability/preferences profiles + MMI-style lifecycle + B3; AMQP link credit/settlement is an alternative binding | Main novelty kill test: tests whether existing mechanisms already suffice |
| B5: prior assistive architecture | Reproduce an available design or use released code with licensing and hardware constraints disclosed | Domain comparison only where faithful reproduction is feasible; do not claim a reimplementation is original authors' result |
| B6: real-time-text rate negotiation | RFC 8865-compatible text branch where modality and hardware support it | Direct counter to generic accessible receiver-rate novelty |
| AMP | Proposed joint progress, consent-bound route transitions, recovery and admission policy | Must improve a declared guarantee/outcome over B3/B4 |

For all variants use the same converters, languages, message corpus, device profiles, physical renderers, compute placement and network traces. Match RAM/storage budgets, tune each baseline on separate development traces, and disclose parameters. Give B3/B4 access to the same rate observations as AMP; otherwise the comparison only tests information availability.

**Ablations:** remove capability negotiation using a correct preconfigured route; disable receiver pacing while retaining safe bounded queues; remove detailed presentation tracking while keeping application receipt; replace recovery with a documented whole-message replay policy; replace adaptive routing with a valid fixed route. Also compare independent pacing plus recovery against the joint mechanism. These isolations test causality; the intentionally reduced variants are not the main competitors.

An implementation based on the repository's PubSubClient cannot simply be described as publishing MQTT QoS 2. Verify the client library/version and actual wire behavior. If the research comparison requires MQTT 5, use a supporting implementation and document the migration.

## 16. Experimental requirements and measurement definitions

### 16.1 Three evaluation layers

1. **Protocol model and fault harness:** enumerate state transitions, delayed/stale ACKs, duplicate events, expiry, consent changes and failures at every persistence/render boundary. Use executable state-machine exploration or model checking for the claimed invariants; a successful demo is insufficient.
2. **Physical testbed:** instrument the actual hub and at least several materially different supported renderers, including a slow serial presentation path. Replay identical workloads and faults across variants. The number of endpoints is chosen to exercise behavior, not to maximize a feature count.
3. **User evaluation:** test communication tasks with intended users after the protocol and hardware meet basic correctness requirements. Separate converter errors from delivery errors.

### 16.2 Metrics

| Family | Required measures and definitions | Evidence basis / limitation |
|---|---|---|
| Timing | Creation→application receipt, first presentation, final confirmed rendering, and user confirmation; p50/p95/p99 when sample support permits | Receipt/display/lifecycle distinctions [S19](https://xmpp.org/extensions/xep-0184.html), [S09](https://xmpp.org/extensions/attic/xep-0333-1.0.0.html), [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/). Tail percentiles are proposed evaluation choices, not reported AMP measurements. |
| Queue/control | Sender and receiver queue lengths, estimated and observed presentation work, admitted/completed rate, blocking, rejected/expired messages | Credit/demand and text-rate precedents [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S08](https://github.com/reactive-streams/reactive-streams-jvm/blob/v1.0.4/README.md), [S13](https://www.rfc-editor.org/rfc/rfc8865.html). Include sender wait so backpressure cannot hide latency. |
| Reliability | Repeated semantic segments, unintended omissions, sequence violations, partial/uncertain outcomes, false completion, successful recovery and recovery delay | Transfer recovery/checkpoint distinctions [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [P23](https://arxiv.org/abs/1506.08603). Segment identity must be meaningful across the tested converters. |
| Resource cost | Control bytes/messages, payload bandwidth, CPU, peak RAM, persistent writes/storage and energy where directly instrumented | Middleware/edge relevance [P16](https://doi.org/10.3390/s24165199), [P17](https://doi.org/10.1016/j.future.2024.05.026), [P25](https://doi.org/10.1109/JIOT.2016.2615180). Do not infer energy from CPU time. |
| Communication | Comprehension, task completion, corrections, repair actions, failed attempts, turns and waiting | Assistive/HCI evaluation precedents [P04](https://doi.org/10.1145/2998181.2998215), [P05](https://doi.org/10.1109/HAPTICS52432.2022.9765570), [P07](https://doi.org/10.3233/TAD-190254). A reported “presented” event is not a comprehension score. |

Report generated, admitted, delivered, rendered, expired, rejected and uncertain populations with denominators. An algorithm that expires most difficult messages can appear fast while failing communication. Include success and delay together. For cross-device timestamps, record clock synchronization error; prefer same-clock intervals where possible. Record a renderer event trace alongside the protocol trace.

### 16.3 Robustness workload

Vary latency/jitter, packet loss, outage length, duplicate/reordered application events, hub/broker/device restart, receiver speed changes, sender bursts, simultaneous conversations, long messages, output failure and memory/storage exhaustion. Choose parameter ranges from measured deployment conditions and stress limits; no unmeasured “realistic” numbers are asserted here.

Inject faults before output, during a semantic segment, after physical output but before durable logging, after logging but before acknowledgment, and during an approved route transition. Test a rejected fallback as well as an accepted one. Include user pause or fatigue-driven slowing and an output that cannot report perception. Exercise fairness so a fast conversation cannot starve a slow one.

Repeat independently seeded traces, separate warm-up, retain raw logs and summarize uncertainty at the independent run level. Treat multiple messages from one run as correlated. For user studies, participant/dyad is a clustering unit; thousands of packets do not create thousands of independent users. Report effect sizes and uncertainty, not only significance tests.

## 17. User study requirements

Claims of usable or effective cross-disability communication require evidence from intended users for the stated interfaces and tasks. Protocol correctness can be studied without such claims; it must then be presented as system correctness only.

**Recruit by functional access needs and skill:** Braille readers for Braille output; Deaf or hard-of-hearing sign-language users for a supported sign-language route; people who use gaze/AAC for relevant input; Deafblind participants where tactile communication is actually claimed. Verify language, literacy and device familiarity individually. Not every blind person reads Braille, and a gesture alphabet is not full sign-language competence.

Use formative co-design to define readable chunks, meaningful completion, fatigue controls and acceptable recovery prompts. Then use a counterbalanced within-dyad design where feasible, with matched tasks and training, comparing AMP against the strongest usable baseline. If crossover is unsuitable, justify another design. Assess ordinary exchanges and ethically acceptable interrupted exchanges; avoid consequential real emergency messages.

Choose primary outcomes such as task success/comprehension and repair actions. Add workload, autonomy, preference compliance and qualitative accounts. Determine sample size from the chosen estimand, expected variability, desired precision or power, and feasible recruitment. There is no universal minimum. A small formative study supports design insight; it does not establish population-wide effectiveness.

Obtain appropriate institutional ethics review before recruitment, accessible consent, compensation and communication support. Let participants pause, withdraw and control fallback/output disclosure. Protect message content and capability profiles; receipt/display telemetry can disclose presence and activity [S09](https://xmpp.org/extensions/attic/xep-0333-1.0.0.html). Support persons can help access but should not replace a participant's own account without explicit justification. Convenience samples and impairment simulations must be labeled and limit generalization [P03](https://doi.org/10.7160/eriesj.2024.170206), [P07](https://doi.org/10.3233/TAD-190254).

## 18. Publication positioning

Scope facts below come from official sources; contribution expectations and mismatch judgments are this review's assessment, not editorial promises. No quartile, impact factor, fee or acceptance probability is asserted. “Q1” must be checked using the institution's accepted database, metric year and category before submission.

| Venue / official scope source | Scope fit and expected contribution | Human evaluation | Mismatch risk |
|---|---|---|---|
| [IEEE Internet of Things Journal](https://ieee-iotj.org/) | Strong conditional fit: IoT architecture/protocols, constrained systems, middleware and testbeds. Need a generalizable mechanism, guarantees and fair system evaluation. | Needed for effectiveness/accessibility claims; system paper can scope claims more narrowly | High if paper is mainly an assembly of assistive modules or MQTT schema |
| [Elsevier Internet of Things](https://shop.elsevier.com/journals/internet-of-things/2542-6605) | Strong alternative for reliable edge/middleware and cyber-physical-human integration with a substantive evaluated contribution | Valuable; necessary for human outcome claims | Still requires research beyond a working integration |
| [Journal of Network and Computer Applications](https://shop.elsevier.com/journals/journal-of-network-and-computer-applications/1084-8045) | Strong if recovery/control generalizes to networked applications; scope includes IoT, delay-tolerant networks and remote collaboration | Not inherently necessary for network-only claims | Weak if network mechanism is routine and results concern only hardware usability |
| [IEEE Access](https://ieeeaccess.ieee.org/about/) | Broad applied-engineering fit for a rigorous integrated platform contribution | According to claims | Breadth does not excuse absent baselines or evidence; no claim that acceptance is easy |
| [IEEE TNSRE](https://www.embs.org/tnsre/for-reviewers/editorial-policy/) | Better if contribution concerns assistive/neural/rehabilitation engineering and function for intended users | Strongly relevant to that positioning | Pure middleware protocol without rehabilitation evidence is a weak fit |
| [ACM TACCESS — ACM-hosted journal description](https://hcibib.perlman.hosting.acm.org/hci-sites/journals) | Plausible if accessibility interaction, autonomy and evaluation are central | Usually central for this proposed positioning | Current charter could not be retrieved; fit is provisional from an official hosted description, not fully current-scope verified |
| [MDPI IoT](https://www.mdpi.com/journal/iot/about) | Applied IoT infrastructure/platform/application fit | According to claims | Do not infer Q1 eligibility from title or indexing |
| [IEEE Sensors Journal](https://ieee-sensors.org/ieee-sensors-journal/) | Appropriate if sensor/actuator design or sensing performance becomes the central advance | Relevant for assistive sensing validation | AMP's present protocol focus is a weaker match |

**Target order for the current protocol direction:** retain IEEE IoT Journal as the intended target; use an evidence gate before drafting around it. Elsevier Internet of Things and JNCA are the closest technical alternatives. Consider TACCESS if the strongest result is human interaction/accessibility rather than networking; consider IEEE Access for a well-evaluated applied platform. This is a scope ranking, not a quartile ranking.

The initial project audit's sensor-oriented recommendation addressed the broader hardware project. The user's later protocol focus changes the preferred target and the research work required.

## 19. Manuscript-ready literature review

See [MANUSCRIPT_RELATED_WORK.md](MANUSCRIPT_RELATED_WORK.md) for a synthesized draft with traceable references. It progressively narrows from accessible communication to the proposed recovery boundary. It contains no invented AMP results. It should be revised after expanded database searching and the final experiments, rather than copied as a claim that novelty has already been established.

## 20. Complete bibliography

Use [REFERENCES.md](REFERENCES.md) for authors, verified title, venue/year, available volume/issue/pages, DOI/official URL and access caveats. [references.bib](references.bib) is an editable bibliography export. Institutional/corporate authors and Unicode names are preserved; match the final publisher bibliography style before submission.

Bibliographic issues are visible: P20 online year versus final issue year; P25 conflicting Crossref author/early-access fields; P22 deposited author spellings checked against the official proceedings listing; P05 author version versus published proceedings; P12 verified DIS publication; S17 withdrawn historical edition; S10 verified version 1.6.3 (2025-07-28). Missing pagination is left missing rather than guessed.

## 21. Citation and anti-hallucination audit

The [citation audit](CITATION_AUDIT.csv) gives identity checks, DOI resolver status, access level, supported claim, evidence locator and uncertainties. A 403/202 after DOI redirection is not marked successful full-text retrieval. Source existence/title/DOI mapping can still be verified through Crossref and an authoritative author/publisher record.

| Audit question | Result / qualification |
|---|---|
| 1. Every cited item real? | Authoritative records verified; search phrases and unrelated candidates excluded. |
| 2. Titles verified? | Yes; Crossref, publisher, official standard or author manuscript. |
| 3. Authors verified? | Yes against available records, with P22 deposited spellings corroborated by the official proceedings listing and P25 discrepancy explicitly corrected using CSIRO. |
| 4. Venues verified? | Yes for retained references; workshop review procedure and current TACCESS scope remain qualified. |
| 5. Years verified? | Publication years checked; online/final differences recorded. S10 version/date verified from the official header. |
| 6. DOIs verified before display? | Crossref identity and resolver destination checked for retained DOI items; blocked landing-page access is explicitly recorded. |
| 7. Functionality inferred from titles? | No metadata-only lead is used for technical conclusions; abstract-only functionality is conservatively limited. |
| 8. Unsupported numbers? | No numerical performance, improvement, participant count or cost is asserted. Corpus counts are generated from retained records. |
| 9. “Not reported” confused with absence? | No; matrix legend and gap confidence expressly limit the inference. |
| 10. Preprint confused with reviewed work? | P23 explicitly preprint; P05/P12 published versions verified. P26 workshop review procedure unverified. |
| 11. Inaccessible databases claimed searched? | No; direct-access limits and incomplete result logging are stated in section 1. |
| 12. Novelty exaggerated? | Current B, conditional C, no first-ever claim. |
| 13. Individual versus combination novelty separated? | Sections 7 and 8 separately assess them. |
| 14. Complexity mistaken for novelty? | Larger integration alone does not earn a higher verdict. |
| 15. Older prior art considered? | Yes; output ACK, MMI, AAC, ISO and messaging foundations included. |
| 16. AAC/HCI ignored? | No; remote AAC, mixed-ability collaboration and user agency affect the claim. |
| 17. Only weak baselines? | No; B3/B4 combine strong control, lifecycle, persistence and progress. |
| 18. Delivery versus presentation distinguished? | Yes; ACK evidence is layer-specific. |
| 19. Presentation versus comprehension distinguished? | Yes; separate measurements and study requirements. |
| 20. Conclusions traceable? | Citations and source locators supplied; exact-equivalence absence remains uncertain because coverage is nonexhaustive. |

### Project evidence and next research actions

The preceding [full project audit](../PROJECT_ANALYSIS_AND_PUBLICATION_STRATEGY.md) and [56-file inventory](../FILE_INVENTORY.csv) remain the source-code assessment. The latest user report says all modules/hub are built and tested. The available checkout still lacks the complete deployed hub, recognition/gaze implementation and experiment records needed to independently assess AMP behavior. The literature review does not reverse the user's hardware report.

1. Associate the deployed software, configurations and raw measurements with a frozen research revision. Reconcile the static issues identified in the project audit before benchmarking.
2. Build B3/B4 and specify one exact failing condition that motivates AMP. If no meaningful failure remains, use an integration contribution.
3. Formalize the state contract, renderer evidence and ambiguous-crash behavior; implement instrumentation before claiming reliability.
4. Run equal-condition control/recovery experiments and then appropriately reviewed user studies.
5. Choose title, claims and venue from the strongest demonstrated result. Do not write performance values or an acceptance-oriented novelty claim in advance.

# Required final conclusion

## 1. What is already established

Remote assistive communication, multimodal architectures, capability/preferences descriptions, presentation feedback, receiver flow control and recovery all have substantial prior art [P01](https://doi.org/10.5772/60416), [P04](https://doi.org/10.1145/2998181.2998215), [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf), [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/), [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf), [S13](https://www.rfc-editor.org/rfc/rfc8865.html).

## 2. What AMP should NOT claim as novel

MQTT plus metadata, a text interchange format, connecting glove/Braille/gaze/voice endpoints, generic presentation ACK, receiver pacing, ordinary session state, edge execution or a “first cross-disability system.”

## 3. Closest prior work

The most consequential threats are MMI, EMBASSI, AMQP and RFC 8865, alongside PARLOMA, AACrobat, MIO, SymbolChat and the other systems in section 4. Their combined capabilities matter more than one feature matrix row.

## 4. Remaining research gap

The reviewed set provides limited evidence for an evaluated joint contract covering partial physical presentation, uncertain completion, approved cross-modal fallback and remaining-work admission after interruption. This is a candidate gap, not proof of absence.

## 5. Most defensible AMP contribution

Investigate whether coordinating presentation progress, recovery and pacing reduces unwanted replay, omission and conversational repair beyond a competent composition of established mechanisms.

## 6. Novelty confidence

**Low for a new-protocol novelty claim; Moderate that the narrower problem merits investigation.** Current architecture is B, with a conditional route to C. Abstract-only close sources and nonexhaustive database coverage limit confidence.

## 7. Biggest novelty threats

MMI already supplies lifecycle structure; EMBASSI supplies presentation-completion feedback; RFC 8865 supplies receiver-rate negotiation; AMQP plus application checkpoints may reproduce the proposed benefit [S01](https://www.w3.org/TR/2012/REC-mmi-arch-20121025/), [P26](https://cdn.aaai.org/Workshops/2002/WS-02-08/WS02-08-006.pdf), [S13](https://www.rfc-editor.org/rfc/rfc8865.html), [S07](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-complete-v1.0-os.pdf).

## 8. Experiments required to prove the contribution

State/fault tests, actual renderer instrumentation, equal-information B3/B4 comparisons, unequal-rate and crash-boundary workloads, resource measurements, ablations and intended-user communication studies. No measured AMP advantage is established by this dossier.

## 9. Recommended paper positioning

Target **IEEE Internet of Things Journal conditionally**, centered on presentation-progress-aware recovery/control in heterogeneous assistive IoT. Use the Ability modules as the experimental platform. Consider Elsevier Internet of Things or JNCA according to the final technical result; TACCESS if human-accessibility interaction dominates.

## 10. Go / Modify / Abandon Decision

**MODIFY.** Continue the project, narrow the primary novelty, and test against composed standards before committing to an IoT Journal submission. If the strongest baseline matches AMP, abandon the broad protocol novelty claim and publish the strongest defensible integration or accessibility result instead.
