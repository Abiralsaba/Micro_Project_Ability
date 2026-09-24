# Literature Review

## Reliable Cross-Modal Assistive Communication: Presentation Progress, Receiver Pacing and Interruption Recovery

**Proposed project:** AMP — A Cross-Disability Remote Communication Protocol for Heterogeneous Assistive IoT Systems

**Student name:** ____________________  

**Student ID:** ____________________  

**Course / department:** ____________________  

**Supervisor / teacher:** ____________________

**Prepared:** 16 September 2026  

**Literature cutoff:** 15 September 2026

## Abstract

Assistive communication systems support interaction through interfaces such as speech, text, gaze, Braille and tactile output. Connecting these interfaces introduces a distinction between successful message delivery and successful presentation to a recipient. This review examines remote assistive communication, multimodal architectures, capability and preference representation, messaging acknowledgments, receiver pacing and interruption recovery. The reviewed literature establishes substantial prior functionality in each area. Consequently, the use of heterogeneous devices, a shared message format or application acknowledgments cannot independently establish the novelty of a new protocol. A narrower candidate research problem concerns coordinating confirmed and uncertain presentation progress with receiver admission and permitted output changes after interruption. This review identifies relevant prior work, explains the limitations of available evidence, and proposes research questions and comparisons for investigating that problem. The proposed AMP mechanism remains to be specified and evaluated; no experimental advantage or first-of-its-kind claim is asserted.

**Keywords:** assistive communication; multimodal interaction; Internet of Things; presentation progress; receiver pacing; interruption recovery.

## 1. Introduction and review objective

Remote assistive communication has been explored through tactile robotic interfaces, publish-subscribe assistance and accessible picture messaging. These systems establish that communicating through specialized interfaces and remote connections predates AMP. Their differences in representation, user population and interaction design nevertheless leave important questions about interoperability and reliable presentation {{P01}} {{P02}} {{P06}}.

The proposed Ability platform combines assistive endpoints with embedded devices and edge hubs. The intended paper focuses on the communication protocol connecting these components. Its central concern is what happens when a message has reached the receiving system but only part of its content has been presented. An interruption can leave a completed prefix, unfinished content and a segment whose completion is uncertain. A subsequent output change may also invalidate the original renderer's progress representation.

The objective of this review is to determine which parts of this problem are already addressed and what would require further investigation. It distinguishes device capability from user preference, transport delivery from presentation, and presentation from human comprehension. It also evaluates whether AMP would provide a scientific contribution beyond combining established mechanisms.

## 2. Search approach and limitations

The underlying review used sixteen topic clusters covering accessible remote communication, Deafblind interfaces, multimodal architectures, capabilities and preferences, intermediate representations, modality selection, IoT middleware, acknowledgment semantics, receiver control, queueing, recovery, conversation state, accessibility standards, AAC, mixed-ability interaction and edge computing. The primary publication window was 2020–15 September 2026. Earlier work was retained when it directly affected the novelty assessment.

Discovery used general web search, followed by checks against publisher, proceedings, institutional and official specification records. Crossref was queried directly for bibliographic verification. Scopus and Web of Science were not directly searched. Research publications and standards were treated as different evidence types. Relevant sources addressed at least one of the communication, interoperability, presentation or recovery questions. Diagnosis-only applications, recognition-only studies without communication relevance, unverified leads and duplicate versions were excluded from the analytical core.

This document synthesizes a selected subset of the broader 46-source dossier. It is a structured critical review, not an exhaustive systematic review. Search-result counts were not consistently recorded, so no PRISMA statistics are reported. Some studies were assessed from abstracts; detailed unreported functions remain unknown. Evidence access is identified in Appendix A. A feature described as unreported must not be interpreted as technically impossible in the earlier system.

## 3. Thematic literature review

### 3.1 Remote assistive and cross-modal communication

PARLOMA connects hand-shape acquisition to remote robotic tactile reproduction. Its significance for AMP is the demonstrated architecture for networked assistive communication; its restricted hand-shape setting should not be equated with unrestricted sign-language translation {{P01}}. Rodrigues and colleagues describe a publish-subscribe architecture for assistance to visually impaired users, providing direct precedent for MQTT-based assistive communication {{P02}}. SymbolChat addresses remote picture-based messaging with configurable interfaces, showing that adaptation to individual communication needs is also established {{P06}}.

Cross-modal prototypes provide a related but distinct line of evidence. The bidirectional Braille–speech system of Pérez-Aguirre and colleagues demonstrates a conversion architecture. However, examination of its methods and discussion shows that the trials used sighted/hearing participants and simulated impairment rather than Deafblind participants. Its findings therefore cannot establish communication effectiveness for the intended population {{P03}}. MIO investigates local vibration-based communication with Deafblind users and identifies practical needs around customization and playback; its local evaluation does not establish a remote recovery protocol {{P05}}.

