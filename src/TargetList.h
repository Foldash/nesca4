#ifndef NESCA_TARGET_LIST_H
#define NESCA_TARGET_LIST_H

#include <array>
#include <cstdint>
#include <functional>
#include <istream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace targets {
struct Range {
    std::uint32_t first, last;
    std::uint64_t size() const noexcept { return std::uint64_t(last) - first + 1; }
};
struct Limits {
    std::size_t bytes = 16 * 1024 * 1024;
    std::size_t lineBytes = 4096;
    std::size_t entries = 100000;
};
class Error : public std::runtime_error {
public:
    const std::size_t line;
    Error(std::size_t lineNumber, const char* reason);
};
struct List {
    std::vector<Range> ranges;
    std::uint64_t addresses = 0; // Includes overlaps, matching the input order.
};
std::array<unsigned int, 4> octets(std::uint32_t address) noexcept;
Range parseRange(std::string_view text);
// Throws Error on invalid input, I/O failure, limits or cancellation. A partial
// list is never returned. Does not expand ranges or perform network operations.
List read(std::istream& input, Limits limits = {},
          const std::function<bool()>& cancelled = {});
}
#endif
