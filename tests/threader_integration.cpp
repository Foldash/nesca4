#include "Threader.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <stdexcept>

std::atomic<int> gThreads{2};
static std::atomic<int> calls{0};
static std::mutex mutex;
static std::condition_variable changed;
static bool releaseTask = false;
static std::promise<void> entered;

static void task(std::string value) {
    if (value == "error") throw std::runtime_error("synthetic task error");
    if (value == "block") {
        entered.set_value();
        std::unique_lock<std::mutex> lock(mutex);
        changed.wait(lock, [] { return releaseTask; });
    }
    ++calls;
}

static void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    try {
        require(!Threader::fireThread("unit", task), "Adapter started implicitly");
        require(!Threader::fireThread("unit", nullptr), "Null callback accepted");
        Threader::start();
        for (int i = 0; i < 100; ++i) require(Threader::fireThread("unit", task), "Submission rejected");
        Threader::finish();
        require(calls == 100 && Threader::stats().completed == 100, "Adapter dropped work while draining");
        Threader::start();
        Threader::fireThread("error", task);
        Threader::fireThread("unit", task);
        Threader::finish();
        require(calls == 101 && Threader::stats().failed == 1, "Adapter did not isolate a task error");

        gThreads = 1;
        calls = 0;
        auto began = entered.get_future();
        Threader::start();
        Threader::fireThread("block", task);
        const bool started = began.wait_for(std::chrono::seconds(2)) == std::future_status::ready;
        if (started) Threader::fireThread("discard", task);
        Threader::requestStop();
        const auto duringStop = Threader::stats();
        const bool rejected = !Threader::fireThread("late", task);
        { std::lock_guard<std::mutex> lock(mutex); releaseTask = true; }
        changed.notify_all();
        Threader::cleanUp();
        require(started && duringStop.active == 1 && duringStop.queued == 0, "Nonblocking cancellation failed");
        require(rejected && calls == 1 && Threader::stats().discarded == 1, "Queued work ran after cancellation");

        Threader::start();
        Threader::fireThread("unit", task);
        Threader::finish();
        require(Threader::stats().completed == 1 && Threader::stats().failed == 0, "New session inherited old state");
        gThreads = 0;
        bool invalid = false;
        try { Threader::start(); } catch (const std::invalid_argument &) { invalid = true; }
        require(invalid, "Invalid thread count accepted");
        std::cout << "Threader adapter: explicit start, drain, cancellation, errors and restart OK\n";
    } catch (const std::exception &error) {
        { std::lock_guard<std::mutex> lock(mutex); releaseTask = true; }
        changed.notify_all();
        Threader::cleanUp();
        std::cerr << error.what() << '\n';
        return 1;
    }
}
