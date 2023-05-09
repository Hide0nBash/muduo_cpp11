#pragma once

#include "noncopyable.h"
#include "EventLoop.h"
#include <vector>
#include <map>

class EventLoop;
class Channel;

class Poller : noncopyable {
public:
    using ChannelList = std::vector<Channel*>;
    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    bool hasChannel(Channel* channel);

    virtual Timestamp poll(int timeoutMs, ChannelList activeChannels) = 0;
    static Poller* newDefaultPoller(EventLoop* loop);
protected:
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap channels_;
private:
    EventLoop* loop_;
};