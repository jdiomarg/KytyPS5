# Source Register (verified at f399d481)

| File | Lines | Fact |
|------|-------|------|
| src/common/emulatorConfig.h | 91-95 | feature flags, all default false |
| src/graphics/host_gpu/renderer/renderDraw.cpp | 776 | gate on TessellationEnabled() |
| src/graphics/shader/recompiler/Tessellation.cpp | full | LS/HS recompiler |
| src/graphics/shader/recompiler/backend/spirv/spirvEmitterAnalysis.cpp | 133-134 | CapabilityStorageImageMultisample |
| src/graphics/shader/recompiler/backend/spirv/spirvEmitterModule.cpp | 826-835 | CapabilityFragmentBarycentricKHR |
| src/graphics/shader/recompiler/backend/spirv/spirvEmitterFlow.cpp | 43-227 | barycentric math |
| src/graphics/shader/recompiler/frontend/translate/Translate.cpp | 1366-1376 | PS5 regs -> IR |
| src/graphics/presentation/window/vulkanWindow.cpp | 535-555 | MSAA storage feature + fallback |
| src/loader/runtimeLinker.cpp | 111-208, 883-891 | stub registry, fail-loud |
| tests/VdecswTests.cpp | 62 | FIXED |

Absent: renderCommands.cpp, XBOX_CHANGES.md, src/uwp/, .appxmanifest, UWP CMake target, WINAPI_PARTITION guards