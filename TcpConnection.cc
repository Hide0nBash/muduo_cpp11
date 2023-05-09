#pragma once

#include "TcpConnection.h"
#include <functional>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/tcp.h>

#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             const InetAddress& localaddr,
                             const InetAddress& peeraddr,
                             int sockfd)
    : loop_(loop)
    , name_(name)
    , localaddr_(localaddr)
    , peeraddr_(peeraddr)
    , state_(kDisconnected)
    , reading_(false)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
}

void TcpConnection::send(const std::string& buf) {
    if(state_ == kConnected) {
        if(loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        }
        else {
            loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if(stated_ == kDisconnected) {
        LOG_ERROR("DISconnected");
    }
    if(!channel_->isWriting() || outputBuffer_.size() == 0) {
        nwrote = ::write(channel_->fd(), data, len);
        if(nwrote >= 0) {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(writeCompleteCallback_, shared_from_this());
            }
        }
        else {
            nwrote = 0;
            if(errno != EWOULDBLOCK) {
                if(errno == EPIPE || errno == ECONNRESET) {
                    LOG_ERROR("/");
                    faultError = true;
                }
            }
        }
    }

    if(faultError == false && remaining > 0) {
        size_t oldLen = outputBuffer_.readableSize();
        if(oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_) {
            loop_->queueInLoop(highWaterMarkCallback_, shared_from_this());
        }
        outputBuffer_.append((char *)data + nwrote, remaining);
        if(!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if(stated_ == kConnected) {
        setState(kDisconnecting);        
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    if(!channel_->isWriting()) {
        socket_->shutDownWrite();
    }
}

void TcpConnection::connectEstablish() {
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroy() {
    if(state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp recieveTime) {
    int saveError = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveError);
    if(n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, recieveTime);
    }
    else if(n == 0) {
        handleClose();
    }
    else {
        handleError();
    }
}

void TcpConnection::handleWrite() {
    
    if(channel_->isWriting()) {
        int saveError = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveError);
        if(n > 0) {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableSize() == 0) {
                channel_->disableWriting();
                if(writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if(state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        }
        else {
            LOG_ERROR("?");
        }
    }
    else {
        LOG_ERROR("???");
    }
        
}

void TcpConnection::handleClose() {
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr conn(shared_from_this());
    connectionCallback_(conn);
    closeCallback_(conn);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d\n", name_.c_str(), err);
}