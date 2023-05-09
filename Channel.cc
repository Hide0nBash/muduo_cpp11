#include "Channel.h"

Channel::Channel(EventLoop* loop, int fd) 
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revent_(0)
    , index_(-1)
    , tied_(false)
{
}

Channel::~Channel()
{
}

void tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp recieveTime) {
    if(tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if(guard) {
            handleEventWithGuard(recieveTime);
        }
    }
    else {
        handleEventWithGuard(recieveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp recieveTime) {
    if((revent_ & EPOLLHUP) && !(revent_ & EPOLLIN)) {
        if(closeCallback_) {
            closeCallback_();
        }
    }

    if(revent_ & EPOLLERR) {
        if(errorCallback_) {
            errorCallback_();
        }
    }

    if((revent_ & EPOLLIN) || (revent_ & EPOLLPRI)) {
        if(readCallback_) {
            readCallback_();
        }
    }

    if(revent_ & EPOLLOUT) {
        if(writeCallback_) {
            writeCallback_();
        }
    }
}