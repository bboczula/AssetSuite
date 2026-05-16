# AssetSuite Runtime Internals

## Codec Ownership

`RuntimeState::CodecStorage` owns the concrete codec, encoder, and loader instances for one runtime context. `RuntimeState::CodecRegistry` is a non-owning lookup table over those instances and must not allocate codecs or store process-global mutable codec state.

Only concrete codec identifiers may be registered or queried. Sentinel values such as `Auto` and `MaxDecoders` are resolved or rejected before lookup.
