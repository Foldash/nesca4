#include "TargetList.h"
#include <iostream>
#include <sstream>
#include <string>

namespace {
int checks = 0;
void require(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
template<class F> void rejects(F action, std::size_t line = 0) {
    try { action(); }
    catch (const targets::Error& error) {
        require(error.line == line, "wrong error line");
        return;
    }
    throw std::runtime_error("invalid input accepted");
}
targets::List read(const std::string& text, targets::Limits limits = {}) {
    std::istringstream input(text);
    return targets::read(input, limits);
}
}
int main() {
    try {
        const auto whole = targets::parseRange("192.0.2.8/0");
        require(whole.first == 0 && whole.last == UINT32_MAX && whole.size() == 4294967296ULL,
                "CIDR /0 overflow");
        const auto single = targets::parseRange("255.255.255.255/32");
        require(single.first == UINT32_MAX && single.size() == 1, "CIDR /32 boundary");
        const auto pair = targets::parseRange("192.0.2.5/31");
        require(pair.first == 0xc0000204 && pair.last == 0xc0000205, "CIDR alignment");
        require(targets::parseRange("0.0.0.0").size() == 1, "zero single address");
        require(targets::parseRange(" 192.0.2.255 - 192.0.3.0\t").size() == 2, "octet carry");
        for (const auto* text : {"", "invalid", "1.2.3", "1.2.3.4.5", "1..3.4", "256.1.2.3",
                "-1.2.3.4", "1.2.3.4x", "1.2.3.4/33", "1.2.3.4/-1", "1.2.3.4/",
                "1.2.3.4/24/2", "1.2.3.4/999999999999999999999999", "1.2.3.4/+1",
                "1.2.3.4-1.2.3.3", "1.2.3.4-", "1.2.3.4-1.2.3.5-1.2.3.6",
                "1.2.3.4/24-1.2.3.6", "1. 2.3.4", "1.2.3.4 # ignored?"})
            rejects([&] { targets::parseRange(text); });

        const auto list = read("\xef\xbb\xbf  # header\r\n\t// comment\r\n\r\n"
                               "192.0.2.0/30\r\n192.0.2.8 - 192.0.2.9\n192.0.2.1");
        require(list.ranges.size() == 3 && list.addresses == 7, "multiple ranges/comments/CRLF/BOM");
        require(list.ranges[0].first == 0xc0000200 && list.ranges[2].first == 0xc0000201,
                "input order and overlapping addresses must be preserved");
        require(read("0.0.0.0/0\n0.0.0.0/0\n").addresses == 8589934592ULL, "64-bit total");
        rejects([] { read("# empty\n\n"); }, 3);
        rejects([] { read("192.0.2.1\n# comment\n192.0.2.256\n"); }, 3);
        rejects([] { read("\xff\xfe" "1.2.3.4"); }, 1);
        rejects([] { read(std::string("1.2.3.4\0", 8)); }, 1);
        rejects([] { read("1.2.3.4\n\xef\xbb\xbf" "1.2.3.5"); }, 2);
        require(read("1.2.3.4", {7, 7, 1}).addresses == 1, "exact byte/line/entry limits");
        rejects([] { read("1.2.3.4\n", {7, 7, 1}); }, 1);
        rejects([] { read("1.2.3.4", {100, 6, 1}); }, 1);
        rejects([] { read("1.2.3.4\n1.2.3.5", {100, 7, 1}); }, 2);
        rejects([] { read(std::string(100000, '0'), {200000, 4096, 10}); }, 1);
        rejects([] { read("1.2.3.4", {0, 0, 0}); }, 1);
        rejects([] { std::istringstream input("1.2.3.4"); input.setstate(std::ios::badbit);
                     targets::read(input); }, 1);
        rejects([] { std::istringstream input("1.2.3.4"); input.setstate(std::ios::failbit);
                     targets::read(input); }, 1);
        std::istringstream throwing("1.2.3.4");
        throwing.exceptions(std::ios::badbit | std::ios::failbit);
        require(targets::read(throwing).addresses == 1, "EOF with stream exceptions");
        rejects([] { std::istringstream input("1.2.3.4\n1.2.3.5"); int calls = 0;
                     targets::read(input, {}, [&] { return ++calls > 9; }); }, 2);
        // Assignment cannot replace an existing validated plan on an invalid file.
        auto previous = read("192.0.2.1");
        rejects([&] { previous = read("192.0.2.2\nwrong"); }, 2);
        require(previous.ranges.size() == 1 && previous.ranges[0].first == 0xc0000201,
                "failed import replaced existing plan");
        std::uint32_t state = 123;
        for (int i = 0; i < 10000; ++i) {
            state = state * 1664525U + 1013904223U;
            const auto parts = targets::octets(state);
            const auto text = std::to_string(parts[0]) + "." + std::to_string(parts[1]) + "." +
                std::to_string(parts[2]) + "." + std::to_string(parts[3]);
            require(targets::parseRange(text).first == state, "address round trip");
        }
        std::string many;
        for (int i = 0; i < 100000; ++i) many += "192.0.2.0/24\n";
        require(read(many).ranges.size() == 100000, "default entry limit boundary");
        rejects([&] { read(many + "192.0.2.1"); }, 100001);
        std::cout << "Target list: " << checks << " checks passed, no network operations\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
