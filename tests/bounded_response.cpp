#include "BoundedResponse.h"
#include <stdexcept>
#include <iostream>
#include <string>

static void require(bool condition) {
    if (!condition) throw std::runtime_error("Bounded response regression failed");
}

int main() {
    std::string buffer;
    std::string data(nesca::responseLimit + 1, 'x');
    require(nesca::appendResponse(data.data(), 1, 3, &buffer) == 3);
    require(buffer == "xxx");
    require(nesca::appendResponse(data.data(), 1, nesca::responseLimit - 3, &buffer) == nesca::responseLimit - 3);
    require(buffer.size() == nesca::responseLimit);
    require(nesca::appendResponse(data.data(), 1, 1, &buffer) == 0);
    require(buffer.size() == nesca::responseLimit);
    buffer.clear();
    require(nesca::appendResponse(data.data(), 1, data.size(), &buffer) == 0);
    require(buffer.empty());
    require(nesca::appendResponse(data.data(), 2, std::numeric_limits<std::size_t>::max(), &buffer) == 0);
    require(buffer.empty());
    require(nesca::appendResponse(nullptr, 1, 1, &buffer) == 0);
    require(nesca::appendResponse(data.data(), 1, 1, nullptr) == 0);
    require(nesca::appendResponse(nullptr, 0, 20, &buffer) == 0);
    buffer.assign(nesca::responseLimit + 1, 'y');
    require(nesca::appendResponse(data.data(), 1, 1, &buffer) == 0);
    require(buffer == std::string(nesca::responseLimit + 1, 'y'));
    std::cout << "OK\n";
}
