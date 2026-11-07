#include "../../alvr_server/bindings.h"

#include "../../alvr_server/Logger.h"
#include "../../shared/backward.hpp"
#include <Windows.h>
#include <ostream>

static bool IsVrServerProcess() {
    char path[MAX_PATH];
    if (GetModuleFileNameA(nullptr, path, MAX_PATH) == 0) {
        return false;
    }

    const char* filename = path;
    for (const char* cursor = path; *cursor != '\0'; ++cursor) {
        if (*cursor == '\\' || *cursor == '/') {
            filename = cursor + 1;
        }
    }

    return lstrcmpiA(filename, "vrserver.exe") == 0;
}

static LONG WINAPI handler(PEXCEPTION_POINTERS ptrs) {
    backward::StackTrace stacktrace;
    backward::Printer printer;
    std::ostringstream stream;

    stacktrace.load_from(ptrs->ExceptionRecord->ExceptionAddress);
    printer.print(stacktrace, stream);
    std::string str = stream.str();
    Error("Unhandled exception: %X\n%s", ptrs->ExceptionRecord->ExceptionCode, str.c_str());

    Sleep(2000);

    return EXCEPTION_EXECUTE_HANDLER;
}

void HookCrashHandler() {
    // Steam can briefly load the external driver while probing it. Installing
    // this process-wide handler there outlives the DLL unload and crashes Steam.
    if (IsVrServerProcess()) {
        SetUnhandledExceptionFilter(handler);
    }
}
