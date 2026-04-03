#include "gtest_prompt.h"
#include "ostream.h"
#include <memory>
#include <fstream>
#include <filesystem>
class TempFile {
public:
    TempFile(const std::string& content) {
        filePath = std::filesystem::temp_directory_path() / "test_file.txt";
        std::ofstream tempFile(filePath);
        tempFile << content;
        tempFile.close();
    }

    ~TempFile() {
        std::filesystem::remove(filePath);
    }

    const std::string& getFilePath() const {
        return filePath;
    }

private:
    std::string filePath;
};

class OutContent {
public:
    explicit OutContent(const std::string& path) : filePath(path), content("") {}

    ~OutContent() {
         try {
            if (std::filesystem::exists(filePath)) {
                std::filesystem::remove(filePath);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            
        }
    }
    const std::string& getFilePath() const {
        return filePath;
    }

    const std::string& getContent() const {
        std::ifstream tempFile(filePath, std::ios::in | std::ios::binary);
        if (!tempFile) {
            throw std::runtime_error("Unable to open file for reading: " + filePath);
        }
        content = std::string((std::istreambuf_iterator<char>(tempFile)), std::istreambuf_iterator<char>());
        return content;
    }

private:
    std::string filePath;
    mutable std::string content;
};

TEST(UnixFileInStreamTest, Read) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);

    char buffer[1024];
    size_t bytesRead = inStream.read(buffer, sizeof(buffer));

    EXPECT_EQ(bytesRead, 13);
    std::string readData(buffer, bytesRead);
    EXPECT_EQ(readData, "Hello, World!");
}

TEST(BufferedInStreamTest, GetChar) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    BufferedInStream bufferedInStream(inStream);

    int c = bufferedInStream.getchar();
    EXPECT_EQ(c, 'H');
}

TEST(BufferedInStreamTest, Read) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    BufferedInStream bufferedInStream(inStream);

    char buffer[1024];
    size_t bytesRead = bufferedInStream.read(buffer, sizeof(buffer));
    int n = bufferedInStream.getchar();

    EXPECT_EQ(bytesRead, 13);
    EXPECT_STREQ(buffer, "Hello, World!");
}

TEST(BufferedInStreamTest, ReadN) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    BufferedInStream bufferedInStream(inStream);

    char buffer[1024];
    size_t bytesReadn = bufferedInStream.readn(buffer, 5);
    EXPECT_EQ(bytesReadn, 5);
    std::string readData(buffer, bytesReadn);
    EXPECT_EQ(readData, "Hello");
}

TEST(InStreamTest, GetChar) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    int c = inStream.getchar();
    EXPECT_EQ(c, 'H');
}

TEST(InStreamTest, ReadN) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    char buffer[1024];
    size_t bytesReadn = inStream.readn(buffer, 5);
    EXPECT_EQ(bytesReadn, 5);
    EXPECT_STREQ(buffer, "Hello");
}

TEST(InStreamTest, ReadAll) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    std::string data = inStream.readall();
    EXPECT_EQ(data, "Hello, World!");
}

TEST(InStreamTest, ReadUntil) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    std::string data = inStream.readuntil(' ');
    EXPECT_EQ(data, "Hello, ");
}

TEST(InStreamTest, ReadUntil_C) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    std::string data = inStream.readuntil("llo", 3);
    EXPECT_EQ(data, "Hello");
}

TEST(InStreamTest, ReadUntilString) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    std::string data = inStream.readuntil("Wo");
    EXPECT_EQ(data, "Hello, Wo");
}

TEST(InStreamTest, ReadLine) {
    TempFile tempFile("Hello, World!\nNext line");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    inStream.readline("\n", 1);
}

TEST(InStreamTest, GetLine) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    std::string data = inStream.getline(' ');
    EXPECT_EQ(data, "Hello,");
}

TEST(InStreamTest, GetLine_C) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    std::string data = inStream.getline("llo", 3);
    EXPECT_EQ(data, "He");
}

TEST(InStreamTest, GetLineString) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    UnixFileInStream inStream(fd);
    std::string data = inStream.getline(std::string("World"));
    EXPECT_EQ(data, "Hello, ");
}

