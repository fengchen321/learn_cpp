#include "gtest_prompt.h"
#include <vector>
#include <iostream>
#include "UniquePtr.h"

class File {
public:
    File(const char *path) {
        puts(__PRETTY_FUNCTION__);
        p = fopen(path, "r");
        if (!p) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }

    ~File() { // dtor
        puts(__PRETTY_FUNCTION__);
        if (p) {
            fclose(p);
        }
    }

    File(File const &other) = delete;  // copy ctor
    File &operator=(File const &other) = delete; // copy assignment

    File(File &&other) { // move ctor
        p = std::exchange(other.p, nullptr);
        puts(__PRETTY_FUNCTION__);
    }

    File &operator=(File &&other) noexcept{ // move assignment
        if (this != &other) {
            if (p) {
                fclose(p);
            }
            p = std::exchange(other.p, nullptr);
            puts(__PRETTY_FUNCTION__);
        }
        return *this;
    }
public:
    FILE* get() const {
        return p;
    }
    
private:
    FILE *p = nullptr;
};

TEST(UniquePtrTest, BigFive) {
    const char* filename = "a.txt";
    FILE* file = fopen(filename, "w");
    ASSERT_TRUE(file != nullptr);
    fputs("hello world", file);
    fclose(file);

    auto a = File(filename);
    ASSERT_EQ((char)fgetc(a.get()), 'h');
    ASSERT_EQ((char)fgetc(a.get()), 'e');
    remove(filename);
}

struct MyClass {
    int a, b, c;
};

struct Animal {
    virtual void speak() = 0;
    virtual ~Animal() = default;
};

struct Dog : Animal {
    int age;

    Dog(int age_) : age(age_) {
    }

    virtual void speak() {
        printf("Bark! I'm %d Year Old!\n", age);
    }
};

struct Cat : Animal {
    int &age;

    Cat(int &age_) : age(age_) {
    }

    virtual void speak() {
        printf("Meow! I'm %d Year Old!\n", age);
    }
};

TEST(UniquePtrTest, UniquePtr) {
    std::vector<UniquePtr<Animal>> zoo;
    int age = 3;
    zoo.push_back(makeUnique<Cat>(age));
    zoo.push_back(makeUnique<Dog>(age));
    for (auto const &a: zoo) {
        a->speak();
    }
    age++;
    for (auto const &a: zoo) {
        a->speak();
    }
}

void func(FILE *fp) {
    fclose(fp);
}

TEST(UniquePtrTest, FileUniquePtr) {
    const char* filename = "a.txt";
    FILE* file = fopen(filename, "w");
    fclose(file);

    UniquePtr<FILE> fp(fopen("a.txt", "r"));
    // func(fp.get());  // double free
    func(fp.release());

    fp.reset(fopen("a.txt", "r"));

    remove(filename);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}