# AMP research dossier

**For teacher submission:** Use the standalone [Word literature review](teacher_submission/AMP_Literature_Review.docx) or [PDF](teacher_submission/AMP_Literature_Review.pdf). Fill in the cover details before submitting. It contains the literature synthesis, closest-work comparison, candidate gap, research questions, 23 numbered references and source-access notes.

**Decision: MODIFY.** Keep IEEE Internet of Things Journal as a conditional target. Narrow the research to presentation-progress-aware recovery and pacing, and compare against composed existing mechanisms.

Start with the [complete 21-part review](REVIEW.md). It includes the search/access limitations, novelty decomposition, kill test, research questions, hypotheses, baselines, experimental design, user study requirements and venue comparison.

**AI/ML follow-up:** Read [Can AI/ML strengthen AMP?](AI_ML_NOVELTY_ANALYSIS.md) for ranked research directions, a concrete model/control proposal, data collection and a feasibility gate. It incorporates your confirmation that the devices work but no structured dataset exists yet. [Supplementary evidence](AI_ML_EVIDENCE.md) documents the additional prior-art checks.

| Artifact | Purpose |
|---|---|
| [Search log](SEARCH_LOG.csv) | Recorded A–P discovery queries, supplemental queries and direct metadata checks |
| [Core literature](CORE_LITERATURE.csv) | 46 sources, 21 columns; 26 research items and 20 standards/specifications |
| [Closest-work comparison](REVIEW.md#4-fifteen-closest-systems-and-frameworks) | Fifteen close systems/frameworks and the proposed difference |
| [Feature matrix](FEATURE_MATRIX.csv) | 24 features; fifteen comparisons plus AMP explicitly proposed |
| [Matrix interpretation](FEATURE_MATRIX.md) | Conservative coding and evaluation caveats |
| [Related-work draft](MANUSCRIPT_RELATED_WORK.md) | Synthesized manuscript text without invented results |
| [References and access notes](REFERENCES.md) | Bibliographic identities, evidence locators and uncertainties |
| [BibTeX](references.bib) | Editable bibliography export |
| [Citation audit](CITATION_AUDIT.csv) | Identity, DOI, access and claim checks |
| [Source assessments](SOURCE_ASSESSMENTS.json) | Machine-readable manual evidence annotations |
| [Original project audit](../PROJECT_ANALYSIS_AND_PUBLICATION_STRATEGY.md) | Full project review, distinct from literature evidence |
| [Original file inventory](../FILE_INVENTORY.csv) | All 56 original files inspected |

## Reproducibility

`python3 paper/literature_review/build_dossier.py` regenerates the CSV tables, references, BibTeX and annotations using preserved metadata and manual assessments. It makes no network requests. The analytical Markdown files are manually written.

`verify_sources.py` performs network metadata discovery/verification with `requests`; its results are metadata, not proof of reading papers. Rechecks write to `crossref_recheck.json` so the original evidence records are preserved. Existing `crossref_records.json`, `crossref_additional.json` and `standard_access_records.json` preserve the checks used for this review. Search rankings and live specification editions can change. The review does not claim complete Scopus/WoS coverage or reproducible PRISMA counts.

Source PDFs were examined in temporary working storage where accessible; copyrighted full texts are not redistributed in this dossier. Use the publisher/author links and evidence locators to inspect them. A retrieval block is not an absent publication.