TEST(UnixFileOutStreamTest, Write) {
    OutContent outContent("/tmp/test_file.txt");
    auto p = out_file_open(outContent.getFilePath().c_str(), OpenFlag::Write);
    UnixFileOutStream* outStream = dynamic_cast<UnixFileOutStream*>(p.get());
    outStream->write("Hello, World!\n", 13);

    EXPECT_EQ(outContent.getContent(), "Hello, World!");
}

TEST(BufferedOutStream, Write) {
    OutContent outContent("/tmp/test_file.txt");
    auto p = out_file_open(outContent.getFilePath().c_str(), OpenFlag::Write);
    BufferedOutStream buffer_out = BufferedOutStream(*p);
    buffer_out.write("Hello, World!\naaaa\n", 18);
    buffer_out.flush();
    EXPECT_EQ(outContent.getContent(), "Hello, World!\naaaa");
    buffer_out.putchar('c');
    buffer_out.flush();
    EXPECT_EQ(outContent.getContent(), "Hello, World!\naaaac");
}

TEST(LineBufferedOutStream, Write) {
    OutContent outContent("/tmp/test_file.txt");
    auto p = out_file_open(outContent.getFilePath().c_str(), OpenFlag::Write);
    LineBufferedOutStream buffer_out = LineBufferedOutStream(*p);
    buffer_out.write("Hello, World!\naaaa\n", 18);
    EXPECT_EQ(outContent.getContent(), "Hello, World!\n");
    buffer_out.putchar('c');
    EXPECT_EQ(outContent.getContent(), "Hello, World!\n");
    buffer_out.putchar('\n');
    EXPECT_EQ(outContent.getContent(), "Hello, World!\naaaac\n");
}

TEST(OStreamTest, OutFileOpenNormal) {
    auto p = out_file_open("/dev/stdout", OpenFlag::Write);
    if (!p) {
        return;
    }
    p->puts("hello world\n");
    p->putchar('c');
    p->putchar('\n');
}

TEST(OStreamTest, OutFileOpenException) {
    ASSERT_THROW(out_file_open("/tmp/aaaaa.txt", OpenFlag::Read), std::system_error);
}

TEST(OStreamTest, InFileOpenNormal) {
    {
        auto p = out_file_open("/tmp/test_file.txt", OpenFlag::Write);
        p->puts("Hello!\nWorld!\n");
    }
    {
        auto p = in_file_open("/tmp/test_file.txt", OpenFlag::Read);
        auto s = p->getline('\n');
        printf("%s\n", s.c_str());
        s = p->getline('\n');
        printf("%s\n", s.c_str());
        s = p->getline('\n');
        printf("%s\n", s.c_str());
    }
}

TEST(OStreamTest, IFileOpenException) {
    ASSERT_THROW(in_file_open("/tmp/aaaaaaa", OpenFlag::Read), std::system_error);
}

TEST(OStreamTest, IO_IN) {
    TempFile tempFile("Hello, World!");
    int fd = open(tempFile.getFilePath().c_str(), O_RDONLY);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    int saved_stdin = dup(STDIN_FILENO);
    dup2(fd, STDIN_FILENO);

    char c = io_in.getchar();
    printf("%d\n", c);

    dup2(saved_stdin, STDIN_FILENO);
    close(fd);
}

TEST(OStreamTest, IO_OUT) {
    OutContent outContent("/tmp/test_file.txt");
    int fd = open(outContent.getFilePath().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    int saved_stdout = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);

    std::string test_data = "Hello, World!\n";
    io_out.puts(test_data.c_str());

    dup2(saved_stdout, STDOUT_FILENO);
    close(fd);

    EXPECT_EQ(outContent.getContent(), test_data);
}

TEST(OStreamTest, IO_ERR) {
    OutContent outContent("/tmp/test_file.txt");
    int fd = open(outContent.getFilePath().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(fd, 0) << "Failed to open file: " << strerror(errno);

    int saved_stderr  = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);

    std::string test_data = "Error: Something went wrong!\n";
    io_err.puts(test_data.c_str());

    dup2(saved_stderr , STDERR_FILENO);
    close(fd);

    EXPECT_EQ(outContent.getContent(), test_data);
   
}

TEST(OStreamTest, IO_perror) {
   io_perror("msg");
   perror("msg");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}