//! Types exchanged with the frontend. TypeScript bindings are generated from them by `tauri-rs-ts-ipc`
//! (`src/utils/backend/models.ts`), so they must not use serde renames: field and variant names are sent as is.
//! Types whose name contains `App` are taken for an `AppHandle` by the generator, avoid them as event payloads.

pub mod app;
pub mod fans;
pub mod games;
