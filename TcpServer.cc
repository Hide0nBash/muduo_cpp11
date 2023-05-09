#include <functional>
#include <string>

#include "TcpServer.h"
#include "Buffer.h"
#include "TcpConnection.h"
#include "Logger.h"

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainLoop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}


TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& addr,
                     const std::string& name,
                     Option = kNoReusePort)
    : loop_(CheckLoopNotNull(loop))
    , name_(name)
    , ipPort(addr->toIpPort())
    , acceptor(new Acceptor(loop, addr, option = kReusePort))
    , pool(new EventLoopThreadPool(loop, name))
    , connectionCallback()
    , messageCallbackk()
    , nextConnId(1);
    , started_(0)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this
                                        , std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    loop_->quit();
    for(auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->loop()->runInLoop(std::bind(&TcpConnection::connectDestroy, this));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    pool_->setNumThreads(numThreads);
}   

void TcpServer::start() {
    if(started_ ++ == 0) {
        pool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& addr) {
    EventLoop* loop = pool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    std::string nameConn = name + buf;
    sockaddr_in local;
    ::memset(&local, 0, sizeof(local));
    socklen_t addrlen = sizeof(local);
    if(::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0) {
        LOG_ERROR("?????");
    }
    InetAddress localaddr(local);
    TcpConnectionPtr conn(loop, nameConn, local, addr, sockfd);
    connections_[nameConn] = conn;
    conn->setConnectionCallback(connectionCallBack_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    loop->runInLoop(&TcpConnection::connectEstablish, conn);
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    conn->loop()->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    connections_->erase(conn->name());
    EventLoop* loop = conn->loop();
    loop->runInLoop(std::bind(&TcpConnection::connectDestroy, this));
}