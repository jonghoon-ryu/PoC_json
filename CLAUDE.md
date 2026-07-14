# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Objective

**The objective of this project is to build a JSON-backed contact list CRUD console application, in three ordered steps, building up from a hand-written JSON library.**

This is a from-scratch C++ exercise (no third-party JSON library): first write a JSON parser/serializer, then package it as a static library, then build a console CRUD app on top of it that persists contact records to a JSON file.

Test data throughout: a list of **10 randomly generated contacts**, each with `name`, `phone number`, `company name`, `DOB`. This same contact schema is used for parser testing (Step 1) and as the CRUD app's data model (Step 3).

## Steps

Follow these steps **in order**. Do not start a step before the previous one has been reviewed and approved by the user. After finishing a step:

1. Ask the user to review the result.
2. Only once the user agrees, `git commit` with message `"step X"` (X = the step number), then `git push` to the remote (`https://github.com/jonghoon-ryu/PoC_json.git`).

- **Step 1 — Hand-written JSON parser/writer.** Write a C++ program that parses JSON text into an in-memory representation and serializes it back to JSON text, with no third-party JSON library. Use the 10-contact list (name, phone number, company name, DOB) as the working test data: generate it, save it to a `.json` file, and parse it back. Include test code (gmock, already vendored in this project) covering parse and save/round-trip behavior.
- **Step 2 — Package as a static library.** Move the Step 1 JSON code into its own static-library project (`.lib`) within the same solution, with a public header exposing the parse/serialize API. The existing console-app project either becomes a thin test/demo harness for the library or is replaced by a new one — decide during the step and confirm with the user. Include test code exercising the library through its public headers only (not internal details).
- **Step 3 — CRUD console application.** Build an interactive console app, linked against the Step 2 library, that manages the contact list (name, phone number, company name, DOB) persisted as a JSON file: Create, Read (list/find), Update, Delete. Include test code for the CRUD operations (e.g. by testing the underlying data-manipulation functions directly, since the interactive I/O loop itself is not unit-testable the same way).

## Decisions

- **JSON feature scope**: the hand-written parser/writer supports the full JSON value model — objects, arrays, strings (with basic escaping), numbers, booleans, and `null` — not just what the flat contact schema needs.
- **DOB representation**: a plain JSON string, e.g. `"1990-05-14"` (JSON has no native date type).
- **Random contact generation**: a one-time seed/test-data generator (fabricates 10 plausible contacts) used only for test cases — not a feature of the shipped CRUD app.
- **Step 2 project split**: the existing `PoC_json` console project is repurposed directly into the eventual CRUD app (Step 3), rather than being replaced by a separate project.

## Open questions / assumptions to confirm

- **Test project structure**: whether tests live in one shared test project (as in the `assemblyCar` sibling project) or per-library, and whether the existing vendored `gmock` package should be shared across all projects in the solution or vendored per-project — to be decided as each step's project structure is built out.

## Repository structure

- `PoC_json.slnx` — Visual Studio solution.
- `PoC_json/` — currently a single scaffolded console-app project (`PoC_json.vcxproj`, `main.cpp` just runs `RUN_ALL_TESTS()` under `_DEBUG`) with `gmock` 1.11.0 vendored via `packages.config`. This will grow into multiple projects (library, app, possibly tests) as the steps above proceed.

## Commands

There is no CLI build script — this is a plain Visual Studio C++ project (`PlatformToolset v145`, C++20, NuGet-restored via `packages.config`), same conventions as the sibling `assemblyCar` kata project.

- Build/run/debug: open `PoC_json.slnx` in Visual Studio, or build via MSBuild, e.g. `msbuild PoC_json.slnx /p:Configuration=Debug /p:Platform=x64`.
- Configurations: `Debug`/`Release`, each for `Win32`/`x64`.
