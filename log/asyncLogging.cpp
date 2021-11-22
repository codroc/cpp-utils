#include "asyncLogging.h"
#include "logger.h"
#include "logStream.h"
#include "appender.h"
#include "timestamp.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// 全局的 日志后端
AsyncLogging *g_asyncLog;

void AsyncLogOutput(const char *msg, int len) {
    g_asyncLog->append(msg, len);
}

AsyncLoggingWatcher::AsyncLoggingWatcher(const char* progname)
    : _upAsyncLogging(new AsyncLogging(progname, 1000*1000*100, 3))
{
    g_asyncLog = _upAsyncLogging.get();
    LogStream::setOutputFunc(AsyncLogOutput);
    _upAsyncLogging->start();
}


AsyncLogging::AsyncLogging(const char *basename, int rollSize, int flushInterval)
    : _running(false),
      _basename(basename),
      _rollSize(rollSize),
      _flushInterval(flushInterval),
      _currentBuffer(new Buffer),
      _nextBuffer(new Buffer),
      _buffers(),
      _lock(),
      _cond(_lock),
//       _startLock(),
//       _started(_startLock),
      _thread(std::bind(&AsyncLogging::mainRoutine, this), "AsyncLogger")
{
    _currentBuffer->bzero();
    _nextBuffer->bzero();
    _buffers.reserve(10); // 20MB * 10
}

void AsyncLogging::append(const char *logline, int len) {
    // printf("start append to logger's back-end\n");
    // 这里需要加锁，因为会有 其他 worker 线程 和 后端线程来竞争 _currentBuffer
    MutexGuard guard(_lock);
    if (_currentBuffer->avail() > static_cast<size_t>(len)) { // ok, append to _currentBuffer
        _currentBuffer->append(logline, len);
        // for test:
        // char tmpbuf[len + 1];
        // memset(tmpbuf, 0, len + 1);
        // memcpy(tmpbuf, logline, len);
        // printf("%s", tmpbuf);
    }
    else { // _currentBuffer 满了，用 _nextBuffer 顶替，实际的操作，可以通过 swap 两个 buffer 来实现
        _buffers.push_back(std::move(_currentBuffer));
        if (_nextBuffer)
            _currentBuffer = std::move(_nextBuffer);
        else { // 如果 _nextBuffer 都被用掉了，说明 现阶段 日志量 很大，需要 wakeup 后端线程 将日志 落盘，并 new 一个 _nextBuffer 出来
            _currentBuffer.reset(new Buffer);
            _cond.wakeup(); // 按理说 3s 超时 就会 落盘，并确保 _nextBuffer 是 != 0 的；那程序执行到这里就说明，在 3s 内就立即写满了 2 个 Buffer，即此时 日志流量 达到 3.33 MB/s，或 3W+ 条日志每秒
        }
        _currentBuffer->append(logline, len);
    }
}

extern int FastSecondToDate(const time_t& unix_sec, struct tm* tm, int time_zone);

// class LogStream;

// extern template std::string formatInteger<int>(int v);


// rollFile 是根据 rollSize 来进行文件更新的，
// 文件命名方式：程序名.日期.主机名.进程id.log
std::string AsyncLogging::rollFile() {
    // 首先进行文件名构造
    struct tm t;
    char tmp[40];
    memset(tmp, 0, sizeof(tmp));
    std::string ret(_basename);

    int64_t secs = Timestamp::now().microSeconds() / Timestamp::kMicroSecondsPerSecond;
    FastSecondToDate(static_cast<time_t>(secs), &t, 8);
    strftime(tmp, sizeof(tmp), "%Y-%m-%d", &t);
    ret.push_back('.');
    ret.append(tmp);

    ret.push_back('.');
    ret.append(formatInteger(static_cast<int>(getpid())).c_str());

    ret.append(".log0");
    // printf("start roll file! %s\n", ret.c_str());
    
    // 执行脚本，对原来的 日志包 解包，对每个日志文件重命名（0=>1，1=>2 ...），然后将 1~N 的日志文件重新压缩打包，父进程等待子进程退出，这阶段父进程不能持有任何锁
    char *args[3];
    char newfile[40];
    memset(newfile, 0, sizeof(newfile));
    memcpy(newfile, ret.c_str(), ret.size());
    args[1] = newfile;
    args[2] = 0;
    pid_t pid;
    if ((pid = fork()) == 0)
        execv("jiaoben", args);
    assert(pid == wait(0)); // 等待子进程结束
    return ret;
}


void AsyncLogging::mainRoutine() {
    BufferPointer buffer1(new Buffer);
    BufferPointer buffer2(new Buffer);
    BPV           buffers;
    std::unique_ptr<Appender> appender(new AppenderFile(rollFile().c_str()));
    int written = 0; // 用于记录已写日志数据量
    // 按照 5W/s 的并发连接数来算，每条连接 3 条日志，那么就是 15W 条/s 的日志量，这将接近充满 4 个 Buffer
    while(_running) {
        buffers.clear();
        { // critical section
            MutexGuard guard(_lock);
            while (_running && _buffers.empty() && _currentBuffer->size() == 0) {
                _cond.waitForSecs(3); // 如果没有需要 append 到 file 的日志，就继续等待或3s超时
            }
            // 有日志需要 append 到文件
            // 包括 _buffers 中的以及 _currentBuffer 中的日志
            // 所以先将 _currentBuffer push_back 到 _buffers 中
            _buffers.push_back(std::move(_currentBuffer));
            // 为了尽量减少 critical section 中的指令数，将 _buffers 和 _currentBuffer 中的内容 swap 到 local variable 中，这样就可以快速解锁，把锁让出去
            _currentBuffer.swap(buffer1);
            // 这里尽量不要在 critical section 中分配内存，因为分配内存时间开销挺大的，把分配内存的部分放到 外面去。
            if (!_nextBuffer)
                _nextBuffer.swap(buffer2);
            buffers.swap(_buffers);
        }
        // printf("start to write disk\n");
        buffer1.reset(new Buffer);
        if (!buffer2)
            buffer2.reset(new Buffer);
        // 首先要判断，是否需要 roll file
        int needWrite = 0;
        for (auto &pBuf : buffers)
            needWrite += pBuf->size();
        if (written + needWrite > _rollSize) {
            appender.reset(); // 一个文件写满了，关闭文件，不然 脚本 在 执行 tar 时会出错
            std::string fname = rollFile(); // 会被阻塞，等待 jiaoben 进程结束
            appender.reset(new AppenderFile(fname.c_str()));
            written = 0;
        }
        // 去实施 真正 的 落盘
        // 落盘的内容 全在 buffers 中
        for (auto &pBuf : buffers)
            appender->append(pBuf->buf(), pBuf->size());
        written += needWrite;
    }
}
