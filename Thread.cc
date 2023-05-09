#include "Thread.h"

Thread::Thread(ThreadFunc func, const std::string &name) 
    : name_(name)
    , started_(false)
    , joined_(false)
    , tid_(0),
    , func_(std::move(func))
{
    setDefaultName();
}

Thread::~Thread() {
    if(started_ && !joined_) {
        thread_->detach();
    }
}

void Thread::setDefaultName() {
    int num = ++numCreated_;
    if(name_.empty()) {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

void start() {
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();
    }))
    sem_wait(&sem);
}

void join() {
    joined_ = true;
    thread_->join();
}