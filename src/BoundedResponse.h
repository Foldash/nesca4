#ifndef BOUNDED_RESPONSE_H
#define BOUNDED_RESPONSE_H

#include <cstddef>
#include <limits>
#include <string>

namespace nesca {
constexpr std::size_t responseLimit = 180000;

// A failed write returns zero to libcurl. Exceptions must not cross its C callback.
inline std::size_t appendResponse(void *contents, std::size_t size,
                                  std::size_t count, void *destination) noexcept {
    if (destination == nullptr || size == 0 || count == 0) return 0;
    if (count > std::numeric_limits<std::size_t>::max() / size) return 0;
    const std::size_t bytes = size * count;
    auto &buffer = *static_cast<std::string *>(destination);
    if (contents == nullptr || buffer.size() > responseLimit ||
        bytes > responseLimit - buffer.size()) return 0;
    try {
        buffer.append(static_cast<const char *>(contents), bytes);
        return bytes;
    } catch (...) {
        return 0;
    }
}
}
#endif
