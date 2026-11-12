# Architecture Map
src/common/ — config, logging, platform
src/loader/ — ELF loading, runtimeLinker.cpp (imports/stubs)
src/kernel/ — memory.cpp (guest VMM)
src/graphics/
  guest_gpu/ — PM4 command processor
  host_gpu/renderer/ — renderDraw.cpp, pipeline/, cache/
  shader/recompiler/ — PS5 ISA -> IR -> SPIR-V
  presentation/window/vulkanWindow.cpp, presentation/performanceOverlay.cpp
src/libs/ — PS5 userland libraries
src/launcher/ — Qt6 GUI
tests/ — ~40 test binaries (EXCLUDE_FROM_ALL)
3rdparty/ — ~15 vendored deps