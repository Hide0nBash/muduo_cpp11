#pragma once

#include "noncopyable.h"
#include "EventLoopThread.h"

class EventLoopThreadPool : noncopyable 
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThreadPool(EventLoop* baseLoop, const std::string &name);
    ~EventLoopThreadPool();

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    EventLoop* getNextLoop();
    std::vector<EventLoop*> getAllLoop();

    void setNumThreads(int numThread) { numThread_ = numThread; }
    bool started() { return started_; }
    const std::string &name() { return name_; }

private:
    EventLoop* baseLoop_;
    const std::string name_;
    int numThreads_;
    std::vector<EventLoopThread> threads_;
    std::vector<EventLoop*> loops_;
    bool started_;
    int next_;
};