#pragma once
#include <memory>
#include <string>
#include <atomic>
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "noncopyable.h"
#include "Callbacks.h"

class TcpConnection : noncopyable {
public:
    TcpConnection(EventLoop* loop, const std::string& name
                  , const InetAddress& localaddr, const InetAddress& peeraddr, int sockfd);
    ~TcpConnection();
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) { highWaterMarkCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    
    EventLoop* loop() { return loop_; }
    const std::string& name() { return name_; }
    const InetAddress& localaddr() { return localaddr_; }
    const InetAddress& peeraddr() { return peeraddr_; }

    bool isConnected() { return state_ == kConnected; }
    
    void send(const std::string& buf);
    void shutdown();

    void connectEstablish();
    void connectDestroy();

private:
    enum StateE {
        kConnected;
        kConnecting;
        kDisconnected;
        kDisconnecting;
    }
    void handleRead(Timestamp recieveTime);
    void handleWrite();
    void handleError();
    void handleClose();
    void sendInLoop();
    void shutdownInLoop();
    
    void setState(StateE state) { state_ = state; }
    std::atomic_int state_;
    bool reading_;

    EventLoop* loop_;
    const std::string& name_;
    const InetAddress& localaddr_;
    const InetAddress& peeraddr_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    ConnectionCallback connectionCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    MessageCallback messageCallback_;
    size_t highWaterMark_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
};