AACrobat connects gaze-based AAC with a mobile companion to support partner collaboration and user autonomy. Although its implementation is distributed, its principal study setting was co-located interaction {{P04}}. These differences matter: conversion, network connectivity and user evaluation are separate forms of evidence. A new platform should specify exactly which modality pairs, languages, tasks and operating conditions it supports.

### 3.2 Mixed-ability interaction and user control

The systematic review by Xiao and colleagues establishes ability-diverse collaboration as an existing HCI research area {{P09}}. Tang and Piper's study of collective access examines how differing and sometimes conflicting access needs interact with communication practices in a mixed-ability organization {{P12}}. Such findings motivate attention to user preferences and the social consequences of adaptation. They do not, by themselves, establish a missing networking algorithm.

Physical output quality also affects communication. A comparison of device-mediated and human-articulated tactile fingerspelling reports differences in intelligibility {{P27}}. Therefore, successful actuation or playback is an inadequate proxy for successful understanding. For AMP, output selection and recovery should preserve user choice, while claims about communication effectiveness require evaluation with the intended users.

### 3.3 Multimodal architecture, capabilities and representation

The W3C Multimodal Architecture and Interfaces specification defines an interaction manager, distributed modality components and lifecycle events, including completion, pause and resume {{S01}}. EMMA provides a representation for multimodal interpretation annotations {{S03}}. These foundations substantially overlap with a hub that coordinates input/output adapters and exchanges intermediate content. A common text or JSON representation is consequently an implementation choice whose scientific value must come from additional semantics or demonstrated behavior.

Device descriptions and user preferences address different requirements. Web of Things Thing Description provides machine-readable service affordances and associated schemas {{S04}}. ISO/IEC 24751-2 describes personal needs and preferences for accessible digital delivery {{S18}}. RFC 8373 specifies human-language negotiation by medium and direction {{S14}}. Together, these sources show that a device's ability to produce an output cannot be treated as evidence that a user prefers or authorizes that output. AMP should express permitted alternatives explicitly rather than assign them solely from disability labels.

### 3.4 IoT middleware and edge execution

INTER-IoT's CasAware case study integrates IoT platforms using syntactic and semantic mediation {{P13}}. A multilayer multimodal architecture also addresses application deployment at the edge {{P16}}. These works supply relevant infrastructure comparisons. Their application contexts differ from person-to-person assistive conversation, but they establish that platform heterogeneity, semantic mediation and edge placement are not new concepts.

A protocol-focused contribution must therefore explain the communication behavior enabled by its design. The use of ESP32 devices, a Raspberry Pi or a message broker does not independently answer that question. Reusing such components can support a sound experimental platform while the research contribution remains concentrated on one clearly defined mechanism.

### 3.5 Acknowledgment and presentation state

Different acknowledgments certify different events. MQTT specifies message-transfer and session behavior {{S06}}. XMPP delivery receipts notify the sender that a message reached a recipient client, whereas displayed markers indicate a client's displayed position {{S19}} {{S09}}. Earlier multimodal output research in EMBASSI also describes feedback following completed presentation {{P26}}. Presentation-aware acknowledgment is therefore not a new concept in itself.

For assistive output, the event associated with completion must be defined separately for each renderer. A command accepted by a motor controller, a completed actuator movement, an audio playback event and a user's explicit confirmation are not equivalent observations. The proposed research should retain those distinctions. It should not interpret a device event as proof that a person perceived or understood the message.

### 3.6 Receiver pacing and interruption recovery

Receiver influence over delivery is also established. AMQP provides link credit and delivery-state mechanisms {{S07}}. RFC 8865 is a particularly relevant precedent because conversational text receivers can negotiate a maximum character rate {{S13}}. A claim that AMP introduces communication adapted to a slow recipient would therefore be too broad.

Recovery mechanisms address additional parts of the problem. AMQP describes received message-section positions for transfer resumption, while the Bundle Protocol addresses communication under disruption {{S07}} {{S11}}. These mechanisms concern data transfer and forwarding. A transfer offset does not automatically represent the portion of tactile or spoken content that a user has perceived.

This distinction motivates the proposed research boundary. If a device stops after physical output but before its completion record is durably stored, the system may not know whether that segment was presented. Replaying it can repeat content; skipping it can omit content. A defensible protocol should expose and reconcile that uncertainty. Its benefits must nevertheless be compared against ordinary application checkpoints and receiver feedback, since a composition of existing mechanisms may already provide adequate behavior.

## 4. Comparison of the closest relevant approaches

The table summarizes overlap and limits in the examined evidence. It is not a ranking of overall system quality. “Not established” denotes an evidence limitation rather than proof that the system cannot support a feature.

