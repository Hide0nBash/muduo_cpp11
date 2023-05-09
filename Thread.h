#pragma once

#include <thread>
#include "noncopyable.h"
#include <atomic>
#include <functional>
#include <memory>
#include <unistd.h>
#include <string>

class Thread : noncopyable {
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() { return started_; }
    const std::string& name() { return name_; }

    pid_t tid() { return tid_; }
    static int numCreated() { return numCreated_; }

private:
    void setDefaultName();

    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    std::string name_;
    ThreadFunc func_;
    bool started_;
    bool joined_;

    static std::atomic_int numCreated_;
};