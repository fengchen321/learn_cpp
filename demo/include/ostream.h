#pragma once
#include <cstdio>
#include <thread>
#include <unistd.h>
#include <system_error>
#include <cstring>
#include <map>
#include <fcntl.h>
// Linux:
// stdout: _IOLBF 行缓冲
// stderr: _IONBF 无缓冲
// fopen: _IOFBF 完全缓冲

// MSVC:
// stdout: _IONBF 无缓冲
// stderr: _IONBF 无缓冲
// fopen: _IOFBF 完全缓冲
enum OpenFlag {
    Read,
    Write,
    Append,
    ReadWrite,
};

static std::map<OpenFlag, int> openFlagToUnixFlag = {
    {OpenFlag::Read, O_RDONLY},
    {OpenFlag::Write, O_WRONLY | O_TRUNC | O_CREAT},
    {OpenFlag::Append, O_WRONLY | O_APPEND | O_CREAT},
    {OpenFlag::ReadWrite, O_RDWR | O_CREAT},
};

struct InStream {
    virtual size_t read(char *__restrict s, size_t len) = 0;
    virtual ~InStream() = default;

    virtual int getchar();

    virtual size_t readn(char * __restrict s, size_t len);

    std::string readall();

    std::string readuntil(char eol);

    std::string readuntil(const char *__restrict eol, size_t neol);

    std::string readuntil(std::string const &eol) {
        return readuntil(eol.data(), eol.size());
    }

    void readline(const char *__restrict eol, size_t neol);
#if __cpp_lib_string_view
    void readline(std::string_view eol) {
        readline(eol.data(), eol.size());
    }
#endif

    std::string getline(char eol);
    std::string getline(const char *__restrict eol, size_t neol);
    std::string getline(std::string const &eol) {
        return getline(eol.data(), eol.size());
    }

#if __cpp_lib_string_view
    void getline(std::string_view eol) {
        getline(eol.data(), eol.size());
    }
#endif
};

struct UnixFileInStream : InStream {
public:
    explicit UnixFileInStream(int fd_) : fd(fd_) {}

    UnixFileInStream(UnixFileInStream &&) = delete;
    ~UnixFileInStream() {
        ::close(fd);
    }
public:
    size_t read(char *__restrict s, size_t len);

    int file_handle() const noexcept {
        return fd;
    }
private:
    bool refill();
private:
    int fd;
};

struct UnixFileInStreamOwned : UnixFileInStream {
    explicit UnixFileInStreamOwned(int fd_) : UnixFileInStream(fd_) {}
    ~UnixFileInStreamOwned() {
        ::close(file_handle());
    }
};

struct BufferedInStream : InStream {
public:
    explicit BufferedInStream(InStream &in_)
        : in(in_), buf((char *)valloc(BUFSIZ)) {}
    BufferedInStream(BufferedInStream &&) = delete;
    ~BufferedInStream() {
        free(buf);
    }
public:
    int getchar();
    size_t read(char *__restrict s, size_t len);
    size_t readn(char * __restrict s, size_t len);

    InStream &base_stream() const noexcept {
        return in;
    }
private:
    bool refill();
private:
    InStream &in;
    char *buf;
    size_t top = 0;
    size_t max = 0;
};

struct BufferedInStreamOwned : BufferedInStream {
    explicit BufferedInStreamOwned(std::unique_ptr<InStream> in) 
        : BufferedInStream(*in) {
        (void)in.release();
    }
    BufferedInStreamOwned(BufferedInStreamOwned &&) = delete;
    ~BufferedInStreamOwned() {
        delete &base_stream();
    }
};

struct OutStream {
    virtual void write(const char *__restrict s, size_t len) = 0;
    virtual ~OutStream() = default;

    void puts(const char *__restrict s) {
        write(s, strlen(s));
    }

    void puts(std::string const &s) {
        write(s.data(), s.size());
    }
#if __cpp_lib_string_view
    void puts(std::string_view s) {
        write(s.data(), s.size());
    }
#endif
    virtual void putchar(char c) {
        write(&c, 1);
    }

    virtual void flush() {}
};

struct UnixFileOutStream : OutStream {
public:
    explicit UnixFileOutStream(int fd_) : fd(fd_) {}
    UnixFileOutStream(UnixFileOutStream &&) = delete;
    ~UnixFileOutStream() {
        ::close(fd);
    }
public:
    void write(const char *__restrict s, size_t len);

    int file_handle() const noexcept {
        return fd;
    }

private:
    int fd;
};

struct UnixFileOutStreamOwned : UnixFileOutStream {
    explicit UnixFileOutStreamOwned(int fd_) : UnixFileOutStream(fd_) {}
    ~UnixFileOutStreamOwned() {
        ::close(file_handle());
    }
};

struct BufferedOutStream : OutStream {
public:
    explicit BufferedOutStream(OutStream &out) 
        : out(out), buf((char *)valloc(BUFSIZ)){}
    BufferedOutStream(BufferedOutStream &&) = delete;
    ~BufferedOutStream() {
        flush();
        free(buf);
    }
public:
    void flush();
    void putchar(char c);
    void write(const char *__restrict s, size_t len);
    OutStream &base_stream() const noexcept {
        return out;
    }

private:
    OutStream &out;
    char *buf;
    size_t top = 0;
};

struct BufferedOutStreamOwned : BufferedOutStream {
    explicit BufferedOutStreamOwned(std::unique_ptr<OutStream> out)
        : BufferedOutStream(*out) {
        (void)out.release();
    }
    BufferedOutStreamOwned(BufferedOutStreamOwned &&) = delete;
    ~BufferedOutStreamOwned() {
        delete &base_stream();
    }
};

struct LineBufferedOutStream : OutStream {
public:
    explicit LineBufferedOutStream(OutStream &out) 
        : out(out), buf((char *)valloc(BUFSIZ)){}
    LineBufferedOutStream(BufferedOutStream &&) = delete;
    ~LineBufferedOutStream() {
        flush();
        free(buf);
    }
public:
    void flush();
    void putchar(char c);
    void write(const char *__restrict s, size_t len);
    OutStream &base_stream() const noexcept {
        return out;
    }

private:
    OutStream &out;
    char *buf;
    size_t top = 0;
};

struct LineBufferedOutStreamOwned : LineBufferedOutStream {
    explicit LineBufferedOutStreamOwned(std::unique_ptr<OutStream> out)
        : LineBufferedOutStream(*out) {
        (void)out.release();
    }
    LineBufferedOutStreamOwned(LineBufferedOutStreamOwned &&) = delete;
    ~LineBufferedOutStreamOwned() {
        delete &base_stream();
    }
};

std::unique_ptr<OutStream> out_file_open(const char *path, OpenFlag flag);
std::unique_ptr<InStream> in_file_open(const char *path, OpenFlag flag);

static BufferedInStreamOwned io_in(std::make_unique<UnixFileInStream>(STDIN_FILENO));
static LineBufferedOutStreamOwned io_out(std::make_unique<UnixFileOutStream>(STDOUT_FILENO));
static UnixFileOutStream io_err(STDERR_FILENO);

void io_perror(const char *msg);
