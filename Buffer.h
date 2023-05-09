#pragma once

#include<string>
#include<vector>
#include<algorithm>
#include<stddef.h>

class Buffer {
public:
    static const int kPrependSize = 8;
    static const int kInitSize = 1024;

    explicit Buffer(size_t initalSize = kInitSize) 
        : buffer_(kPrependSize + initalSize)
        , readIndex_(kPrependSize)
        , writeIndex_(kPrependSize) 
    {
    }
    size_t readableSize() { return writeIndex_ - readIndex_; }
    size_t writeableSize() { return buffer_.size() - kPrependSize; }
    size_t prependableSize() { return readIndex_; }

    void retrieve(size_t len) {
        if(len < readableSize()) {
            readIndedx += len;
        }
        else {
            retrieveAll();
        }
    }
    void retrieveAll() {
        readIndex_ = kPrependSize;
        writeIndex_ = kPrependSize;
    }

    std::string retrieveAllAsString() { return retrieveAsString(readableSize); }
    std::string retrieveAsString(size_t len) {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    void enableWrite(size_t len) {
        if(writeableSize() < len) {
            makeSpace(len);
        }
    }

    void append(const char* data, size_t len) {
        enableWrite(len);
        std::copy(data, data+len, beginWrite());
        writeIndex_ += len;
    }

    char* peek() { return begin() + kPrependSize; }
    char* beginWrite() { return begin() + writeIndex_; }
    const char* beginWrite() { return begin() + writeIndex_; }
 
    ssize_t readFd(int fd, int *saveErrno);
    ssize_t writeFd(int fd, int *saveErrno);

private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }

    void makeSpace(size_t len) {
        if(prependableSize() + writeableSize() >= len + kPrependSize) {
            size_t readable = readableSize();
            std::copy(begin() + readIndex_,
                      begin() + writeIndex_,
                      begin() + kPrependSize);
            readIndex_ = kPrependSize;
            writeIndex_ = kPrependSize + readable;
        }
        else {
            buffer_.resize(writeIndex_ + len);
        }
    }
    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;
};