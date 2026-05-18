# AssetSuite Runtime Internals

## Runtime Ownership

`AssetSuite::ContextHandle` points to the private `AssetSuiteContext_t` bridge in `source/common`. That bridge owns one `Internal::RuntimeContext`, and the runtime context owns the per-context runtime state used by the current SDK adapter layer.

The runtime currently owns:

- the normalized `ContextDesc`
- logger callback state
- allocator policy placeholders
- diagnostics storage
- file loading support
- private blob handle storage, raw bytes, and copied source metadata
- source file metadata
- raw, decoded, and formatted buffers
- transient image and mesh metadata
- codec registry access
- codec, encoder, and Wavefront loader instances

Legacy `Manager` remains as a compatibility facade for existing tests and demo code. It is bound to `RuntimeState` instead of owning runtime buffers and codec objects directly.

## Codec Ownership

`RuntimeState::CodecStorage` owns the concrete codec, encoder, and loader instances for one runtime context. `RuntimeState::CodecRegistry` is a non-owning lookup table over those instances and must not allocate codecs or store process-global mutable codec state.

Only concrete codec identifiers may be registered or queried. Sentinel values such as `Auto` and `MaxDecoders` are resolved or rejected before lookup.

The built-in registry treats `PpmEncoder` as the currently supported image encoder path. Format probing should prefer recognized signatures or content markers over filename extensions, with extensions used as a fallback when bytes are unavailable or inconclusive. If an extension and signature disagree, the recognized byte signature is authoritative.

## SDK Boundary

Runtime headers under `source/runtime` are private implementation files. They must not be installed with the public SDK headers and must not be included from `include/AssetSuite`.

The public SDK surface is guarded by `PublicHeaderCompile`, `PublicHeaderHygiene.ps1`, and `AssertPublicInstallSurface.ps1`. These checks reject private runtime names in public headers and verify that the installed include tree contains only `AssetSuite/*.h`.

## Blob Representation

`Internal::Blob` stores raw asset bytes in runtime-owned memory and copies minimal source metadata into the runtime object. The metadata currently tracks the original source path, source extension, and best-known public `AssetFormat` derived from the extension when possible.

`BlobHandle` is treated as an opaque context/slot/generation token by the runtime. The public type remains pointer-shaped for ABI opacity, but blob validation decodes the token and never dereferences the handle value directly. This token encoding requires a 64-bit target and is guarded by a compile-time assertion.

`RuntimeState::BlobStorage` owns reusable blob slots for one context. `ReleaseBlob` clears the blob payload, advances the slot generation, returns the slot to the free list, and nulls the caller's handle. Copied stale handles are rejected by generation mismatch, and handles from another context are rejected by context-id mismatch before slot lookup.

## File Loading

`RuntimeState::FileLoader` is the shared file-read service for public blob loading and the legacy `Manager` image/mesh file entry points. Missing paths return `ErrorCode::NonExistingFile`, while existing paths that cannot be opened, sized, or fully read return `ErrorCode::IoFailure`.

The public SDK adapter maps `NonExistingFile` to `Result::ErrorFileNotFound` and `IoFailure` to `Result::ErrorIoFailure`. Text-mode loads append a null terminator after successful reads; binary loads preserve the exact file bytes.

## Deferred Scope

This runtime layer is intentionally foundational. The following work is deferred to later stories:

- image and mesh child-handle tracking
- codec probing and richer format detection
- public diagnostics reporting
- allocator hooks wired into all internal allocations
- broader asset storage and cleanup policies
