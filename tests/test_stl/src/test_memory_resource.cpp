#include "gtest_prompt.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>
#include <list>
#include <memory_resource>

#define TICK(x) auto bench_##x = std::chrono::steady_clock::now();
#define TOCK(x) std::cerr << #x ": " << \
        std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - bench_##x).count()\
        << " s \n";

TEST(MemoryResourceTest, time_iota) {
    TICK(vector);
    std::vector<int> num;
    num.reserve(65536);
    for (size_t i = 0; i < 65536; i++) {
        num.push_back(i);
    }
    TOCK(vector);
    TICK(iota);
    std::vector<int> num_1;
    // num_1.reserve(65536); 不能用reserve是因为他只分配内存
    num_1.resize(65536);
    std::iota(num_1.begin(), num_1.end(), 0);
    TOCK(iota);
    EXPECT_EQ(num, num_1);
}

TEST(MemoryResourceTest, CheckAlignmentMethods) {
    size_t align = 8;
    for (size_t offset = 0; offset < 65536; ++offset) {
        EXPECT_EQ((offset + align - 1) & ~(align - 1), (offset + align - 1) / align * align);
    }
}

static char g_buf[65536 * 24];
struct my_memory_resource {
    char *m_buf = g_buf;
    size_t m_offset = 0;

    char *do_allocate(size_t n) {
        if (m_offset + n > sizeof(g_buf)) {
            throw std::bad_alloc();
        }
        char *ptr = m_buf + m_offset;
        m_offset += n;
        return ptr;
    }

    char *do_allocate(size_t n, size_t align) {
        m_offset = (m_offset + align - 1) / align * align;
        // m_offset = (m_offset + align - 1) & ~(align - 1);
        if (m_offset + n > sizeof(g_buf)) {
            throw std::bad_alloc();
        }
        char *ptr = m_buf + m_offset;
        m_offset += n;
        return ptr;
    }
};

template <class T>
struct my_custom_allocator {
    my_memory_resource *m_resource{};
    using value_type = T;

    my_custom_allocator(my_memory_resource *resource) : m_resource(resource) {}

    T *allocate(size_t n) {
        // char *ptr = m_resource->do_allocate(n * sizeof(T));
        char *ptr = m_resource->do_allocate(n * sizeof(T), alignof(T));
        return  reinterpret_cast<T*>(ptr);
    }

    void deallocate(T *p, size_t n) {}

    my_custom_allocator() = default;
    template <class U>
    constexpr my_custom_allocator(my_custom_allocator<U> const &other) noexcept : m_resource(other.m_resource) {
    }

    constexpr bool operator==(my_custom_allocator const &other) const noexcept {
        return m_resource == other.m_resource;
    }
};

TEST(MemoryResourceTest, defaultallocator) {
    {
        std::vector<char, std::allocator<char>> a;
        TICK(default_vector);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(default_vector);
        EXPECT_EQ(a.size(), 65536);
    }
    {
        std::list<char, std::allocator<char>> a;
        TICK(default_list);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(default_list);
        EXPECT_EQ(a.size(), 65536);
    }
}

TEST(MemoryResourceTest, Customallocator) {
    my_memory_resource mem;
    {
        std::vector<char, my_custom_allocator<char>> a{my_custom_allocator<char>{&mem}};
        TICK(custom_vector);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(custom_vector);
        EXPECT_EQ(a.size(), 65536);
    }
    mem.m_offset = 0;
    {
        std::list<char, my_custom_allocator<char>> a{my_custom_allocator<char>{&mem}};
        // prev(8) + next(8) + char(1) + padding(7) = 24 字节对齐
        TICK(custom_list);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(custom_list);
        EXPECT_EQ(a.size(), 65536);
    }
}
/*
monotonic_buffer_resource:栈上临时缓冲区(效率最高，但只适合短生命周期)
synchronized_pool_resource:允许多个线程共享的内存池(内部需要上锁，效率低)
unsynchronized_pool_resource:每个线程各自私有的内存池(内部不需要上锁，效率较高)
null_memory_resource:任何分配请求都会抛出 std::bad_alloc 异常
new_delete_resource: 转发给传统new/delete
*/
TEST(MemoryResourceTest, PMRallocator) {
    std::pmr::synchronized_pool_resource pool;
    {
        std::pmr::monotonic_buffer_resource mem{65536 * 2, &pool};
        // std::vector<char, std::pmr::polymorphic_allocator<char>> a{std::pmr::polymorphic_allocator<char>{&mem}};
        std::pmr::vector<char> a{&mem};
        TICK(pmr_vector);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(pmr_vector);
        EXPECT_EQ(a.size(), 65536);
    }
    {
        std::pmr::monotonic_buffer_resource mem{65536 * 24, &pool};
        // std::list<char, std::pmr::polymorphic_allocator<char>> a{std::pmr::polymorphic_allocator<char>{&mem}};
        std::pmr::list<char> a{&mem};
        TICK(pmr_list);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(pmr_list);
        EXPECT_EQ(a.size(), 65536);
    }
}

TEST(MemoryResourceTest, PMRallocator2) {
    std::pmr::unsynchronized_pool_resource mem{};
    {
        std::vector<char> a;
        TICK(vector);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(vector);
    }
    {
        std::pmr::vector<char> a{&mem};
        TICK(pmr_unsync_vector);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(pmr_unsync_vector);
    }
    {
        std::pmr::monotonic_buffer_resource lmem{&mem};
        std::pmr::vector<char> a{&lmem};
        TICK(pmr_monotonic_vector);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(pmr_monotonic_vector);
    }
    {
        std::list<char> a;
        TICK(list);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(list);
    }
    {
        std::pmr::list<char> a{&mem};
        TICK(pmr_unsync_list);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(pmr_unsync_list);
    }
    {
        std::pmr::monotonic_buffer_resource lmem{&mem};
        std::pmr::list<char> a{&lmem};
        TICK(pmr_monotonic_list);
        for (size_t i = 0; i < 65536; i++) {
            a.push_back(42);
        }
        TOCK(pmr_monotonic_list);
    }

}

struct memory_resource_inspector : std::pmr::memory_resource {
public:
    explicit memory_resource_inspector(std::pmr::memory_resource *upstream)
    : m_upstream(upstream) {}

private:
    void *do_allocate(size_t bytes, size_t alignment) override {
        void *p = m_upstream->allocate(bytes, alignment);
        std::cout << "allocate    " << p << "  " << bytes << "  " << alignment << "\n";
        return p;
    }

    bool do_is_equal(std::pmr::memory_resource const &other) const noexcept override {
        return other.is_equal(*m_upstream);
    }

    void do_deallocate(void *p, size_t bytes, size_t alignment) override {
        std::cout << "deallocate  " << p << "  " << bytes << "  " << alignment << "\n";
        return m_upstream->deallocate(p, bytes, alignment);
    }

    std::pmr::memory_resource *m_upstream;
};

TEST(MemoryResourceTest, PMRInspector) {
    memory_resource_inspector mem{std::pmr::new_delete_resource()};
    {
        std::pmr::list<int> l{&mem};
        for (size_t i = 0; i < 8; i++) {
            l.push_back(42);
        }
    }
    puts("------------------------------------");
    {
        std::pmr::vector<int> v{&mem};
        for (size_t i = 0; i < 8; i++) {
            v.push_back(42);
        }
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}