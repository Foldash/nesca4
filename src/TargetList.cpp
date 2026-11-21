#include "TargetList.h"
#include <limits>
#include <string>

namespace targets {
namespace {
bool space(char c) { return c == ' ' || c == '\t' || c == '\r'; }
std::string_view trim(std::string_view s) {
    while (!s.empty() && space(s.front())) s.remove_prefix(1);
    while (!s.empty() && space(s.back())) s.remove_suffix(1);
    return s;
}
unsigned number(std::string_view s, unsigned maximum) {
    if (s.empty()) throw Error(0, "missing number");
    unsigned value = 0;
    for (char c : s) {
        if (c < '0' || c > '9' || value > maximum / 10 ||
            (value == maximum / 10 && unsigned(c - '0') > maximum % 10))
            throw Error(0, "invalid number or number out of range");
        value = value * 10 + unsigned(c - '0');
    }
    return value;
}
std::uint32_t ipv4(std::string_view s) {
    s = trim(s);
    std::uint32_t address = 0;
    for (int i = 0; i < 4; ++i) {
        const auto dot = s.find('.');
        if ((i < 3) != (dot != std::string_view::npos))
            throw Error(0, "IPv4 requires four octets");
        address = (address << 8) | number(s.substr(0, dot), 255);
        if (i < 3) s.remove_prefix(dot + 1);
    }
    return address;
}
}
Error::Error(std::size_t lineNumber, const char* reason)
    : std::runtime_error(reason), line(lineNumber) {}

std::array<unsigned int, 4> octets(std::uint32_t address) noexcept {
    return {{address >> 24, (address >> 16) & 255, (address >> 8) & 255, address & 255}};
}
Range parseRange(std::string_view text) {
    text = trim(text);
    const auto dash = text.find('-'), slash = text.find('/');
    if (dash != std::string_view::npos && slash != std::string_view::npos)
        throw Error(0, "mixed range and CIDR syntax");
    if (dash != std::string_view::npos) {
        const auto first = ipv4(text.substr(0, dash));
        const auto last = ipv4(text.substr(dash + 1));
        if (first > last) throw Error(0, "range end precedes start");
        return {first, last};
    }
    if (slash != std::string_view::npos) {
        const auto address = ipv4(text.substr(0, slash));
        const auto prefix = number(trim(text.substr(slash + 1)), 32);
        const std::uint32_t mask = prefix == 0 ? 0 : (UINT32_MAX << (32 - prefix));
        return {address & mask, (address & mask) | ~mask};
    }
    const auto address = ipv4(text);
    return {address, address};
}

List read(std::istream& input, Limits limits, const std::function<bool()>& cancelled) {
    List result;
    std::string line;
    std::size_t bytes = 0, lineNumber = 1;
    const auto consume = [&] {
        std::string_view text(line);
        if (lineNumber == 1 && text.substr(0, 3) == "\xef\xbb\xbf") text.remove_prefix(3);
        text = trim(text);
        // Full-line comments only. A typo before a comment must not be hidden.
        if (text.empty() || text.front() == '#' || text.substr(0, 2) == "//") return;
        if (result.ranges.size() >= limits.entries) throw Error(lineNumber, "too many entries");
        Range range{};
        try { range = parseRange(text); }
        catch (const Error& e) { throw Error(lineNumber, e.what()); }
        if (range.size() > std::numeric_limits<std::uint64_t>::max() - result.addresses)
            throw Error(lineNumber, "address count overflow");
        result.ranges.push_back(range);
        result.addresses += range.size();
    };
    for (;;) {
        if (cancelled && cancelled()) throw Error(lineNumber, "import cancelled");
        int c;
        try { c = input.get(); }
        catch (const std::ios_base::failure&) {
            if (!input.eof() || input.bad()) throw Error(lineNumber, "input read failed");
            break;
        }
        if (c == std::char_traits<char>::eof()) {
            if (!input.eof() || input.bad()) throw Error(lineNumber, "input read failed");
            break;
        }
        if (bytes == limits.bytes) throw Error(lineNumber, "file byte limit exceeded");
        ++bytes;
        if (c == '\n') {
            consume();
            line.clear();
            ++lineNumber;
        } else {
            if (line.size() == limits.lineBytes) throw Error(lineNumber, "line byte limit exceeded");
            line.push_back(static_cast<char>(c));
        }
    }
    if (!line.empty()) consume();
    if (result.ranges.empty()) throw Error(lineNumber, "empty address list");
    return result;
}
}
