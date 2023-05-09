#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

ssize_t Buffer::readFd(int fd, int *saveErrno) {
    char extrabuf[65536] = {0};
    struct iovec vec[2];
    size_t writeable = writeableSize();
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writeable;
    vec[1].iovbase = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writeable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if(n < 0) {
        *saveErrno = errno;
    }
    else if(n <= writeable) {
        writeIndex_ += n;
    }
    else {
        writeIndex_ = buffer_.size();
        append(extrabuf, n - writeable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno) {
    ssize_t n = ::write(fd, peek(), readableSize());
    if(n < 0) {
        *saveErrno = errno;
    }
    return n;
}