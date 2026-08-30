# Pebble StrongLifts

Pebble StrongLifts is a proposed workout tracker for the current Pebble watch lineup. It is intentionally limited to the standard StrongLifts 5x5 Workout A and Workout B flow and is designed for fast, low-distraction use while lifting.

This repository is currently in requirements and design discussion. No application implementation has begun.

## Documentation

- [Product requirements](docs/requirements.md)
- [Architecture and data strategy](docs/architecture.md)
- [Development discussion guide](docs/development-guide.md)
- [Research sources](docs/research-sources.md)

## Product direction

The intended product is one installable Pebble watchapp with a bundled PebbleKit JS component that runs inside the official Pebble Android app. Users must not need to install or sideload a separate Android companion application.

The watch must remain useful without a phone connection. The bundled phone-side component may retain longer-term history and synchronize with the watch when available. Import and export are desirable, but are not guaranteed acceptance criteria because the PebbleKit JS sandbox may prevent a clean Android file workflow.

## Current status

The next step is to resolve the remaining design questions in [the development discussion guide](docs/development-guide.md), then turn the accepted requirements into an implementation plan.
