#pragma once

#include <functional>

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;
class InetAddress;

class Acceptor : noncopyable {
public:
    using newConnectionCallback = std::function<void(int sockfd, const InetAddress &inet);
    Acceptor(EventLoop* baseLoop, const InetAddress &addr, bool reuseport);
    ~Acceptor();
    void setNewConnectionCallback(const newConnectionCallback &cb) { newConnectionCallback_ = cb; }
    void listen();
    bool listening() { return listening_; }

private:
    void handleRead();
    bool listening_;
    EventLoop* loop_;
    Socket acceptSocket_;
    Channel *acceptChannel_;
    newConnectionCallsback newConnectionCallback_;
};