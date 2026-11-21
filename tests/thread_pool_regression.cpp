#include "ThreadPool.h"
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace std::chrono_literals;
using Pool = nesca::ThreadPool;

static void require(bool value, const char *message) {
    if (!value) throw std::runtime_error(message);
}
template<class T> static void ready(std::future<T> &future) {
    require(future.wait_for(2s) == std::future_status::ready, "Operation did not complete");
}
class Gate {
public:
    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&] { return open_; });
    }
    void open() {
        { std::lock_guard<std::mutex> lock(mutex_); open_ = true; }
        cv_.notify_all();
    }
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool open_ = false;
};

static void drainsAndRestarts() {
    Pool pool;
    require(!pool.submit([] {}), "Stopped pool accepted work");
    for (int session = 0; session < 25; ++session) {
        std::atomic<int> done{0};
        pool.start(3, 4);
        for (int i = 0; i < 100; ++i) {
            require(pool.submit([&] { ++done; }), "Running pool rejected work");
            require(pool.stats().queued <= 4, "Queue capacity exceeded");
        }
        pool.stop(Pool::StopMode::drain);
        const auto stats = pool.stats();
        require(done == 100 && stats.completed == 100, "Drain lost queued work");
        require(stats.active == 0 && stats.queued == 0, "Drain left unfinished work");
        require(!pool.submit([] {}), "Work accepted after stop");
        pool.stop();
    }
}

static void cancelWakesBlockedProducer() {
    Pool pool;
    Gate active;
    pool.start(1, 1);
    std::promise<void> entered;
    auto began = entered.get_future();
    pool.submit([&] { entered.set_value(); active.wait(); });
    ready(began);
    std::atomic<int> queuedTaskCalls{0};
    pool.submit([&] { ++queuedTaskCalls; });
    auto producer = std::async(std::launch::async, [&] {
        return pool.submit([&] { ++queuedTaskCalls; });
    });
    const bool blocked = producer.wait_for(20ms) == std::future_status::timeout;
    pool.requestStop();
    const bool woke = producer.wait_for(2s) == std::future_status::ready;
    active.open();
    pool.stop();
    require(blocked && woke, "Cancellation failed to wake a backpressured producer");
    require(!producer.get(), "Cancelled producer accepted work");
    require(queuedTaskCalls == 0, "Cancelled queued task ran");
    require(pool.stats().discarded == 1, "Discard accounting incorrect");
}

static void idleIncludesActiveWork() {
    Pool pool;
    Gate active;
    std::promise<void> entered;
    auto began = entered.get_future();
    pool.start(1, 1);
    pool.submit([&] { entered.set_value(); active.wait(); });
    ready(began);
    auto waiter = std::async(std::launch::async, [&] { pool.waitIdle(); });
    const bool waited = waiter.wait_for(20ms) == std::future_status::timeout;
    active.open();
    ready(waiter);
    pool.stop(Pool::StopMode::drain);
    require(waited, "Idle reported while a task was still active");
}

static void exceptionsDoNotKillWorkers() {
    Pool pool;
    pool.start(1, 2);
    pool.submit([] { throw std::runtime_error("task failure"); });
    pool.submit([] {});
    pool.waitIdle();
    require(pool.stats().failed == 1 && pool.stats().completed == 1, "Task exception accounting failed");
    require(bool(pool.firstError()), "Task error was not retained");
    bool expected = false;
    try { std::rethrow_exception(pool.firstError()); }
    catch (const std::runtime_error &) { expected = true; }
    require(expected, "Task error type was lost");
    pool.submit([] {});
    pool.stop(Pool::StopMode::drain);
    require(pool.stats().completed == 2, "Worker did not survive task exception");
}

static void concurrentStopAndOwnerChecks() {
    Pool pool;
    pool.start(2, 2);
    pool.submit([&] { pool.stop(); });
    pool.submit([&] { pool.submit([] {}); });
    pool.waitIdle();
    require(pool.stats().failed == 2, "Blocking calls from a worker were not rejected");
    auto first = std::async(std::launch::async, [&] { pool.stop(); });
    auto second = std::async(std::launch::async, [&] { pool.stop(); });
    ready(first);
    ready(second);
    first.get(); second.get();
    require(pool.stats().state == Pool::State::stopped, "Concurrent stop left pool running");
    pool.start(1, 1);
    require(!pool.firstError(), "New session retained an old error");
    pool.stop();
}

static void destructorJoinsRunningTask() {
    Gate active;
    auto pool = std::make_unique<Pool>();
    pool->start(1, 1);
    std::promise<void> entered;
    auto began = entered.get_future();
    pool->submit([&] { entered.set_value(); active.wait(); });
    ready(began);
    auto destroy = std::async(std::launch::async, [&] { pool.reset(); });
    const bool joined = destroy.wait_for(20ms) == std::future_status::timeout;
    active.open();
    ready(destroy);
    require(joined, "Destructor failed to join an active worker");
}

static void invalidConfiguration() {
    Pool pool;
    bool rejected = false;
    try { pool.start(0, 1); } catch (const std::invalid_argument &) { rejected = true; }
    require(rejected, "Zero workers accepted");
    rejected = false;
    try { pool.start(1, 0); } catch (const std::invalid_argument &) { rejected = true; }
    require(rejected, "Zero queue capacity accepted");
    pool.start(1, 1);
    rejected = false;
    try { pool.start(1, 1); } catch (const std::logic_error &) { rejected = true; }
    require(rejected, "Live pool started twice");
    pool.stop();
}

int main() {
    try {
        drainsAndRestarts();
        cancelWakesBlockedProducer();
        idleIncludesActiveWork();
        exceptionsDoNotKillWorkers();
        concurrentStopAndOwnerChecks();
        destructorJoinsRunningTask();
        invalidConfiguration();
        std::cout << "Thread pool: lifecycle, bounds, cancellation, exceptions and restart OK\n";
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
