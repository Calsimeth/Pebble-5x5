# import/export feasibility: import/export feasibility

Status: investigated 2026-08-31. No user-facing import or export was implemented.

## Conclusion

Clean StrongLifts CSV import and history export are not feasible within the bundled PebbleKit JS runtime and the official Pebble Android app alone. The documented runtime has no API for opening a user-selected local file, receiving an Android `content://` URI, writing a user-visible file, or invoking the Android share sheet. A configuration webview is a settings transport, not a document picker or export destination. A separate Android companion, sideloaded software, or undocumented bridge would violate the product constraints.

The official StrongLifts CSV remains the only intended import format, but parser compatibility is unverified because no representative, legally usable fixture is present in this repository.

## Evidence examined

- The [PebbleKit JS guide](https://developer.repebble.com/guides/communication/using-pebblekit-js/) documents app-scoped `localStorage`, `XMLHttpRequest`, geolocation, and configuration pages. It describes the JS component as running in a sandbox inside the official mobile app.
- The [PebbleKit JS API](https://developer.repebble.com/docs/pebblekit-js/Pebble/) documents communication, `openURL`, and configuration events (`showConfiguration` and `webviewclosed`), but no file picker, filesystem, Android intent, URI permission, or share API.
- The [App Configuration guide](https://developer.repebble.com/guides/user-interfaces/app-configuration-static/) describes an online/local web page returning encoded configuration through the webview close URL; it does not describe a general-purpose document workflow.
- The [Persistent Storage guide](https://developer.repebble.com/guides/events-and-services/persistent-storage/) describes phone-side `localStorage` as key/value persistence and notes that it is unavailable while disconnected.

## Experiment and constraints

The committed persistence/synchronization `src/pkjs/index.js` and `package.json` were inspected against that API surface. The implementation has only AppMessage input and app-scoped `localStorage` output, split into history chunks and an index. No file, picker, URI, intent, or share primitive exists in the project or examined documentation. The existing dependency-free sync tests provide no CSV fixture. A physical-device experiment cannot demonstrate a supported workflow without a documented API boundary; undocumented OEM behavior would not justify a compatibility claim.

| User flow | Result | Reason |
| --- | --- | --- |
| Select a CSV file | Not feasible | No documented picker or file/URI callback. |
| Preview and validate | Feasible in isolation | JavaScript can parse text, but no supported way supplies user-file text. |
| Import into history | Conditional only | `localStorage` can store validated records, but acquisition and safe staging are missing. |
| Export to file/share destination | Not feasible | No documented file writer, Android intent, or share-sheet API. |

## Data safety

`localStorage` is app-scoped persistence, not a user-browsable backup. It can fail or become unavailable and is not an export. Any future importer must stage rows, detect headers, validate required fields, confirm ambiguous units, deduplicate stably, and commit atomically without overwriting synchronized history. No importer is included in this slice. Uninstall/factory reset must not be described as guaranteed backup or recovery.

## Product decision

Ship import/export feasibility as documentation-only. Keep StrongLifts CSV import/export unsupported and aspirational until official platform support exposes a clean user-controlled file/share workflow and a representative legally usable fixture is available. No separate Android application or undocumented bridge should be added.
