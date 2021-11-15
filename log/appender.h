#ifndef APPENDER_H
#define APPENDER_H

class Appender {
public:
    virtual ~Appender() {}
    virtual void append(const char *logline, int len) {}
};

class AppenderStdout : public Appender {
public:
    //default 构造与析构
    // 将 日志 追加到 控制台
    void append(const char *logline, int len) override {}
};

class AppenderFile : public Appender {
public:
    AppenderFile(const char* filename)
        : _filename(filename),
          _openFile(fopen(filename, "a"))
    {}
    
    ~AppenderFile() {
        fflush(_openFile);
        fclose(_openFile);
    }

    // 将 日志 追加到 文件中去
    void append(const char *logline, int len) override {
        fwrite(logline, len, 1, _openFile);
        fflush(_openFile);
    }
private:
    const char* _filename;
    FILE*       _openFile;
};

#endif
