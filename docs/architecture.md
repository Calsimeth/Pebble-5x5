# Architecture and data strategy

## Architecture decision

The product should be delivered as one Pebble application package containing:

1. A C watchapp targeting the current Pebble platforms.
2. A bundled PebbleKit JS component that runs in the official Pebble Android app.

A separate native Android companion application is explicitly out of scope.

## Why C is the likely watch language

The current target lineup spans `flint`, `emery`, and `gabbro`. Alloy currently supports `emery` and `gabbro`, but not `flint`. Supporting Pebble 2 Duo therefore points to the C SDK unless platform support changes before implementation begins.

This decision should be revalidated against the current SDK immediately before scaffolding the project.

## Component responsibilities

### Watchapp

The watchapp owns the workout experience and must function offline. It is responsible for:

- Workout A/B sequencing
- Set and repetition entry
- Rest timers and vibration
- Plate calculation
- Warm-up calculation
- Progression and deload suggestions
- Current-workout persistence
- A compact synchronization outbox
- Watch UI and platform-specific layouts

### Bundled PebbleKit JS

The phone-side component runs inside the official Pebble mobile app. It should be responsible for:

- Receiving completed workout records
- Acknowledging durable receipt
- Maintaining longer-term history in phone-side `localStorage`
- Sending required history or settings back to the watch
- Deduplicating retried messages
- Providing configuration capabilities supported by the Pebble environment
- No import/export in the current release: PebbleKit JS has no documented file picker, file writer, or Android share bridge.

It must not require a separate installation.

## Storage tiers

### Tier 1: active state on the watch

The watch must persist the minimum state needed to resume safely:

- Storage schema version
- Current workout identifier and type
- Current exercise and set
- Logged sets in the active workout
- Timer start or expiry time
- Next target per exercise
- Failure and deload state per exercise
- Last completed workout date
- Next expected Workout A/B value

Traditional Pebble persistent storage is constrained, historically around 4 KB per app, so the active state uses a compact schema-9 representation; history leaves the watch as bounded compact JSON.

### Tier 2: watch synchronization outbox

Completed records not yet acknowledged by the phone must remain queued on the watch. The queue should be bounded and must never silently overwrite an unsynchronized record without a visible warning.

Each message carries a stable record identifier so retries are idempotent. Slice nine uses protocol v1 AppMessage keys `message` (compact JSON record) and `ack` (uint32 record ID). Records contain `v`, `id`, `t`, `w`, `e`, `wt`, `r`, `c`, and `d`. The watch outbox has three queued records plus one pending completion; a fifth completion is blocked before final-set reps are written. Transport delivery does not remove a record: the head remains in flight until its matching ACK, with a 5/15/30/60-second bounded retry sequence.

### Tier 3: phone-side history

PebbleKit JS `localStorage` is the intended long-term store. It is larger than watch persistence but should not be treated as an unlimited or user-browsable filesystem.

History should be stored in versioned chunks rather than one indefinitely growing JSON value. A possible organization is:

```text
schemaVersion
settings
historyIndex
historyChunk:0001
historyChunk:0002
...
```

Chunking reduces the amount rewritten for every workout and creates a path for bounded transfers and migrations.

## Synchronization protocol

The protocol should be designed before UI implementation.

Implemented flow:

1. The watch saves a completed record locally.
2. The watch sends a compact record with a stable identifier.
3. PebbleKit JS validates and stores the record.
4. PebbleKit JS responds with an acknowledgement containing the identifier.
5. Only after acknowledgement may the watch remove the record from its outbox.
6. Repeated delivery of the same identifier must not create duplicates.

The phone stores `schemaVersion`, `historyIndex`, and separate `historyChunk:0001`-style JSON values in PebbleKit JS `localStorage`. It scans all chunk keys before every store, reconciles orphan records by ID, chooses an unused key, and writes the chunk before updating the index. It sends ACK only after both writes succeed. Corrupt indexes or chunks are preserved byte-for-byte; orphan chunks are recovered without silent deletion, overwriting, or pruning. Schema-9 state validates queue count, IDs, workout IDs, exercise IDs, weight snapshots, repetition counts, duplicate IDs, selected repetitions, and blocked completion state; invalid queued data is rejected while active workout state is preserved.

AppMessage payloads are size-limited, so large records may require chunking. Protocol messages should contain a schema version and message type.

## Data model principles

- Historical facts are immutable by default.
- Corrections create an explicit replacement or revision rather than silently changing unrelated progression state.
- Recommendations are derived from workout facts and current settings.
- Weights should use scaled integers, not floating point. For example, store quarter-pounds or grams as integers.
- Dates and times should use an unambiguous epoch representation plus any local-date fields needed for display.
- Every stored structure must include or inherit a schema version.

## Import/export boundary

StrongLifts CSV compatibility remains an unsupported, aspirational capability. Slice 10 found no documented PebbleKit JS or official Pebble Android API for selecting a local file, receiving a content URI, writing a user-accessible file, or opening the Android share sheet. See [the Slice 10 feasibility report](import-export-feasibility.md).

The internal model should nevertheless be rich enough to represent the fields in a StrongLifts export so that a future migration utility can be built without redesigning history storage.

## Platform layouts

The UI should share state and behavior while using platform-specific presentation where needed:

- `flint`: monochrome rectangular display
- `emery`: larger color rectangular display
- `gabbro`: color round display

The 20-character text rule applies to every watch platform. Layout tests should include the narrowest practical text area, not merely the largest screen.

## Security and privacy

Workout data is local personal data. The initial product should not require an account, cloud service, analytics service, or network API. Any future external synchronization must be opt-in and separately specified.
