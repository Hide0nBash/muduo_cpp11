#pragma once

#include "noncopyable.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"

#include <functional>
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>

class TcpServer : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    enum Option {
        kNoReusePort,
        kReusePort
    }
    TcpServer(EventLoop* loop,
              const InetAddress& addr,
              const std::string& name,
              Option option = kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(ThreadInitCallback cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(ConnectionCallback cb) { connectionCallBack_ = cb; }
    void setMessageCallback(MessageCallback cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = cb; }

    void start();

private:
    void newConnection(int sockfd, const InetAddress& addr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);
    
    using ConnectMap = std::unordered_map<std::string, TcpConnectionPtr>;
    EventLoop* loop_;
    std::string name_;
    std::string ipPort_;

    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> pool_;

    ConnectionCallback connectionCallBack_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback = threadInitCallback_;

    std::atomic started_;
    int nextConnId;
    ConnectMap connections_;
};