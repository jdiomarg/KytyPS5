#ifndef KYTY_LOADER_CRASH_REPORT_H_
#define KYTY_LOADER_CRASH_REPORT_H_

#include <cstdint>

namespace Loader::CrashReport {

// Directory where crash reports are written:
//   %LOCALAPPDATA%\Kyty\crashes (Windows)
//   $XDG_DATA_HOME/Kyty/crashes or ~/.local/share/Kyty/crashes (other)
[[nodiscard]] const char* Directory();

// All the guest-fault context the runtime already extracts. Filled by the
// caller from Common::HostException::ExceptionInfo, kept as plain scalars so
// this module stays platform-agnostic.
struct FaultInfo {
    int      exception_type         = -1;
    uint64_t exception_address      = 0;
    int      access_violation_type  = -1;
    uint64_t access_violation_vaddr = 0;
    uint64_t rax = 0, rbx = 0, rcx = 0, rdx = 0;
    uint64_t rsi = 0, rdi = 0, rbp = 0, rsp = 0;
    uint64_t r8  = 0, r9  = 0, r10 = 0, r11 = 0;
    uint64_t r12 = 0, r13 = 0, r14 = 0, r15 = 0;
};

// Write a crash report. `fault` may be null when the report is being written
// for a non-fault reason. Returns the full path written, or empty on failure.
// Pointer is valid until the next call to Write() on the same thread.
[[nodiscard]] const char* Write(const char* reason, const FaultInfo* fault);

} // namespace Loader::CrashReport

#endif