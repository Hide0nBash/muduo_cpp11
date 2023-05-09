#include "EventLoop.h"
#include <sys/epollfd.h>
#include "Poller.h"
#include "Channel.h"
#include "Logger.h"

#include <errno.h>
#include <memory>
#include <fcntl.h>
#include <unistd.h>


__thread EventLoop *t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

int createEventFd() {
    int fd = ::epollfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(fd < 0) {
        LOG_FATAL("?");
    }
    return fd;
}

EventLoop::EventLoop() 
    : looping_(false)
    , quit_(false)
    , threadId_(CurrentThread::tid())
    , poller_(new newDefaultPoller(this))
    , wakeupFd_(createEventFd())
    , callingPendingFunctors_(false)
    , wakeupChannel_(new Channel(this, wakeupFd_))

{
    if(t_loopInThisThread) {
        LOG_FATAL(">");
    }
    else {
        t_loopInThisThread = this;
    }

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    while(!quit) {
        channels_.clear();
        Timestamp pollrecvTime = poller_->poll(kPollTimeMs, channels);
        for(Channel* channel : channels) {
            channel->handleEvent(pollrecvTime);
        }
        doPendingFunctors();
    }
    
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if(!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor func) {
    if(isInLoopThread()) {
        func();
    }
    else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor func) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    if(!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);
}

bool hasChannel(Channel* channel) {
    return poller_->hasChannel(channel);
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one)) {
        LOG_ERROR("?");
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    int n = write(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one)) {
        LOG_ERROR("?");
    }
}

void EventLoop::doPendingFunctors() {
    callingPendingFunctors_ = true;
    
    std::vector<Functor> tmpFunctors_;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        tmpFunctors_.swap(pendingFunctors_);
    }

    for(const Functor func : pendingFunctors_) {
        func();
    }

    callingPendingFunctors_ = false;
}
