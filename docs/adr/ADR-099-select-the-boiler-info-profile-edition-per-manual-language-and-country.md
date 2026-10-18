---
id: "ADR-099"
title: "Select the Boiler Info profile edition per manual language and country"
status: "Proposed"
date: "2026-10-18"
binding: false
gate: null
documents_shipped: false
verified_in: []
supersedes: []
superseded_by: null
related:
  - "ADR-008"
  - "ADR-051"
topics:
  - "boiler-info"
  - "model-profile"
  - "localisation"
  - "littlefs"
aliases:
  - "profile edition"
  - "manual language"
  - "boiler info language"
  - "gas type factory values"
components:
  - "Model profile table (kBoilerModelProfiles, BoilerProfiles.h)"
  - "Boiler Info REST response (sendBoilerInfoV2, profile_file)"
  - "Boiler Info page (renderBoilerDetails, index.js)"
  - "Model profile files on LittleFS (data/*.json)"
symbols:
  - "kBoilerModelProfiles"
  - "BoilerModelProfile"
  - "sendBoilerInfoV2"
  - "renderBoilerDetails"
context_scope: "selective"
format: "madr"
---

<!-- markdownlint-disable MD025 -->

# ADR-099 Select the Boiler Info profile edition per manual language and country

## Status

Proposed, 2026-10-18.

**Decision Maker:** pending. Drafted by the agent on request of the user (STemplar, fork
otgw-1.x.x) under TASK-1246; acceptance needs the user's explicit approval.

## Status History

```yaml
status_history:
  - date: 2026-10-18
    status: Proposed
    changed_by: "User: STemplar"
    reason: Initial proposal, drafted by the agent on request (TASK-1246)
    changed_via: manual
```

## Context and Problem Statement

The Boiler Info page shows manufacturer data for a recognised boiler model. The firmware
recognises the model from member ID, ID127 and, where needed, ID131
(`kBoilerModelProfiles[]` in `src/OTGW-firmware/BoilerProfiles.h`) and reports one file name
as `profile_file` in `/api/v2/boiler/info`. The page loads that file from LittleFS.

The file holds data from one printed manual: status, substatus and fault texts, parameter
labels, setting ranges and factory values. For the Tzerra M 28c Plus this is the Dutch
manual 7618573 v.10. That data depends on the edition of the manual:

