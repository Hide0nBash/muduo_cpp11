#include "Socket.h"

Socket::~Socket() {
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localaddr) {
    if(0 != ::bind(sockfd_, (sockaddr *)localaddr.getSockAddr(), sizeof(sockaddr_in))) {
        LOG_FATAL("?");
    }
}

void Socket::listen() {
    if(0 != ::listen(sockfd_, 1024)) {
        LOG_FATAL("?");
    }
}

void Socket::accept(InetAddress *peeraddr) {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    ::memset(&addr, 0, len);
    int connfd = ::accept4(sockfd_, (sockaddr *)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd >= 0) {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutDownWrite() {
    if(::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_ERROR("?");
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}
void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}
void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}
void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}