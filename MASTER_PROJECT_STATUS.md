# KytyPS5 for Xbox — Project Status
Baseline audited: f399d481 (2026-11-09)
Session: 2026-11-13
Build state: WORKING (Windows desktop)

## Phase 2 — D3D12 tessellation: CODE COMPLETE
- 2.A: SpirvStage maps HS/DS to DXIL (badf91aa)
- 2.B.1/2: LinkedStage::Kind::{Hull,Domain} + LinkedPrograms fields (94bcc964)
- 2.B.1: dxilCompiler switch handles Hull/Domain (fdbab842)
- 2.B.5a: LinkKey extended with hs/ds slots (41649b26)
- 2.B.5b: Link() builds 3-stage pipelines (7352eb0d)
- 2.B.5c: rect_list excludes tess (298d1144)
- 2.B.6: PSO stream HS/DS/TESSELLATION subobjects (fa6a2727)
- 2.C: Native mode + CLI flag (440dbad3)
- 2.C: CLI flag in --help (b30660e0)

## Test suite
46/80 pass, 34/80 blocked by local AMD iGPU (needs VK_EXT_pipeline_robustness)

## Blockers
1. No PS5 game dumps available for runtime testing
2. No Xbox GDK installed / no UWP files on branch

## Next actions
1. Investigate Xbox Dev Mode and UWP build
2. Obtain game dump -> run --d3d12-tessellation native
