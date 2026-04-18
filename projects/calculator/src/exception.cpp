#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <memory>
#include "exception.h"

namespace {

std::string execCommand(const std::string& cmd) {
    std::array<char, 512> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    // Remove trailing newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    return result;
}

std::string getSourceLocation(void* address, const char* filename, void* baseAddr) {
    if (!filename) return "";

    // Calculate relative address for shared libraries
    uintptr_t relativeAddr = reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(baseAddr);

    std::ostringstream cmd;
    cmd << "addr2line -e " << filename << " -f " << std::hex << "0x" << relativeAddr;

    std::string funcName = execCommand(cmd.str());
    if (funcName.empty() || funcName.find("??") != std::string::npos || funcName == "??") {
        return "";
    }

    // Second line is the file:line
    std::string location = execCommand(cmd.str() + " 2>/dev/null | tail -1");

    // Use -p option for combined output
    cmd.str("");
    cmd << "addr2line -e " << filename << " -p " << std::hex << "0x" << relativeAddr;
    std::string output = execCommand(cmd.str());

    if (output.empty() || output.find("??") != std::string::npos) {
        return "";
    }

    // Output format: "function at /path/to/file.cpp:line (discriminator...)"
    size_t atPos = output.find(" at ");
    if (atPos != std::string::npos) {
        std::string locPart = output.substr(atPos + 4);
        // Remove any trailing discriminator info
        size_t parenPos = locPart.find(" (");
        if (parenPos != std::string::npos) {
            locPart = locPart.substr(0, parenPos);
        }
        return locPart;
    }
    return output;
}

} // namespace

std::string CalcException::captureStackTrace() {
    constexpr int kMaxFrames = 64;
    void* buffer[kMaxFrames];
    int numFrames = backtrace(buffer, kMaxFrames);
    char** symbols = backtrace_symbols(buffer, numFrames);

    std::ostringstream oss;
    // Skip frame 0 (captureStackTrace itself) and frame 1 (CalcException constructor)
    for (int i = 2; i < numFrames; ++i) {
        oss << "#" << (i - 2) << " ";

        Dl_info info;
        if (dladdr(buffer[i], &info) && info.dli_sname) {
            int status = 0;
            char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
            if (status == 0 && demangled) {
                oss << demangled;
                free(demangled);
            } else {
                oss << info.dli_sname;
            }

            std::string location = getSourceLocation(buffer[i], info.dli_fname, info.dli_fbase);
            if (!location.empty()) {
                oss << " at " << location;
            }
        } else {
            oss << symbols[i];
        }
        oss << "\n";
    }
    free(symbols);
    return oss.str();
}

CalcException::CalcException(const std::string& message)
    : message_(message), stackTraceCache_(captureStackTrace()) {}

const char* CalcException::what() const noexcept {
    return message_.c_str();
}

const char* CalcException::stackTrace() const noexcept {
    return stackTraceCache_.c_str();
}

SyntaxError::SyntaxError(const std::string& message) : CalcException(message) {}

RuntimeError::RuntimeError(const std::string& message) : CalcException(message) {}

DivisionByZeroError::DivisionByZeroError() : RuntimeError("Division by zero") {}

UndefinedVariableError::UndefinedVariableError(const std::string& name)
    : RuntimeError("Undefined variable: " + name) {}

UninitializedVariableError::UninitializedVariableError(const std::string& name)
    : RuntimeError("Variable not initialized: " + name) {}

UnknownFunctionError::UnknownFunctionError(const std::string& name)
    : SyntaxError("Unknown function: " + name) {}

InvalidTokenError::InvalidTokenError(char ch)
    : SyntaxError("Invalid character: '" + std::string(1, ch) + "'") {}