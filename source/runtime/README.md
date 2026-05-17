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

## SDK Boundary

Runtime headers under `source/runtime` are private implementation files. They must not be installed with the public SDK headers and must not be included from `include/AssetSuite`.

The public SDK surface is guarded by `PublicHeaderCompile`, `PublicHeaderHygiene.ps1`, and `AssertPublicInstallSurface.ps1`. These checks reject private runtime names in public headers and verify that the installed include tree contains only `AssetSuite/*.h`.

## Blob Representation

`Internal::Blob` stores raw asset bytes in runtime-owned memory and copies minimal source metadata into the runtime object. The metadata currently tracks the original source path, source extension, and best-known public `AssetFormat` derived from the extension when possible.

`BlobHandle` is treated as an opaque context/slot/generation token by the runtime. The public type remains pointer-shaped for ABI opacity, but blob validation decodes the token and never dereferences the handle value directly. This token encoding requires a 64-bit target and is guarded by a compile-time assertion.

`RuntimeState::BlobStorage` owns reusable blob slots for one context. `ReleaseBlob` clears the blob payload, advances the slot generation, returns the slot to the free list, and nulls the caller's handle. Copied stale handles are rejected by generation mismatch, and handles from another context are rejected by context-id mismatch before slot lookup.

## Deferred Scope

This runtime layer is intentionally foundational. The following work is deferred to later stories:

- image and mesh child-handle tracking
- shared file loading APIs
- codec probing and richer format detection
- public diagnostics reporting
- allocator hooks wired into all internal allocations
- broader asset storage and cleanup policies
