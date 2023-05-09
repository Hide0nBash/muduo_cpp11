#include "Poller.h"
#include "Channel.h"

Channel::Channel(int timeoutMs, EventLoop* loop)
    : loop_(loop)
{
}

bool Channel::hasChannel(Channel* channel) {
    auto it = channels.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}