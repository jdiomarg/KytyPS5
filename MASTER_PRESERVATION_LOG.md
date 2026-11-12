# Changes Log 2026-11-12

APPLIED — tests/VdecswTests.cpp:62,68
- Line 62: std::aligned_alloc -> _aligned_malloc (MSVC STL lacks std::aligned_alloc)
- Line 68: std::free -> _aligned_free (must match _aligned_malloc; free() on
  _aligned_malloc memory is UB and corrupts the heap on MSVC)
- Added #include <malloc.h>.
ATTEMPTED/REVERTED — src/common/emulatorConfig.h:93 — --tessellation is a documented user flag
RULED OUT — streamBuffer.cpp:292 — false positive
ADDED — .github/workflows/ci.yml