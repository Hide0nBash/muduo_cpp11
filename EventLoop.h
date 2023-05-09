#pragma once

#include "noncopyable.h"
#include "Poller.h"
#include "CurrentThread.h"

class EventLoop : noncopyable {
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    void runInLoop(Functor func);
    void queueInLoop(Functor func);

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    int wakeup();

    bool isInLoopThread() { return threadId_ == CurrentThread::tid(); }
    

private:
    void handleRead();
    void doPendingFunctors();

    std::atomic_bool looping_;
    std::atomic_bool quit_;
    const pid_t threadId_;
    
    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::vector<Channel*> channels_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    std::atomic_bool callingPendingFunctors_;
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;
};