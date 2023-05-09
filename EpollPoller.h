#pragma once

#include "Poller.h"
#include <sys/epoll>
#include <vector>
#include "Timestamp.h"

class Channel;

class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
    Timestamp poll(int timeoutMs, ChannelList channels) override;

private:
    static const int kInitEventListSize = 16;
    void fillActiveChannels(int numChannels, ChannelList channels) const;
    void update(int op, Channel *channel);

    using EventList = std::vector<epoll_event>;
    
    int epoll_fd;
    EventList events_;
}