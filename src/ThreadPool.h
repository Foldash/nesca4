#ifndef NESCA4_THREAD_POOL_H
#define NESCA4_THREAD_POOL_H

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace nesca {

// Lifecycle operations belong to the owner, never to one of this pool's tasks.
// Cancellation drops queued work; the owner must arrange cooperative cancellation
// of a running task before stop() can join it.
class ThreadPool {
public:
    using Task = std::function<void()>;
    enum class State { stopped, running, stopping };
    enum class StopMode { drain, discard };
    struct Stats {
        State state;
        std::size_t queued;
        std::size_t active;
        std::size_t completed;
        std::size_t failed;
        std::size_t discarded;
        std::size_t capacity;
    };

    ThreadPool() = default;
    ~ThreadPool();
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    void start(std::size_t workers, std::size_t queueCapacity);
    bool submit(Task task);
    void waitIdle();
    void requestStop(StopMode mode = StopMode::discard);
    void stop(StopMode mode = StopMode::discard);
    Stats stats() const;
    std::exception_ptr firstError() const;

private:
    void workerLoop();
    void requireOwner() const;

    // Serialize start/join, while workers and producers use mutex_ only.
    std::mutex lifecycleMutex_;
    mutable std::mutex mutex_;
    std::condition_variable workAvailable_;
    std::condition_variable spaceAvailable_;
    std::condition_variable idle_;
    std::queue<Task> tasks_;
    std::vector<std::thread> workers_;
    State state_ = State::stopped;
    std::size_t capacity_ = 0;
    std::size_t active_ = 0;
    std::size_t completed_ = 0;
    std::size_t failed_ = 0;
    std::size_t discarded_ = 0;
    std::uint64_t generation_ = 0;
    std::exception_ptr error_;
    inline static thread_local const ThreadPool *workerPool_ = nullptr;
};
}
#endif
