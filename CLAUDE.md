# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Objective

**The objective of this project is to build a JSON-backed contact list CRUD console application, in ordered steps, building up from a hand-written JSON library.**

This is a from-scratch C++ exercise (no third-party JSON library): first write a JSON parser/serializer, then package it as a static library, then build a console CRUD app on top of it that persists contact records to a JSON file.

Test data throughout: a list of **10 randomly generated contacts**, each with `name`, `phone number`, `company name`, `DOB`. This same contact schema is used for parser testing (Step 1) and as the CRUD app's data model (Step 3); Step 4 refines the schema (name split, optional fields).

## Steps

Follow these steps **in order**. Do not start a step before the previous one has been reviewed and approved by the user. After finishing a step:

1. Ask the user to review the result.
2. Only once the user agrees, `git commit` with message `"step X"` (X = the step number), then `git push` to the remote (`https://github.com/jonghoon-ryu/PoC_json.git`).

- **Step 1 — Hand-written JSON parser/writer.** ✅ Done & approved (commit `81fa1a2`, "step 1 : json done"). Wrote `JsonValue` (`Json.h`/`Json.cpp`, then in `PoC_json/`): a hand-written recursive-descent parser and pretty-printing serializer, no third-party JSON library, supporting the full JSON value model (objects, arrays, strings with escaping, numbers, booleans, `null`). `ContactTestData.h`/`.cpp` generates the 10-contact test list; `JsonTest.cpp` covers parsing, malformed input, and save/load round-trips.
- **Step 2 — Package as a static library.** ✅ Done & approved (commit `24858fc`, "step 2 : json library done"). Moved `Json.h`/`Json.cpp` into a new `JsonLib` static-library project (`JsonLib.vcxproj`) in the same solution. `PoC_json` links against it via `<ProjectReference>` and keeps `#include "Json.h"` working via an additional include path — no call-site changes needed, since `PoC_json`'s files only touch `Json.h`'s public API. `PoC_json` itself was kept (not replaced) and is what Step 3 turns into the CRUD app.
- **Step 3 — CRUD console application.** ✅ Done & approved (commit `98979ec`, "step 3 : contact CRUD app done"). Added `ContactStore.h`/`.cpp` (a `Contact` struct + `ContactStore` class doing add/update/remove/get/findByName, backed by `JsonLib`, persisted to `contacts.json`) and `ContactStoreTest.cpp` (10 tests). `main.cpp`'s Release build became the interactive menu-driven console app (List / Add / Update / Delete / Find by name / Exit), auto-saving after every mutation.
- **Step 4 — Data-quality improvements.** Not started. Three changes to `ContactStore`'s data model and the console app's input flow:
  1. **Split `name` into first/last name.** `Contact.name` becomes two fields (e.g. `firstName`/`lastName`); update the JSON schema, `ContactStore`, and the console prompts (Add/Update) accordingly.
  2. **DOB validation.** Reject invalid calendar dates (e.g. `1900-00-00`) using `std::chrono`'s calendar facilities (`std::chrono::year_month_day` + `.ok()`, C++20 standard library — no new third-party dependency). On rejection, **re-prompt** the user for DOB rather than accepting the invalid value or aborting.
  3. **Explicit skip/NA option for optional fields.** `name` (first/last) and `phone` stay **required**. `company` and `dob` become **optional**: when prompting for either, present an explicit choice — e.g. "1. Enter value  2. Skip" — and store JSON `null` for that field if skipped (not the literal string `"NA"`). Existing code that reads `company`/`dob` via `.at(...).asString()` needs to handle the `null` case.

## Decisions

- **JSON feature scope**: the hand-written parser/writer supports the full JSON value model — objects, arrays, strings (with basic escaping), numbers, booleans, and `null` — not just what the flat contact schema needs.
- **DOB representation**: a plain JSON string, e.g. `"1990-05-14"` (JSON has no native date type).
- **Random contact generation**: a one-time seed/test-data generator (fabricates 10 plausible contacts) used only for test cases — not a feature of the shipped CRUD app.
- **Step 2 project split**: the existing `PoC_json` console project is repurposed directly into the eventual CRUD app (Step 3), rather than being replaced by a separate project.
- **Test project structure**: tests live alongside the code they test within `PoC_json` (`JsonTest.cpp`, `ContactStoreTest.cpp`) rather than a separate test project; `gmock` is vendored once, in `PoC_json`, and `JsonLib` has no tests of its own — it's exercised indirectly through `PoC_json`'s tests via its public header.
- **Step 4 — name split**: `name` becomes first/last name (exact field names TBD during implementation).
- **Step 4 — DOB validation**: use `std::chrono`'s calendar validation (already-available C++20 standard library, no new dependency) to reject invalid calendar dates; re-prompt on rejection rather than rejecting the whole operation.
- **Step 4 — optional fields**: only `company` and `dob` get a skip/NA option; `name` and `phone` remain required. A skipped field is stored as JSON `null`.

## Open questions / assumptions to confirm

- **Step 4 backward compatibility**: an existing `contacts.json` written by Step 3 (flat `name` string, no `null`s) won't match the Step 4 schema. Whether to migrate old files, or just treat this as a breaking schema change (fine since `contacts.json` is gitignored, runtime-generated data) — to be decided during implementation.

## Repository structure

- `PoC_json.slnx` — Visual Studio solution, referencing both projects below.
- `JsonLib/` — static-library project (`JsonLib.vcxproj`) containing the hand-written JSON parser/serializer: `Json.h` (public API: `JsonValue`, `JsonParseError`) and `Json.cpp`.
- `PoC_json/` — the CRUD console-app project (`PoC_json.vcxproj`), linked against `JsonLib`:
  - `main.cpp` — Debug build runs `RUN_ALL_TESTS()`; Release build is the interactive contact-list console app.
  - `ContactStore.h`/`.cpp` — `Contact` struct + CRUD operations, persisted via `JsonLib`.
  - `ContactTestData.h`/`.cpp` — random 10-contact generator, test fixture data only.
  - `JsonTest.cpp`, `ContactStoreTest.cpp` — gmock tests (only compiled under `_DEBUG`).
  - `gmock` 1.11.0 vendored via `packages.config` (used only by `PoC_json`; `JsonLib` has no test dependency).

## Commands

There is no CLI build script — this is a plain Visual Studio C++ project (`PlatformToolset v145`, C++20, NuGet-restored via `packages.config`), same conventions as the sibling `assemblyCar` kata project.

- Build/run/debug: open `PoC_json.slnx` in Visual Studio, or build via MSBuild, e.g. `msbuild PoC_json.slnx /p:Configuration=Debug /p:Platform=x64`.
- Configurations: `Debug`/`Release`, each for `Win32`/`x64`.