* **Language.** All texts are Dutch (the page caption says "Manufacturer descriptions:
  Nederlands").
* **Country.** The manual's Tab.2 lists only the Netherlands, with G20, G25, G25.3 and G31.
  Its §7.6.1 states that the factory setting is gas group G25 (L-gas), and Tab.34-41 give the
  factory values of P17-P21 per gas type. A Belgian or German edition of the same boiler can
  have another factory gas and other tables.

The user wants Boiler Info selectable for several languages and countries (2026-10-18).
Two rules from the user frame the problem:

* The fixed page texts ("Device identification", "Fault status", ...) stay English: they are
  OpenTherm protocol terms, kept to avoid confusion.
* Member ID company names stay as in the official OpenTherm Association list.

Constraints: one LittleFS image per build (ADR-008), file names of at most 30 characters
(checked by `test/host/test_boilerInfo.cpp`), about 40 KB usable RAM, settings as JSON with
named sections (ADR-051).

## Decision Drivers

* Showing another country's factory values as if they applied misleads the installer.
* Every value must stay traceable to one manual edition (document number, version,
  language), as the existing profile already is.
* The setting is a property of the installation (where the boiler is, which manual applies),
  not of one browser.
* No firmware RAM cost for texts: the texts stay in LittleFS files read by the browser.
* Keep model recognition in one place (`kBoilerModelProfiles[]`).

## Considered Options

1. **One file per edition, chosen by a firmware setting.** `tzerra-m-28c-plus.nl-NL.json`,
   `tzerra-m-28c-plus.fr-BE.json`, ...; a setting holds the edition code; the firmware reports
   the edition file.
2. **One file per edition, chosen in the browser** (`localStorage`), firmware unchanged.
3. **One base file plus a language overlay** (numbers in the base, texts per language).
4. **Do nothing:** one edition per model, as now.

## Decision Outcome

Chosen option: **1, one self-contained file per manual edition, chosen by a firmware
setting.**

* Name a profile file `<model>.<edition>.json`, where `<edition>` is a language-country code
  such as `nl-NL` or `fr-BE` (BCP 47 form). Each file is complete and follows one manual
  edition; it carries `edition`, `language` and the manual's document number and version in
  `source`.
* Keep `kBoilerModelProfiles[]` as the only place for recognition; its file field becomes the
  model part (`tzerra-m-28c-plus`).
* Add one setting for the edition code, default `nl-NL`, in a Boiler Info section of
  `OTGWSettings` (ADR-051), editable on the settings page.
* `sendBoilerInfoV2()` reports `profile_file` as `<model>.<edition>.json` and `profile_edition`.
* The page loads that file. When it is missing, the page shows the generic view with the note
  that no manufacturer data exists for that edition; it does not load another edition.
* Rename the current file to `tzerra-m-28c-plus.nl-NL.json` without content changes.

Option 2 was not chosen because the edition belongs to the installation, and two browsers
could show different factory values for the same boiler. Option 3 was not chosen because
factory values and gas tables also differ per country, so the "numbers" part is not shared
either, and an overlay splits one manual over two files. Option 4 does not meet the request.

## Decision Contract

### Must

* Take every manufacturer value in a profile file from the one manual edition named in its
  `source`.
* Report the edition the page shows (`profile_edition`) together with `profile_file`.
* Fall back to the generic view when the selected edition has no file.

### Must Not

* Load another edition's file as a fallback.
* Translate the OpenTherm terms of the page or the member ID company names.
* Put manufacturer texts in firmware RAM or flash strings.

### Exceptions

* None.

### Verification

* `test/host/test_boilerInfo.cpp`: every model plus edition file exists, carries the same
  `id`, and its name is at most 30 characters.
* `test/host/test_otBrandTextUi.js`: edition chosen from `profile_file`, generic view for a
  missing edition.

## Consequences

### Positive

* Factory values and gas tables follow the country of the installation.
* A new edition is one data file plus nothing in code.
* All viewers see the same edition.

### Negative

* Data shared by editions (units, signed flags, ranges) is repeated per file.
* A settings schema change (new section and key) and one more REST field.
* LittleFS space grows by one file per edition (the current profile is about 12 KB).

### Risks

* A file name of model plus edition could exceed 30 characters for long model names; the host
  test fails the build then and the model part has to be shortened.

## Pros and Cons of the Options

### Option 1: file per edition, firmware setting

* Good: per installation, traceable, no RAM cost.
* Bad: settings and REST change; repeated data.

### Option 2: file per edition, browser choice

* Good: no firmware change.
* Bad: per browser, so two viewers can disagree about the boiler's factory values.

### Option 3: base plus language overlay

* Good: no repeated numbers within one country.
* Bad: numbers differ per country too; one manual is split over two files.

### Option 4: do nothing

* Good: no work.
* Bad: does not meet the request.

## Open Questions

* [ ] Which editions are wanted first, and which manuals are available for them?
* [ ] Edition code form: `nl-NL` (language-country) or country only (`NL`, `BE`)?
* [ ] Setting on the settings page only, or also selectable on the Boiler Info page?

## Related Decisions

* **ADR-008 (LittleFS for configuration and web UI files)**: profile files stay on LittleFS.
* **ADR-051 (settings and state architecture)**: the edition setting is a new named section
  in `OTGWSettings`.

## References

* `src/OTGW-firmware/BoilerProfiles.h`: `kBoilerModelProfiles[]`.
* `src/OTGW-firmware/OTGW-Core.ino`: `sendBoilerInfoV2()`, `profile_file`.
* `src/OTGW-firmware/data/index.js`: `renderBoilerDetails()`.
* `src/OTGW-firmware/data/tzerra-m-28c-plus.json`: current profile (NL, 7618573 v.10).
* Remeha Tzerra Plus installation and service manual 7618573 v.10 (17-03-2017): Tab.2 p.10,
  §7.6.1 p.50, Tab.34-41 p.58-60 (local copy
  `Documentatie/Vendoronderzoek/Remeha/Handleidingen/Tzerra-Plus-Gebruikers-Installatie-en-Servicehandleiding.pdf`).
* BCP 47 language tags: <https://www.rfc-editor.org/info/bcp47>
* TASK-1242 (Boiler Info page layout and tooltips), TASK-1246 (this decision).

## Enforcement

Manual review only until accepted; the rules become checkable once the file naming exists.
