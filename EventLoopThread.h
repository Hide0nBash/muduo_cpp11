#pragma once

#include "Thread.h"
#include "noncopyable.h"
#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>

class EventLoopThread : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                    const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();
private:
    void threadFunc();
    
    EventLoop* loop_;
    Thread thread_;
    bool exiting_;

    std::mutex mutex_;
    std::conditional_variable cond_;
    ThreadInitCallback callback_;
};