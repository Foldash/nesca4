#include <cstring>
#include <locale>
#include <stdexcept>
#include <string>
#include <iostream>
#include "Utils.h"

static void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}

static void check(const std::string &name) {
    const QString missing("No definition found!");
    if (name == "normal") {
        require(Utils::GetNSErrorDefinition("{\"notify\":\"Invalid access key\"}", "notify") == "Invalid access key", "normal value");
        require(Utils::GetNSErrorDefinition("{\"notify\":\"\"}", "notify").isEmpty(), "empty value");
        require(Utils::GetNSErrorDefinition("{}", "notify") == missing, "missing key");
    } else if (name == "malformed") {
        require(Utils::GetNSErrorDefinition("notify:", "notify") == missing, "missing opening quote");
        require(Utils::GetNSErrorDefinition("notify:\"unfinished", "notify") == missing, "missing closing quote");
    } else if (name == "tail") {
        require(Utils::GetNSErrorDefinition("notify", "notify") == missing, "key at end");
    } else if (name == "null") {
        require(Utils::GetNSErrorDefinition(nullptr, "notify") == missing, "null input");
        require(Utils::GetNSErrorDefinition("{}", nullptr) == missing, "null key");
        require(Utils::GetNSErrorDefinition("{}", "") == missing, "empty key");
    } else if (name == "long") {
        for (std::size_t n : {127u, 128u, 129u, 4096u}) {
            const std::string text = "{\"notify\":\"" + std::string(n, 'x') + "\"}";
            require(Utils::GetNSErrorDefinition(text.c_str(), "notify") == QString(static_cast<int>(std::min(n, std::size_t{128})), QChar('x')), "bounded long value");
        }
    } else if (name == "locale") {
        my_equal<char> equal(std::locale::classic());
        require(equal('A', 'a'), "locale lifetime");
        require(!equal('A', 'b'), "different characters");
        const std::string text("prefix VALUE suffix");
        require(Utils::ustrstr(text, "value") == 7, "substring position");
        require(Utils::ustrstr(text, "absent") == -1, "absent substring");
        for (int i = 0; i < 256; ++i) {
            const char c = static_cast<char>(i);
            require(equal(c, c), "all byte values");
        }
    } else if (name == "truncated") {
        const std::string text("HTTP/1.1 400 Bad Request\r\n\r\n{\"notify\":\"example\"}");
        for (std::size_t n = 0; n <= text.size(); ++n)
            Utils::GetNSErrorDefinition(text.substr(0, n).c_str(), "notify");
    } else {
        throw std::runtime_error("unknown test");
    }
}

int main(int argc, char **argv) {
    try {
        if (argc == 2) check(argv[1]);
        else for (const auto *name : {"normal", "malformed", "tail", "null", "long", "locale", "truncated"}) check(name);
        std::cout << "OK\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
