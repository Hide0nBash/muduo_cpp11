#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"


int createNonBlocking() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0) {
        LOG_FATAL("?");
    }
    return sockfd;
}


Acceptor::Acceptor(EventLoop* loop, cosnt InetAddress& addr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createNonBlocking())
    , acceptChannel_(loop, acceptSocket_.fd())
    , listening_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_->setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_->disableAll();
    acceptChannel_->remove();
}

void Acceptor::listen() {
    listening = true;
    acceptSocket_->listen();
    acceptChannel_->enableReading();
}

void Acceptor::handleRead() {
    InetAddress addr;
    int connfd = acceptSocket_.accept(addr);
    if(connfd > 0) {
        if(newConnectionCallback_) {
            newConnectionCallback_(connfd, addr);
        }
        else {
            ::close(connfd);
        }
    }
    else {
        LOG_FATAL("?");
    }
}