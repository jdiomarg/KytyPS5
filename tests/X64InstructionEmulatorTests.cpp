#include "loader/x64InstructionEmulator.h"
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <vector>

namespace {
int g_pass = 0;
int g_fail = 0;
#define CHECK(cond) do { if (cond) { ++g_pass; } else { std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); ++g_fail; } } while (0)

// ZydisDecoderDecodeFull reads up to ZYDIS_MAX_INSTRUCTION_LENGTH (15) bytes
// from Rip. The production caller always has readable guest code there.
// Pad to 16 bytes so the test obeys the same contract.
struct FakeContext {
    CONTEXT              ctx {};
    std::vector<uint8_t> code;
    std::vector<uint8_t> written;
    uint64_t             write_address = 0;
    size_t               write_size    = 0;
    int                  write_count   = 0;
    explicit FakeContext(std::initializer_list<uint8_t> bytes) {
        code.assign(bytes);
        code.resize(16, 0xcc);          // pad with INT3 (never executed here)
        ctx.ContextFlags = CONTEXT_ALL;
        ctx.Rip          = reinterpret_cast<DWORD64>(code.data());
    }
};
FakeContext* g_active = nullptr;
bool RecordWriter(uint64_t address, const void* data, size_t size) {
    if (g_active) {
        g_active->write_address = address;
        g_active->write_size    = size;
        g_active->written.assign(static_cast<const uint8_t*>(data),
                                 static_cast<const uint8_t*>(data) + size);
        ++g_active->write_count;
    }
    return true;
}
void TestStoreMovRegToMem() {
    FakeContext fc {0x48, 0x89, 0x18};
    fc.ctx.Rax = 0x1000; fc.ctx.Rbx = 0xDEADBEEFCAFEBABEull;
    g_active = &fc;
    const bool r = Loader::X64InstructionEmulator::EmulateStore(&fc.ctx, RecordWriter);
    g_active = nullptr;
    CHECK(r == true); CHECK(fc.write_count == 1); CHECK(fc.write_address == 0x1000); CHECK(fc.write_size == 8);
    const uint8_t expected[8] = {0xBE,0xBA,0xFE,0xCA,0xEF,0xBE,0xAD,0xDE};
    CHECK(fc.written.size() == 8); CHECK(std::memcmp(fc.written.data(), expected, 8) == 0);
    CHECK(fc.ctx.Rip == reinterpret_cast<DWORD64>(fc.code.data()) + 3);
}
void TestStoreImmediateToMem() {
    FakeContext fc {0xC7, 0x00, 0x2A, 0x00, 0x00, 0x00};
    fc.ctx.Rax = 0x2000;
    g_active = &fc;
    const bool r = Loader::X64InstructionEmulator::EmulateStore(&fc.ctx, RecordWriter);
    g_active = nullptr;
    CHECK(r == true); CHECK(fc.write_count == 1); CHECK(fc.write_address == 0x2000); CHECK(fc.write_size == 4);
    const uint8_t expected[4] = {0x2A,0x00,0x00,0x00};
    CHECK(std::memcmp(fc.written.data(), expected, 4) == 0);
}
void TestStoreImmediate64() {
    FakeContext fc {0x48, 0xC7, 0x00, 0x2A, 0x00, 0x00, 0x00};
    fc.ctx.Rax = 0x3000;
    g_active = &fc;
    const bool r = Loader::X64InstructionEmulator::EmulateStore(&fc.ctx, RecordWriter);
    g_active = nullptr;
    CHECK(r == true); CHECK(fc.write_size == 8);
    const uint8_t expected[8] = {0x2A,0,0,0,0,0,0,0};
    CHECK(std::memcmp(fc.written.data(), expected, 8) == 0);
}
void TestNonStoreReturnsFalse() {
    FakeContext fc {0x90};
    g_active = &fc;
    const bool r = Loader::X64InstructionEmulator::EmulateStore(&fc.ctx, RecordWriter);
    g_active = nullptr;
    CHECK(r == false); CHECK(fc.write_count == 0);
}
// Line 920 of x64InstructionEmulator.cpp: FS-relative stores return false.
void TestFsSegmentReturnsFalse() {
    FakeContext fc {0x64, 0x48, 0x89, 0x18};   // MOV [FS:RAX], RBX
    fc.ctx.Rax = 0x4000;
    g_active = &fc;
    const bool r = Loader::X64InstructionEmulator::EmulateStore(&fc.ctx, RecordWriter);
    g_active = nullptr;
    CHECK(r == false); CHECK(fc.write_count == 0);
}
// Line 920 of x64InstructionEmulator.cpp: GS-relative stores return false.
void TestGsSegmentReturnsFalse() {
    FakeContext fc {0x65, 0x48, 0x89, 0x18};   // MOV [GS:RAX], RBX
    fc.ctx.Rax = 0x5000;
    g_active = &fc;
    const bool r = Loader::X64InstructionEmulator::EmulateStore(&fc.ctx, RecordWriter);
    g_active = nullptr;
    CHECK(r == false); CHECK(fc.write_count == 0);
}
} // namespace

int main() {
    TestStoreMovRegToMem();
    TestStoreImmediateToMem();
    TestStoreImmediate64();
    TestNonStoreReturnsFalse();
    TestFsSegmentReturnsFalse();
    TestGsSegmentReturnsFalse();
    std::printf("x64_emulator_tests: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}