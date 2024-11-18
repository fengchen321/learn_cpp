#include "gtest_prompt.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <memory>
struct Resource {
    Resource() {
        puts("Resource default constructor");
        p = malloc(1);
        vaild = true;
    }
    Resource(Resource &&other) {
        puts("Resource move constructor");
        p = other.p;
        vaild = other.vaild;
        other.vaild = false;
    }

    Resource(const Resource &other) = delete;

    ~Resource() {
        if (vaild) {
            puts("Resource destructor 1");
            free(p);
        }
    }
    void* p;
    bool vaild;
};

void func(Resource r) {}

TEST(STD_MOVE_TEST, move) {
    Resource r;
    func(std::move(r));
}

struct Resource_ptr {
private:
    struct Impl {
        void* p;
        Impl() {
            puts("Impl default constructor");
            p = malloc(1);
        }
        Impl(Impl const &other) {
            puts("Impl copy constructor");
            p = malloc(1);
        }
        ~Impl() {
            puts("Impl destructor");
            free(p);
        }
        void speak() const {
            printf("Impl speak, this = %p\n", p);
        }
    };
    std::shared_ptr<Impl> impl;

    Resource_ptr(std::shared_ptr<Impl> impl_) : impl(impl_) {}
public:
    Resource_ptr(): impl(std::make_shared<Impl>()) {}
    Resource_ptr(Resource_ptr const &) = default;

    Resource_ptr clone() const {
        return std::make_shared<Impl>(*impl);
    }

    Impl * operator->() {
        return impl.get();
    }
};

void func(Resource_ptr r) {
    r->speak();
    auto y = r;
    y->speak();

    auto z = r.clone();
    z->speak();
}

TEST(STD_MOVE_TEST, ptr) {
    auto x = Resource_ptr();
    func(x);
}

// P-IMPL 模式 head file
struct Resource_IMPL {
private:
    struct Impl;

    std::unique_ptr<Impl> impl;

public:
    Resource_IMPL();
    void use();
    ~Resource_IMPL();
};
// P-IMPL 模式 impl file
struct Resource_IMPL::Impl {
    FILE *p;

    Impl() {
        puts("open file");
        p = fopen("CMakeCache.txt", "r");
    }

    Impl(Impl &&) = delete;

    void use() {
        printf("use file %p\n", p);
    }

    ~Impl() {
        puts("close file");
        fclose(p);
    }
};

Resource_IMPL::Resource_IMPL():impl(std::make_unique<Impl>()) {
}

void Resource_IMPL::use() {
    return impl->use();
}

Resource_IMPL::~Resource_IMPL() = default;

TEST(STD_MOVE_TEST, pimpl) {
    Resource_IMPL r;
    r.use();
}

/*
* finally  用于在作用域结束时执行某个回调函数
*/
template <class Callback>
struct Finally {
    Callback func;
    bool valid;

    Finally() : func(), valid(false) {}

    Finally(Callback func) 
        : func(func), valid(true) {
    }

    Finally(Finally &&that) noexcept 
        : func(std::move(that.func)), valid(that.valid) {
        that.valid = false;
    }

    Finally &operator=(Finally &&that) noexcept {
        if (this != &that) {
            if (valid) {
                func();
            }
            func = std::move(that.func);
            valid = that.valid;
            that.valid = false;
        }
        return *this;
    }

    void cancel() {
        valid = false;
    }

    void trigger() {
        if (valid) {
            func();
        }
        valid = false;
    }

    ~Finally() {
        if (valid) {
            func();
        }
    }
};

template <class Callback> // C++17 CTAD
Finally(Callback) -> Finally<Callback>;

TEST(STD_MOVE_TEST, finally) {
    Finally cb = [] {
        puts("invoke finally ");
    };
    // cb.trigger();
    srand(time(NULL));
    bool success = rand() % 2 == 0;
    if (success) {
        puts("operation false");
        return;
    }
    puts("operation success");
    cb.cancel();
}

/*
* 自动生成代码对齐
*/
struct IndentGuard {
    IndentGuard(std::string &indent_) : indent(indent_) {
        old_indent = indent;
        indent += "  ";
    }
    IndentGuard(IndentGuard &&) = delete;

    ~IndentGuard() {
        indent = old_indent;
    }

    std::string &indent;
    std::string old_indent;
};

struct Codegen {
    std::string code;
    std::string indent;

    void emit(std::string const &text) {
        code += indent + text + "\n";
    }

    void emit_variable(std::string const &name) {
        code += indent + "int " + name + ";\n";
    }
    
    void code_gen() {
        emit("#include <iostream>");
        emit("int main() {");
        {
            IndentGuard guard(indent);
            emit_variable("a");
            emit_variable("b");
        }
        emit("}");
    }
};

TEST(STD_MOVE_TEST, code_gen) {
    Codegen cg;
    cg.code_gen();
    std::cout << cg.code << std::endl;
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
