#include "ThreadPool.h"
#include <stdexcept>
#include <utility>

namespace nesca {
ThreadPool::~ThreadPool() { stop(); }

void ThreadPool::requireOwner() const {
    if (workerPool_ == this)
        throw std::logic_error("A pool task cannot start, join, or wait for its own pool");
}

void ThreadPool::start(std::size_t workers, std::size_t queueCapacity) {
    requireOwner();
    if (workers == 0 || queueCapacity == 0)
        throw std::invalid_argument("Worker count and queue capacity must be positive");
    std::lock_guard<std::mutex> lifecycle(lifecycleMutex_);
    std::unique_lock<std::mutex> lock(mutex_);
    if (state_ != State::stopped) throw std::logic_error("Pool is already started");
    capacity_ = queueCapacity;
    active_ = completed_ = failed_ = discarded_ = 0;
    error_ = nullptr;
    ++generation_;
    state_ = State::running;
    try {
        workers_.reserve(workers);
        for (std::size_t i = 0; i < workers; ++i)
            workers_.emplace_back(&ThreadPool::workerLoop, this);
    } catch (...) {
        state_ = State::stopping;
        lock.unlock();
        workAvailable_.notify_all();
        spaceAvailable_.notify_all();
        for (auto &worker : workers_) if (worker.joinable()) worker.join();
        workers_.clear();
        lock.lock();
        state_ = State::stopped;
        throw;
    }
}

bool ThreadPool::submit(Task task) {
    requireOwner();
    if (!task) throw std::invalid_argument("Task must not be empty");
    std::unique_lock<std::mutex> lock(mutex_);
    const auto generation = generation_;
    spaceAvailable_.wait(lock, [&] {
        return generation_ != generation || state_ != State::running || tasks_.size() < capacity_;
    });
    // A producer woken by cancellation must not leak work into a restarted pool.
    if (generation_ != generation || state_ != State::running) return false;
    tasks_.push(std::move(task));
    lock.unlock();
    workAvailable_.notify_one();
    return true;
}

void ThreadPool::waitIdle() {
    requireOwner();
    std::unique_lock<std::mutex> lock(mutex_);
    idle_.wait(lock, [&] { return tasks_.empty() && active_ == 0; });
}

void ThreadPool::requestStop(StopMode mode) {
    std::queue<Task> discarded;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ == State::stopped) return;
        state_ = State::stopping;
        if (mode == StopMode::discard) {
            discarded_ += tasks_.size();
            tasks_.swap(discarded);
        }
    }
    workAvailable_.notify_all();
    spaceAvailable_.notify_all();
    idle_.notify_all();
    // Destroy discarded captures outside the queue mutex.
}

void ThreadPool::stop(StopMode mode) {
    requireOwner();
    std::lock_guard<std::mutex> lifecycle(lifecycleMutex_);
    requestStop(mode);
    for (auto &worker : workers_) if (worker.joinable()) worker.join();
    workers_.clear();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        state_ = State::stopped;
    }
    spaceAvailable_.notify_all();
    idle_.notify_all();
}

ThreadPool::Stats ThreadPool::stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return {state_, tasks_.size(), active_, completed_, failed_, discarded_, capacity_};
}

std::exception_ptr ThreadPool::firstError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return error_;
}

void ThreadPool::workerLoop() {
    workerPool_ = this;
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            workAvailable_.wait(lock, [&] { return state_ != State::running || !tasks_.empty(); });
            if (tasks_.empty() && state_ != State::running) break;
            task = std::move(tasks_.front());
            tasks_.pop();
            ++active_;
        }
        spaceAvailable_.notify_one();
        std::exception_ptr error;
        try { task(); }
        catch (...) { error = std::current_exception(); }
        task = nullptr;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            --active_;
            if (error) {
                ++failed_;
                if (!error_) error_ = error;
            } else {
                ++completed_;
            }
        }
        idle_.notify_all();
    }
    workerPool_ = nullptr;
}
}
