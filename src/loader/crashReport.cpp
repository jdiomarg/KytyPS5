#include "loader/crashReport.h"

#include "common/common.h"
#include "common/debugCounters.h"
#include "kernel/memory.h"

#if KYTY_PLATFORM == KYTY_PLATFORM_WINDOWS
#include <windows.h>
#include <shlobj.h>
#endif

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>

namespace Loader::CrashReport {

namespace {

std::filesystem::path ComputeDirectory() {
#if KYTY_PLATFORM == KYTY_PLATFORM_WINDOWS
    PWSTR p = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &p))) {
        std::filesystem::path dir{p};
        CoTaskMemFree(p);
        dir /= L"Kyty";
        dir /= L"crashes";
        return dir;
    }
    return std::filesystem::current_path() / "crashes";
#else
    const char* xdg = std::getenv("XDG_DATA_HOME");
    if (xdg != nullptr && xdg[0] != '\0') {
        return std::filesystem::path{xdg} / "Kyty" / "crashes";
    }
    const char* home = std::getenv("HOME");
    return std::filesystem::path{home != nullptr ? home : "."} / ".local" / "share" / "Kyty" / "crashes";
#endif
}

const std::filesystem::path& CrashDirectory() {
    static const auto dir = ComputeDirectory();
    return dir;
}

std::string& PathBuffer() {
    static thread_local std::string s;
    return s;
}

void WriteFault(FILE* f, const FaultInfo* fault) {
    if (fault == nullptr) {
        std::fprintf(f, "fault=null\n");
        return;
    }
    std::fprintf(f, "fault.exception_type=%d\n",         fault->exception_type);
    std::fprintf(f, "fault.exception_address=0x%llx\n",  static_cast<unsigned long long>(fault->exception_address));
    std::fprintf(f, "fault.access_violation_type=%d\n",  fault->access_violation_type);
    std::fprintf(f, "fault.access_violation_vaddr=0x%llx\n", static_cast<unsigned long long>(fault->access_violation_vaddr));
    std::fprintf(f, "fault.rax=0x%llx\n", static_cast<unsigned long long>(fault->rax));
    std::fprintf(f, "fault.rbx=0x%llx\n", static_cast<unsigned long long>(fault->rbx));
    std::fprintf(f, "fault.rcx=0x%llx\n", static_cast<unsigned long long>(fault->rcx));
    std::fprintf(f, "fault.rdx=0x%llx\n", static_cast<unsigned long long>(fault->rdx));
    std::fprintf(f, "fault.rsi=0x%llx\n", static_cast<unsigned long long>(fault->rsi));
    std::fprintf(f, "fault.rdi=0x%llx\n", static_cast<unsigned long long>(fault->rdi));
    std::fprintf(f, "fault.rbp=0x%llx\n", static_cast<unsigned long long>(fault->rbp));
    std::fprintf(f, "fault.rsp=0x%llx\n", static_cast<unsigned long long>(fault->rsp));
    std::fprintf(f, "fault.r8 =0x%llx\n", static_cast<unsigned long long>(fault->r8));
    std::fprintf(f, "fault.r9 =0x%llx\n", static_cast<unsigned long long>(fault->r9));
    std::fprintf(f, "fault.r10=0x%llx\n", static_cast<unsigned long long>(fault->r10));
    std::fprintf(f, "fault.r11=0x%llx\n", static_cast<unsigned long long>(fault->r11));
    std::fprintf(f, "fault.r12=0x%llx\n", static_cast<unsigned long long>(fault->r12));
    std::fprintf(f, "fault.r13=0x%llx\n", static_cast<unsigned long long>(fault->r13));
    std::fprintf(f, "fault.r14=0x%llx\n", static_cast<unsigned long long>(fault->r14));
    std::fprintf(f, "fault.r15=0x%llx\n", static_cast<unsigned long long>(fault->r15));
}

void WriteCounters(FILE* f) {
    using C = Common::DebugCounters::Counter;
    using P = Common::DebugCounters::Peak;
    std::fprintf(f, "counters_enabled=%s\n", Common::DebugCounters::Enabled() ? "true" : "false");
    struct Entry { const char* name; C c; };
    const Entry entries[] = {
        {"draws",                      C::Draws},
        {"draw_instances",             C::DrawInstances},
        {"indirect_draws",             C::IndirectDraws},
        {"dispatches",                 C::Dispatches},
        {"queue_submits",              C::QueueSubmits},
        {"gpu_waits",                  C::GpuWaits},
        {"gpu_wait_ns",                C::GpuWaitNs},
        {"gpu_faults",                 C::GpuFaults},
        {"shaders_compiled",           C::ShadersCompiled},
        {"shader_compile_ns",          C::ShaderCompileNs},
        {"pipelines_created",          C::PipelinesCreated},
        {"pipeline_create_ns",         C::PipelineCreateNs},
        {"texture_upload_bytes",       C::TextureUploadBytes},
        {"texture_download_bytes",     C::TextureDownloadBytes},
        {"texture_evictions",          C::TextureEvictions},
        {"color_metadata_readbacks",   C::ColorMetadataReadbacks},
        {"color_metadata_readback_ns", C::ColorMetadataReadbackNs},
        {"color_metadata_checks",      C::ColorMetadataChecks},
        {"buffer_upload_bytes",        C::BufferUploadBytes},
        {"buffer_download_bytes",      C::BufferDownloadBytes},
        {"buffer_evictions",           C::BufferEvictions},
        {"flips_requested",            C::FlipsRequested},
        {"flips_rejected",             C::FlipsRejected},
        {"late_vblanks",               C::LateVblanks},
        {"swapchain_recreations",      C::SwapchainRecreations},
        {"dropped_presents",           C::DroppedPresents},
        {"emulated_instructions",      C::EmulatedInstructions},
        {"unresolved_import_calls",    C::UnresolvedImportCalls},
        {"guest_file_read_bytes",      C::GuestFileReadBytes},
        {"guest_file_write_bytes",     C::GuestFileWriteBytes},
        {"audio_underruns",            C::AudioUnderruns},
        {"gpu_thread_busy_ns",         C::GpuThreadBusyNs},
        {"present_ns",                 C::PresentNs},
        {"present_acquire_ns",         C::PresentAcquireNs},
        {"overlay_ns",                 C::OverlayNs},
    };
    for (const auto& e : entries) {
        std::fprintf(f, "%s=%llu\n", e.name,
                     static_cast<unsigned long long>(Common::DebugCounters::Get(e.c)));
    }
    std::fprintf(f, "peak_shader_compile_ns=%llu\n",
                 static_cast<unsigned long long>(Common::DebugCounters::Get(P::ShaderCompileNs)));
    std::fprintf(f, "peak_pipeline_create_ns=%llu\n",
                 static_cast<unsigned long long>(Common::DebugCounters::Get(P::PipelineCreateNs)));
}

