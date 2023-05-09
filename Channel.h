#pragma once

#include "EventLoop.h"
#include "noncopyable.h"
#include "Timestamp.h"

class Channel : noncopyable {
public:
    using ReadEventCallback = std::function<void(Timestamp)>;
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void setReadCallback(ReadEventCallback cb);
    void setWriteCallback(EventCallback cb);
    void setErrorCallback(EventCallback cb);
    void setCloseCallback(EventCallback cb);

    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= kReadEvent; update(); }
    void enableWriting() { events |= kWriteEvent; update(); }
    void disableWriting() { events_ &= kWriteEvent; update(); }
    void disableAll() { events_ |= kNoneEvent; update(); }

    void handleEvent(Timestamp);
    void setRevent(int revt) { revent = revt; }
    void events() { return events_; }
    void fd() { return fd_; }
    void isReading() { return events_ & kReadEvent; }
    void isWriting() { return events_ & kWriteEvent; }
    void isNoneEvent() { return events_ & kNoneEvent; }

    void index() { return index_; }
    void setIndex(int idx) { index_ = idx; }
    
    void tie(const shared_ptr<void> &);

    void remove();
private:
    void update();
    void handleEventWithGuard(Timestamp recieveTime);
    
    static const int kReadEvent;
    static const int kWriteEvent;
    static const int kNoneEvent;

    int events_;
    int revent_;

    EventLoop* loop;
    int fd_;
    int index_;

    bool tied_;
    std::weak_ptr<void> tie_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
};
