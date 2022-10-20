#include "pch.h"
#include "thread_pool.h"

#include <cassert>

namespace cb {

    // static
    unsigned int ThreadPool::GetNumLogicalCores() {
        // TODO(cbraley): Apparently this is broken in some older stdlib
        // implementations?
        const unsigned int dflt = std::thread::hardware_concurrency();
        if (dflt == 0) {
            // TODO(cbraley): Return some error code instead.
            return 16;
        }
        else {
            return dflt;
        }
    }

    ThreadPool::~ThreadPool() {
        // TODO(cbraley): The current thread could help out to drain the work_ queue
        // faster - for example, if there is work that hasn't yet been scheduled this
        // thread could "pitch in" to help finish faster.

        {
            std::lock_guard<std::mutex> scoped_lock(mu_);
            exit_ = true;
        }
        condvar_.notify_all();  // Tell *all* workers we are ready.

        for (std::thread& thread : workers_) {
            thread.join();
        }
    }

    void ThreadPool::Wait() {
        std::unique_lock<std::mutex> lock(mu_);
        if (!work_.empty()) {
            work_done_condvar_.wait(lock, [this] { return work_.empty(); });
        }
    }

    ThreadPool::ThreadPool(int num_workers)
        : num_workers_(num_workers), exit_(false) {
        assert(num_workers_ > 0);
        // TODO(cbraley): Handle thread construction exceptions.
        workers_.reserve(num_workers_);
        for (int i = 0; i < num_workers_; ++i) {
            workers_.emplace_back(&ThreadPool::ThreadLoop, this);
        }
    }

    void ThreadPool::Schedule(std::function<void(void)> func) {
        ScheduleAndGetFuture(std::move(func));  // We ignore the returned std::future.
    }

    void ThreadPool::ThreadLoop() {
        // Wait until the ThreadPool sends us work.
        while (true) {
            WorkItem work_item;

            int prev_work_size = -1;
            {
                std::unique_lock<std::mutex> lock(mu_);
                condvar_.wait(lock, [this] { return exit_ || (!work_.empty()); });
                // ...after the wait(), we hold the lock.

                // If all the work is done and exit_ is true, break out of the loop.
                if (exit_ && work_.empty()) {
                    break;
                }

                // Pop the work off of the queue - we are careful to execute the
                // work_item.func callback only after we have released the lock.
                prev_work_size = work_.size();
                work_item = std::move(work_.front());
                work_.pop();
            }

            // We are careful to do the work without the lock held!
            // TODO(cbraley): Handle exceptions properly.
            work_item.func();  // Do work.

            if (work_done_callback_) {
                work_done_callback_(prev_work_size - 1);
            }

            // Notify a condvar is all work is done.
            {
                std::unique_lock<std::mutex> lock(mu_);
                if (work_.empty() && prev_work_size == 1) {
                    work_done_condvar_.notify_all();
                }
            }
        }
    }

    int ThreadPool::OutstandingWorkSize() const {
        std::lock_guard<std::mutex> scoped_lock(mu_);
        return work_.size();
    }

    int ThreadPool::NumWorkers() const { return num_workers_; }

    void ThreadPool::SetWorkDoneCallback(std::function<void(int)> func) {
        work_done_callback_ = std::move(func);
    }

}  // namespace cb