void WriteMemory(FILE* f) {
    const auto mem = Libs::LibKernel::Memory::GetDebugStats();
    std::fprintf(f, "direct_total=%llu\n",           static_cast<unsigned long long>(mem.direct_total));
    std::fprintf(f, "direct_allocated=%llu\n",       static_cast<unsigned long long>(mem.direct_allocated));
    std::fprintf(f, "direct_mapped=%llu\n",          static_cast<unsigned long long>(mem.direct_mapped));
    std::fprintf(f, "pooled_allocated=%llu\n",       static_cast<unsigned long long>(mem.pooled_allocated));
    std::fprintf(f, "automatic_allocated=%llu\n",    static_cast<unsigned long long>(mem.automatic_allocated));
    std::fprintf(f, "flexible_total=%llu\n",         static_cast<unsigned long long>(mem.flexible_total));
    std::fprintf(f, "flexible_used=%llu\n",          static_cast<unsigned long long>(mem.flexible_used));
    std::fprintf(f, "pool_committed=%llu\n",         static_cast<unsigned long long>(mem.pool_committed));
    std::fprintf(f, "cpu_page_entries=%llu\n",       static_cast<unsigned long long>(mem.cpu_page_entries));
    std::fprintf(f, "gpu_page_entries=%llu\n",       static_cast<unsigned long long>(mem.gpu_page_entries));
    std::fprintf(f, "cpu_page_entries_total=%llu\n", static_cast<unsigned long long>(mem.cpu_page_entries_total));
    std::fprintf(f, "gpu_page_entries_total=%llu\n", static_cast<unsigned long long>(mem.gpu_page_entries_total));

    const auto gt = Libs::LibKernel::Memory::GetGuestMemoryTotals();
    std::fprintf(f, "resident=%llu\n",     static_cast<unsigned long long>(gt.resident));
    std::fprintf(f, "swapped=%llu\n",      static_cast<unsigned long long>(gt.swapped));
    std::fprintf(f, "swapped_out=%llu\n",  static_cast<unsigned long long>(gt.swapped_out));
    std::fprintf(f, "swapped_in=%llu\n",   static_cast<unsigned long long>(gt.swapped_in));
    std::fprintf(f, "pool_bytes=%llu\n",   static_cast<unsigned long long>(gt.pool_bytes));
    std::fprintf(f, "pool_hits=%llu\n",    static_cast<unsigned long long>(gt.pool_hits));
    std::fprintf(f, "pool_misses=%llu\n",  static_cast<unsigned long long>(gt.pool_misses));
    std::fprintf(f, "swap_wait_ns=%llu\n", static_cast<unsigned long long>(gt.swap_wait_ns));
    std::fprintf(f, "pager=%s\n",          gt.pager ? "true" : "false");
}

} // namespace

const char* Directory() {
    static const std::string path = CrashDirectory().string();
    return path.c_str();
}

const char* Write(const char* reason, const FaultInfo* fault) {
    PathBuffer().clear();
    if (reason == nullptr) {
        reason = "unknown";
    }
    std::error_code ec;
    std::filesystem::create_directories(CrashDirectory(), ec);
    if (ec) {
        return PathBuffer().c_str();
    }
    const auto now = std::chrono::system_clock::now();
    const auto t   = std::chrono::system_clock::to_time_t(now);
    std::tm    tm {};
#if KYTY_PLATFORM == KYTY_PLATFORM_WINDOWS
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char name[64];
    std::snprintf(name, sizeof(name), "crash-%04d%02d%02d-%02d%02d%02d.log",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
    const auto path = CrashDirectory() / name;
    FILE* f = nullptr;
#if KYTY_PLATFORM == KYTY_PLATFORM_WINDOWS
    _wfopen_s(&f, path.c_str(), L"w");
#else
    f = std::fopen(path.c_str(), "w");
#endif
    if (f == nullptr) {
        return PathBuffer().c_str();
    }
    std::fprintf(f, "kyty_crash_report_version=1\n");
    std::fprintf(f, "time=%04d-%02d-%02d %02d:%02d:%02d\n",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec);
    std::fprintf(f, "reason=%s\n", reason);
    WriteFault(f, fault);
    std::fprintf(f, "\n[counters]\n");
    WriteCounters(f);
    std::fprintf(f, "\n[memory]\n");
    WriteMemory(f);
    std::fflush(f);
    std::fclose(f);
    PathBuffer() = path.string();
    return PathBuffer().c_str();
}

} // namespace Loader::CrashReport