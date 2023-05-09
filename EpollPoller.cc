#include "EpollPoller.h"

EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop)
    , epoll_fd(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize)
{
    if(epoll_fd < 0) {
        LOG_FATAL("?")
    }
}

EpollPoller::~EpollPoller()
{
    ::close(epoll_fd);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList channels) override {
    int numEvents = ::epoll_wait(epoll_fd, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvents > 0) {
        fillActiveChannels(numEvents, channels);
        if(numEvents == channels.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents < 0) {
        LOG_DEBUG("?");
    }
    else {
        /*
        非重要逻辑暂时不处理
        */
    }
    return now;
}

void EpollPoller::updateChannel(Channel* channel) override{
    int index = channel->index();
    if(index == kNew || index == kDelete) {
        if(index == kNew) {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        else {

        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        // int fd = channel->fd();
        if(channel->isNoneEvents()) {
            channel->set_index(kDelete);
            update(EPOLL_CTL_DEL, channel);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel) override{
    int index = channel->index();
    if(index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    int fd = channel->fd();
    channels_.erase(fd);

    channel->set_index(kNew);
} 

void EpollPoller::fillActiveChannels(int numChannels, ChannelList channels) const {
    for(int i=0;i<numChannels;++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->setRevent(events_[i].events);
        channels->push_back(channel);
    }
}

void EpollPoller::update(int op, Channel* channel) {
    epoll_event event;
    ::memset(&event, 0, sizeof(event));

    int fd = channel->fd();
    event.data.fd = fd;
    event.data.ptr = channel;
    event.events = channel->events();
    if(::epoll_ctl(epoll_fd, op, fd, &event) < 0) {
        if(op == EPOLL_CTL_DEL) {
            LOG_ERROR("DEL error");
        }
        else {
            LOG_FATAL("ADD / MOD error");
        }
    }
}