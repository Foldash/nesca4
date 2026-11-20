#include "BoundedResponse.h"
#include <cassert>
#include <iostream>
#include <string>

int main() {
    std::string buffer;
    std::string data(nesca::responseLimit + 1, 'x');
    assert(nesca::appendResponse(data.data(), 1, 3, &buffer) == 3);
    assert(buffer == "xxx");
    assert(nesca::appendResponse(data.data(), 1, nesca::responseLimit - 3, &buffer) == nesca::responseLimit - 3);
    assert(buffer.size() == nesca::responseLimit);
    assert(nesca::appendResponse(data.data(), 1, 1, &buffer) == 0);
    assert(buffer.size() == nesca::responseLimit);
    buffer.clear();
    assert(nesca::appendResponse(data.data(), 1, data.size(), &buffer) == 0);
    assert(buffer.empty());
    assert(nesca::appendResponse(data.data(), 2, std::numeric_limits<std::size_t>::max(), &buffer) == 0);
    assert(buffer.empty());
    assert(nesca::appendResponse(nullptr, 1, 1, &buffer) == 0);
    assert(nesca::appendResponse(data.data(), 1, 1, nullptr) == 0);
    assert(nesca::appendResponse(nullptr, 0, 20, &buffer) == 0);
    buffer.assign(nesca::responseLimit + 1, 'y');
    assert(nesca::appendResponse(data.data(), 1, 1, &buffer) == 0);
    assert(buffer == std::string(nesca::responseLimit + 1, 'y'));
    std::cout << "OK\n";
}
