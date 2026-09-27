# IEEE IoT Journal: AMP protocol research plan

**Updated novelty assessment:** The subsequent [literature dossier](literature_review/REVIEW.md) supersedes this plan's provisional novelty framing: **MODIFY**, current concept mostly integration; investigate a narrower joint presentation-progress/recovery/pacing contribution against strong composed baselines.

**Date:** 15 September 2026  
**User-selected primary venue:** IEEE Internet of Things Journal  
**Selected contribution:** Cross-Disability Remote Communication Protocol  
**Status:** Research design; proposed mechanisms and outcomes below are not implemented or validated by this document.

## 1. Decision

Yes, this is a credible direction for an IEEE IoT Journal submission. The journal explicitly includes IoT architectures, networking protocols, service middleware, applications and testbeds in its [author guidelines](https://ieee-iotj.org/guidelines-for-authors/). Scope fit does not establish novelty or acceptance.

The glove, Braille, gaze and voice devices form the experimental platform. Their recognition/rendering methods can be established techniques; the paper can concentrate on the protocol contribution. Document their errors and timing because those affect end-to-end outcomes.

The owner reports that all module families and the hub are physically built and tested. The previous audit found that the local checkout lacks the complete deployed software and recorded measurements. Retrieve those artifacts before evaluating AMP.

## 2. Research problem

A communication system must account for more than whether bytes arrive. A receiver may support only certain output formats/languages, need several seconds to present a message, disconnect midway, or require user-controlled progression. A fast sender can generate a growing queue for a slow receiver. Replaying a message after reconnection can duplicate content already presented; dropping it can omit content the user still needs.

**Primary question:** Can a protocol that accounts for receiver capabilities, presentation progress and output pace improve successful remote communication under intermittent connectivity, within a bounded device-resource budget?

Define successful communication at two levels:

1. **System outcome:** permitted content is presented with the specified ordering, completeness, recovery and expiry behavior.
2. **Human outcome:** the recipient correctly understands the message and completes a communication task.

A presentation-completion event supports the first outcome only. It does not prove understanding. Without physical sensing, a renderer's completion event describes its software execution, not verified actuator movement.

## 3. Novelty boundary

The following are established building blocks, not sufficient individual novelty claims:

- MQTT, text interchange, hub/cloud deployment, encryption, acknowledgements, queues and device profiles.
- General coordination of input/output modalities. The [W3C Multimodal Architecture](https://www.w3.org/TR/mmi-arch/) already defines an interaction manager, modality components and lifecycle events.
- Adapting to device capabilities and user preferences. The [W3C Multimodal Interaction Framework](https://www.w3.org/TR/mmi-framework/) explicitly discusses these requirements, including distributed sessions.
- MQTT-based remote assistive architecture. [Publish-Subscribe Architecture for Delivering Assistance to Visually Impaired People](https://www.sciencedirect.com/science/article/abs/pii/S2405896316325976) describes remote assistance using MQTT.
- General cloud-to-device delivery feedback and expiration acknowledgements, which are available in systems such as [Azure IoT Hub](https://learn.microsoft.com/en-us/azure/iot-hub/how-to-cloud-to-device-messaging).

**Candidate contribution to investigate:** a precisely specified protocol and scheduling/recovery policy that jointly respects accessible output compatibility, receiver pace, conversational ordering and bounded resources, with a demonstrated benefit over capable conventional implementations.

This candidate remains a hypothesis. A deeper literature comparison must establish which requirement combination is inadequately addressed and why the proposed solution improves on existing methods. Do not claim “the first cross-disability remote communication system.”

## 4. Proposed AMP design

### 4.1 Capability and preference contract

Describe the interface instead of routing only by diagnosis:

- Accepted input/output representations and languages.
- Braille code/grade/cell count or supported sign vocabulary where applicable.
- Permitted fallback formats, chosen by the user.
- Automatic or user-paced presentation; current availability.
- Payload/queue limits and an estimate of presentation duration.

For example, a user who requests tactile output may not accept audio fallback. The protocol must represent that constraint explicitly. Supporting a Braille bit pattern is not equivalent to supporting a complete Braille language/code.

### 4.2 Message and presentation lifecycle

Specify IDs, conversation/sequence numbers, recipient capability version, language, permitted representations and lifecycle events. A possible lifecycle is:

`accepted → queued → presenting → presentation_completed`

with separately defined `rejected`, `paused`, `expired`, `cancelled`, `failed` and `completion_unknown` outcomes.

The final state machine must define permitted transitions, authoritative state owners and recovery after crashes. Do not assume transport QoS produces exactly-once physical presentation. Specify progress boundaries and how a user may request replay; distinguish an intentional replay from a duplicate caused by transport recovery.

### 4.3 Receiver-paced admission, scheduling and recovery

This should be the technical center if the literature comparison supports it:

- Estimate backlog in presentation time, as well as bytes/messages.
- Use receiver feedback to bound outstanding work.
- Preserve message/conversation order where required.
- Handle partially presented content after interruption.
- Make expiry/cancellation/failure visible to the sender; never silently summarize or discard user messages.
- Respect user-approved output alternatives.
- Avoid starving one conversation or slow modality when capacity is shared.

Begin with the simplest defensible policy and quantify its trade-offs. A weighted objective could balance completion delay, failure/expiry rate and resource use, subject to accessibility, ordering and fairness constraints. Define workload assumptions, metrics and parameter selection before claiming optimality. Reinforcement learning is not required.

### 4.4 Security and endpoints

Use established authenticated pairing and encryption. Specify nonce/key handling, replay protection, retention and metadata exposure. In the existing architecture, hubs process plaintext; the intended protection is hub-to-hub against the relay. Security is a system requirement unless the paper introduces and evaluates a separate security contribution.

## 5. Three connected contributions for the manuscript

These are proposed contribution categories, not claims of completed work:

1. **Protocol model:** explicit capability, pacing and presentation/recovery semantics for heterogeneous assistive endpoints.
2. **Technical method:** a defined scheduling/recovery mechanism that improves a stated objective under realistic constraints, compared with strong baselines.
3. **Reproducible evaluation:** real multi-hub implementation, controlled network/endpoint experiments and a bounded study of communication effectiveness.

The measured advance in contribution 2 is particularly important. A schema plus feature list may remain ordinary integration work even if implemented well.

## 6. Experimental design

### Fair baselines

1. Conventional MQTT routing with fixed endpoint mapping, queueing and transport reliability configured appropriately.
2. The same system with application acknowledgements, persistence and deduplication.
3. A credible fixed-window or receiver-feedback flow-control method, using identical encoders, renderers and hardware.
4. Full AMP plus ablations removing each proposed adaptation independently.

Include relevant lifecycle/middleware functionality from the literature in the comparison. Do not compare AMP only against a deliberately naive or broken MQTT implementation.

### Testbed

Use two physically distinct hubs over a real remote connection. Include a slow tactile receiver, speech/text output, and sign/gaze input as available. Start with a small set of different paths and evaluate both directions. Expand protocol conformance coverage across the full intended matrix, distinguishing synthetic endpoint tests from real devices and participant evidence.

Use two complementary experiments:

- **Protocol isolation:** known text/events and recorded endpoint service times remove recognition quality as a confounder.
- **Full pipeline:** real inputs and outputs reveal how sensing, translation and human pacing affect practical communication.

Vary network delay/loss/disconnections, message length, arrival rate, simultaneous conversations, output rate, queue capacity, receiver failure and restart position. Label emulated network conditions and simulated devices clearly.

### Measurements

| Question | Evidence |
|---|---|
| Is presentation correct? | Complete/partial/duplicated/out-of-order presentation events, including unknown completion states |
| Is communication usable? | Message comprehension, task completion, correction requests, user wait and frustration |
| Does pacing help? | Presentation backlog, queue overflow/admission behavior, completion latency and fairness |
| Does recovery help? | Missing/repeated content and recovery delay after interruption/restart |
| What does it cost? | Control-message bytes, CPU/RAM, storage writes, energy and added normal-condition latency |

Report trial counts, distributions and uncertainty. Define clocks and synchronization error. Keep input acquisition, network receipt, presentation start and presentation completion as separate timing points. Slower Braille reading is not evidence that the network is slow.

Choose participant count using the research question, variability and feasible recruitment; no universal five-person threshold establishes this paper's conclusions. Follow the institutional ethics process and evaluate people with the access needs actually claimed. All six design profiles need not become six broad clinical claims.

## 7. Working title and paper structure

**Working title:** *AMP: A Receiver-Paced Protocol for Cross-Disability Remote Communication in Heterogeneous IoT Systems*

The title is provisional: retain “receiver-paced” only if that mechanism is implemented and the central evidence supports it.

Suggested structure:

1. Concrete communication problem and requirements.
2. Closest protocols, multimodal middleware and assistive systems.
3. System model, assumptions and performance objectives.
4. AMP semantics and proposed scheduling/recovery method.
5. Implementation and experimental methods.
6. Protocol results, ablations and communication-task results.
7. Trade-offs, limits, deployment implications and conclusion.

## 8. Submission decision gates

- **Novelty:** a documented difference from the closest capable systems, beyond application labels.
- **Correctness:** specified failure behavior and verified conformance; repaired or replaced draft source issues from the project audit.
- **Benefit:** meaningful improvement on the primary outcome without unacceptable regression elsewhere; report negative cases.
- **Evidence:** archived source/configuration/data, actual hardware measurements and appropriately scoped human outcomes.

If the benefit disappears when compared with ordinary application acknowledgements and flow control, revise the method or claims. A working demonstration remains valuable, but does not by itself establish a sufficiently substantial protocol contribution for IoT Journal.

**Immediate next step:** freeze the deployed system, write the protocol's requirements/state machine, and construct the baseline testbed before choosing a more elaborate algorithm or drafting result claims.
