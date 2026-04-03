#include "perf_duration_trace.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

[[noreturn]] void fail(const std::string& message) {
    throw std::runtime_error(message);
}

void expect(bool cond, const std::string& message) {
    if (!cond) {
        fail(message);
    }
}

void test_analyzer_smoke() {
    perf_duration_trace::Runtime::instance().reset_for_tests({4U, 128U});
    const std::string bin_path = "/tmp/perf_duration_trace_smoke.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_smoke.sites.tsv";
    const std::string json_path = "/tmp/perf_duration_trace_smoke.json";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());
    std::remove(json_path.c_str());

    {
        PERF_SCOPE("analyze_smoke_case");
    }

    const auto stats = perf_duration_trace::Runtime::instance().finalize(
        {bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "finalize should succeed before analyzer smoke");

    const std::string command =
        "python3 projects/perf_duration_trace/tools/perf_duration_analyze.py " + bin_path + " " + site_path +
        " --json > " + json_path;
    const int exit_code = std::system(command.c_str());
    expect(exit_code == 0, "analyzer command should succeed");

    std::ifstream in(json_path);
    expect(in.good(), "analyzer should write json output");
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    expect(content.find("\"record_count\"") != std::string::npos,
           "analyzer json output should contain record_count");
}

}  // namespace

int main() {
    try {
        test_analyzer_smoke();
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "perf_duration_trace_analyze_smoke failed: %s\n", ex.what());
        return 1;
    }

    std::puts("perf_duration_trace_analyze_smoke passed");
    return 0;
}
