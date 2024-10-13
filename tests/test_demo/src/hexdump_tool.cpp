#include <gtest/gtest.h>
#include "hexdump_tool.h"
#include <random>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <cctype>
#include <set>

TEST(HexDumpTest, simple_str) {
    std::string s = "Hello, world!\n";  // hexdump -C file
    // std::random_device rd;
    std::mt19937 gen(3);
    std::uniform_int_distribution<uint8_t> dis;
    for (int i = 0; i < 100; i++) {
        s.push_back(dis(gen));
    }
    hex_dump(s);

    hex_dump(s, 8);
}

TEST(HexDumpTest, simple_vector_uint8_t) {
    std::vector<uint8_t> s;
    // std::random_device rd;
    std::mt19937 gen(3);
    std::uniform_int_distribution<uint8_t> dis;
    for (int i = 0; i < 100; i++) {
        s.push_back(dis(gen));
    }
    hex_dump(s);
}

TEST(HexDumpTest, simple_vector_uint16_t) {
    std::vector<uint16_t> s;
    // std::random_device rd;
    std::mt19937 gen(3);
    std::uniform_int_distribution<uint16_t> dis(-0x8000, 0x7fff);
    for (int i = 0; i < 100; i++) {
        s.push_back(dis(gen));
    }
    hex_dump(s);
}

TEST(HexDumpTest, simple_set_uint16_t) {
    std::set<uint16_t> s;
    // std::random_device rd;
    std::mt19937 gen(3);
    std::uniform_int_distribution<uint16_t> dis(-0x8000, 0x7fff);
    for (int i = 0; i < 100; i++) {
        s.insert(dis(gen));
    }
    hex_dump(s);
}

TEST(HexDumpTest, simple_set_uint8_t) {
    std::set<uint8_t> s;
    // std::random_device rd;
    std::mt19937 gen(3);
    std::uniform_int_distribution<uint8_t> dis(-0x80, 0x7f);
    for (int i = 0; i < 100; i++) {
        s.insert(dis(gen));
    }
    hex_dump(s);
}

TEST(HexDumpTest, streamTest) {
    const char* filename = "a.txt";
    std::ofstream file(filename);
    file << "hello world!\n";
    file.close();

    std::ifstream ifs("a.txt");
    if (!ifs.good()) {
        std::cerr << std::strerror(errno) << std::endl;
    }
    hex_dump(IstreamRange(ifs));
    remove(filename);
}

TEST(HexDumpTest, streamTest2) {
    std::string path = "../.gitignore";
    std::ifstream ifs(path);
    if (!ifs.good()) {
        std::cerr << std::strerror(errno) << ": " << path << std::endl;
    }
    hex_dump(IstreamRange(ifs));

    // hex_dump(IstreamRange(std::cin));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    return 0;
}