| Work | Established contribution | Relevance to AMP and remaining question |
|---|---|---|
| PARLOMA {{P01}} | Remote tracked hand-shape to tactile robotic output | Remote assistive communication is prior art; a general recoverable presentation contract is not established in the examined work. |
| Braille–speech system {{P03}} | Bidirectional conversion and prototype trials | Demonstrates conversion; remote recovery and intended-user effectiveness require separate evidence. |
| AACrobat {{P04}} | Distributed gaze/mobile AAC and user-controlled partner collaboration | Autonomy and heterogeneous-device interaction overlap; durable interrupted-presentation progress is not established. |
| MIO {{P05}} | Customizable local vibration communication with Deafblind user evaluation | Informs tactile interaction and replay needs; remote protocol behavior was not the evaluated contribution. |
| INTER-IoT/CasAware {{P13}} | Platform interoperability and semantic mediation | Challenges generic middleware novelty; human presentation is a different application boundary. |
| EMBASSI output modeling {{P26}} | Multimodal output planning and completion feedback | Direct precedent against a generic presentation-ACK novelty claim. |
| W3C MMI {{S01}} | Distributed modality-component lifecycle | Provides a strong architectural foundation and comparator for AMP state transitions. |
| AMQP 1.0 {{S07}} | Receiver credit, delivery states and transfer recovery | Strong reliability/control baseline; transfer progress must be distinguished from physical presentation. |
| RFC 8865 {{S13}} | Conversational text with negotiated receiver rate | Direct precedent against generic receiver-paced accessible communication novelty. |

## 5. Synthesis and candidate research gap

The reviewed literature establishes remote assistive communication, multimodal coordination, capability and preference representation, completion feedback, receiver control and recovery mechanisms. These findings rule out treating AMP's broad component list as sufficient novelty. They also show why comparison against a basic transport-only implementation would be inadequate.

Within the examined sources, evidence is limited for an evaluated mechanism that jointly coordinates: (1) confirmed and uncertain presentation progress; (2) recovery after interruption; (3) admission based on unfinished presentation work; and (4) output transitions constrained by user permission. This is a candidate gap in the reviewed evidence, not proof that no equivalent mechanism exists.

The central issue is whether coordinating these functions changes a useful guarantee or communication outcome compared with their independent composition. For example, progress information may change the estimated work remaining after a restart, while an approved output transition may require mapping progress to a different representation. If that mapping cannot preserve content boundaries, the system should return to a declared safe boundary or request user choice rather than silently reuse an incompatible offset.

AMP should not claim exactly-once human perception or universal cross-disability communication. Its claims should be limited to defined protocol events, supported interfaces, tested recovery conditions and the participant populations actually evaluated.

## 6. Proposed research direction

The proposed study will investigate a mechanism that reconciles confirmed and uncertain content progress and coordinates receiver admission with permitted output transitions. Stable message and segment identities, explicit presentation states and versioned routes are candidate design elements. They remain to be formalized and tested; this review reports no implemented mechanism or measured advantage.

Three research questions follow from the evidence:

1. **RQ1:** Can presentation-progress reconciliation reduce unwanted repetition or omission after interruption compared with durable messaging and application checkpoints?
2. **RQ2:** Does coordinating unfinished presentation work with admission improve waiting, recovery or resource use beyond adaptive receiver feedback?
3. **RQ3:** For supported interfaces and intended users, do any system improvements reduce repair effort without reducing comprehension or user control?

The principal baseline should combine persistence, application acknowledgments, deduplication, progress checkpoints and adaptive receiver control. A second comparison should use a multimodal lifecycle framework with equivalent capabilities and resources. Experiments should hold converters and hardware constant, exercise failure boundaries, and measure both sender and receiver waiting so that moving a queue does not appear to reduce total delay.

System outcomes should include repeated or omitted segments, recovery delay, false completion, expired/rejected messages and resource cost. Human-effectiveness claims additionally require appropriately designed studies with intended users. Automated device traces can establish renderer behavior but cannot establish comprehension. AI/ML may later be considered if real observations show useful, actionable uncertainty in receiver behavior; it is not required to define the initial protocol contribution.

## 7. Conclusion

Existing research already addresses many components of cross-modal assistive communication. The most defensible direction for AMP is therefore a narrow investigation of interrupted presentation and its interaction with receiver control and permitted output transitions. The research opportunity remains conditional: the proposed coordination must provide a meaningful advantage over strong combinations of established methods. The next stage is to specify the mechanism, instrument the working platform, collect structured baseline data and evaluate the identified failure cases. This review supports that investigation while leaving novelty and effectiveness to be established through further evidence.

## References

{{REFERENCES}}

## Appendix A. Evidence access

“Full text” indicates that relevant sections were examined; it does not imply independent replication. “Abstract only” limits conclusions to functionality stated in the abstract. The research publications and specifications below are distinct evidence types. Technical standards define behavior but do not constitute participant studies.

{{ACCESS}}
