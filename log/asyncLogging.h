#ifndef ASYNCLOGGING
#define ASYNCLOGGING

#include "thread.h"
#include "mutex.h"
#include "condition.h"
#include <string.h>

#include <string>
#include <vector>
#include <memory>
#include <cassert>

// LogBuffer 目前只能用于 日志系统后端
template <int BUFSIZE>
class LogBuffer {
public:
    LogBuffer()
        : _cur(_buf)
    {}

    void append(const char *logline, int len) {
        memcpy(_cur, logline, len);
        _cur += len;
    }

    void bzero() {
        memset(_buf, 0, sizeof(_buf));
    }
    char*  end()   { return _buf + BUFSIZE; }
    size_t avail() { return static_cast<size_t>(end() - _cur); }
    size_t size()  { return static_cast<size_t>(_cur - _buf); }
    const char* buf() const { return _buf; }
private:
    char  _buf[BUFSIZE];
    char *_cur; // 指向 append 的地址
};

class AsyncLogging {
public:
    using Buffer = LogBuffer<100 * 20000 * 2>; // 4w 条 (4MB)
    using BufferPointer = std::unique_ptr<Buffer>;
    // BPV <==> BufferPointerVector
    using BPV = std::vector<BufferPointer>;

    AsyncLogging(const char *basename, int rollSize, int flushInterval);
    ~AsyncLogging() {
        assert(!_running);
    }

    void start() {
        assert(!_running);
        _running = true;
        _thread.start();
    }

    void stop() {
        // 如果 _currentBuffer 中有数据是不能让 _running 变成 False 的，不然就丢失数据了
        // 必须是 前端退出后，才能调用后端的 stop
        assert(_running);
        size_t size;
        {
            MutexGuard guard(_lock);
            size = _currentBuffer->size();
        }
        printf("AsyncLogInit::stop()\tsize: %ld\n", size);
        while (size > 0) {
            _cond.wakeup();
            MutexGuard guard(_lock);
            size = _currentBuffer->size();
        }
        _running = false;
        _cond.wakeup();
        _thread.join();
    }

    // 前端调用接口，将一条日志发送给后端，而后端其实就是把日志缓存起来，等待合适时机落盘
    void append(const char *logline, int len);

    void mainRoutine();

private:
    // 滚动文件
    std::string rollFile();

private:
    // appender: file or stdout
    // std::unique_ptr<Appender> _upAppender;

    bool          _running;
    const char*   _basename;
    int           _rollSize;
    int           _flushInterval;
    BufferPointer _currentBuffer; // 用于缓存 日志前端 发送来的 日志
    BufferPointer _nextBuffer;    // _currentBuf 满的时候用 _nextBuf 顶替上
    BPV           _buffers;

    MutexLock     _lock; // 用于 前端与后端 数据传输时避免 race condition
    Condition     _cond; // 用于 至少 3s 内后端被叫醒 去 落盘

    Thread        _thread;
};

class AsyncLoggingWatcher {
public:
    AsyncLoggingWatcher(const char* progname);
    ~AsyncLoggingWatcher() { _upAsyncLogging->stop(); }
private:
    std::unique_ptr<AsyncLogging> _upAsyncLogging;
};

#endif
