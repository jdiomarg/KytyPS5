# KytyPS5 for Xbox — Project Status
Baseline audited: f399d481 (2026-11-09)
Branch tip: 1b2fc23d (229 commits ahead)
Session: 2026-11-12

## Test suite (at f399d481)
- 46/80 pass, 34/80 GPU-gated on local AMD iGPU, 0 assertion failures

## Bugs found and fixed
| File | Line | Class | Status |
|------|------|-------|--------|
| tests/VdecswTests.cpp | 62 | test-only | FIXED aligned_alloc -> _aligned_malloc |

## Bugs ruled out
| File | Line | Verdict |
|------|------|---------|
| src/graphics/host_gpu/renderer/cache/streamBuffer.cpp | 292 | FALSE POSITIVE |

## Audit claims — final verdict (verified at f399d481)
| Claim | Verdict |
|-------|---------|
| renderCommands.cpp skips tessellation | FALSE — file does not exist |
| Tessellation unsupported | FALSE — full impl + --tessellation flag |
| runtimeLinker.cpp stubs | TRUE but already fail-loud |
| XBOX_CHANGES.md documents paging | FALSE — file does not exist |
| Barycentric unsupported | FALSE — 9 files, CapabilityFragmentBarycentricKHR |
| Multisampled-storage limitations | FALSE — capability + device feature + fallback |
| GPU memory growth | UNVERIFIED — needs runtime |

## ChatGPT recs
DONE: shader cache, perf overlay, fail-loud stubs, stub telemetry, paging, mapping, haptics, HDR
ADDED: CI workflow
REMAINING: telemetry/crash reporting, predictive streaming, compression, golden list, Xbox/UWP packaging

## Blockers
- Xbox GDK not installed
- No Xbox/UWP files on this branch
- No PS5 game dumps available