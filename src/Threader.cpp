#include "Threader.h"
#include "externData.h"
#include <algorithm>
#include <utility>
#include <stdexcept>

int Threader::gThreadDelay = 10;
nesca::ThreadPool Threader::pool_;

void Threader::start() {
    const int configured = gThreads.load();
    if (configured <= 0) throw std::invalid_argument("Thread count must be positive");
    const auto count = static_cast<std::size_t>(configured);
    pool_.start(count, count);
}

bool Threader::fireThread(std::string ip, void (*func)(std::string)) {
    if (func == nullptr) return false;
    return pool_.submit([ip = std::move(ip), func] { func(ip); });
}

void Threader::requestStop() { pool_.requestStop(); }
void Threader::finish() { pool_.stop(nesca::ThreadPool::StopMode::drain); }
void Threader::cleanUp() { pool_.stop(); }
nesca::ThreadPool::Stats Threader::stats() { return pool_.stats(); }
