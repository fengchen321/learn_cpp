#include "ostream.h"

void io_perror(const char *msg) {
    io_err.puts(msg);
    io_err.puts(": ");
    io_err.puts(strerror(errno));
    io_err.puts("\n");
}

int InStream::getchar() {
    char c;
    size_t n = read(&c, 1);
    return n == 1 ? c : EOF;
}

size_t InStream::readn(char *__restrict s, size_t len) {
    size_t n = read(s, len);
    if (n == 0) return 0;
    while (n < len) {
        size_t m = read(s + n, len - n);
        if (m == 0) break;
        n += m;
    }
    return n;
}

std::string InStream::readall() {
    std::string s;
    s.resize(32);
    char *buf = &s[0];
    size_t pos = 0;
    while (true) {
        size_t n = read(buf + pos, s.size() - pos);
        if (n == 0) break;
        pos += n;
        if (pos == s.size()) {
            s.resize(s.size() * 2);
            buf = &s[0];
        }
    }
    s.resize(pos);
    return s;
}

std::string InStream::readuntil(char eol) {
    std::string s;
    while (true) {
        int c = getchar();
        if (c == EOF) break;
        s.push_back(c);
        if (c == eol) break;
    }
    return s;
}

std::string InStream::readuntil(const char *__restrict eol, size_t neol) {
    std::string s;
    while (true) {
        int c = getchar();
        if (c == EOF) break;
        s.push_back(c);
        if (s.size() >= neol) {
            if (memcmp(s.data() + s.size() - neol, eol, neol) == 0) {
                break;
            }
        }
    }
    return s;
}

void InStream::readline(const char *__restrict eol, size_t neol) {
    std::string s = getline(eol, neol);
    fprintf(stderr, "readline: %s\n", s.c_str());
}

std::string InStream::getline(char eol) {
    std::string s = readuntil(eol);
    if (s.size() > 0 && s.back() == eol) {
        s.pop_back();
    }
    return s;
}

std::string InStream::getline(const char *__restrict eol, size_t neol) {
    std::string s = readuntil(eol, neol);
    if (s.size() >= neol 
            && memcmp(s.data() + s.size() - neol, eol, neol) == 0) {
        s.resize(s.size() - neol);
    }
    return s;
}

size_t UnixFileInStream::read(char *__restrict s, size_t len) {
    if (len == 0) return 0;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ssize_t n = ::read(fd, s, len);
    if (n < 0) {
        throw std::system_error(errno, std::generic_category());
    }
    return n;
}

bool BufferedInStream::refill() {
    top = 0;
    max = in.read(buf, BUFSIZ);
    return max > 0;
}

int BufferedInStream::getchar() {
    if (top == max) {
        if (!refill()) {
            return EOF;
        }
    }
    return buf[top++];
}

size_t BufferedInStream::read(char *__restrict s, size_t len) {
    char *__restrict p = s;
    while (p != s + len) {
        if (top == max) {
            if (p != s || !refill()) {
                break;
            }
        }
        int c = buf[top++];
        *p++ = c;
    }
    return p - s;
}

size_t BufferedInStream::readn(char * __restrict s, size_t len) {
    char *__restrict p = s;
    while (p != s + len) {
        if (top == max) {
            if (!refill()) {
                break;
            }
        }
        int c = buf[top++];
        *p++ = c;
    }
    return p - s;
}

void UnixFileOutStream::write(const char *__restrict s, size_t len) {
    if (len == 0) return;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ssize_t written = ::write(fd, s, len);
    if (written < 0) {
        throw std::system_error(errno, std::generic_category());
    }
    if (written == 0) {
        throw std::system_error(EPIPE, std::generic_category());
    }
    while ((size_t)written != len) {
        written = ::write(fd, s, len);
        if (written < 0) {
            throw std::system_error(errno, std::generic_category());
        }
        if (written == 0) {
            throw std::system_error(EPIPE, std::generic_category());
        }
        s += written;
        len -= written;
    }
}

void BufferedOutStream::flush() {
    if (top) {
        out.write(buf, top);
        top = 0;
    }
}

void BufferedOutStream::putchar(char c) {
    if (top == BUFSIZ) {
        flush();
    }
    buf[top++] = c;
}

void BufferedOutStream::write(const char *__restrict s, size_t len) {
    for (const char *__restrict p = s; p != s + len; ++p) {
        if (top == BUFSIZ) {
            flush();
        }
        buf[top++] = *p;
    }
}

void LineBufferedOutStream::flush() {
    if (top) {
        out.write(buf, top);
        top = 0;
    }
}

void LineBufferedOutStream::putchar(char c) {
    if (top == BUFSIZ) {
        flush();
    }
    buf[top++] = c;
    if (c == '\n') {
        flush();
    }
}

void LineBufferedOutStream::write(const char *__restrict s, size_t len) {
    for (const char *__restrict p = s; p != s + len; ++p) {
        if (top == BUFSIZ) {
            flush();
        }
        buf[top++] = *p;
        if (*p == '\n') {
            flush();
        }
    }
}

std::unique_ptr<OutStream> out_file_open(const char *path, OpenFlag flag) {
    int oflag = openFlagToUnixFlag.at(flag);
    int fd = ::open(path, oflag, 0666);
    if (fd < 0) {
        throw std::system_error(errno, std::generic_category());
    }
    auto file = std::make_unique<UnixFileOutStream>(fd);
    return file;
}

std::unique_ptr<InStream> in_file_open(const char *path, OpenFlag flag) {
    int oflag = openFlagToUnixFlag.at(flag);
    int fd = ::open(path, oflag);
    if (fd < 0) {
        throw std::system_error(errno, std::generic_category());
    }
    auto file = std::make_unique<UnixFileInStream>(fd);
    return file;
}