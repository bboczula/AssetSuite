# Legacy Internal Demo

This demo exercises the pre-2.0 internal implementation API from `source/common`.
It intentionally uses legacy types and headers such as `AssetSuite::Manager`,
`BYTE`, `FLOAT`, and STL containers.

Do not use this directory as public SDK integration guidance. External consumers
should include `<AssetSuite/AssetSuite.h>` from the installed include root and
follow the 2.0 SDK surface documented in the repository README.
