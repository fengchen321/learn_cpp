//
// Created by lsfco on 2024/9/14.
//
#include <iostream>
#include <iterator>
#include <functional>
#include <algorithm>
#include <cassert>
#include "gtest_prompt.h"

template <typename T, size_t const Size>
class dummy_array_iterator {
public:
    typedef dummy_array_iterator            self_type;
    typedef T                               value_type;
    typedef T&                              reference;
    typedef T*                              pointer;
    typedef std::random_access_iterator_tag iterator_category;
    typedef ptrdiff_t                       difference_type;
public:
    explicit dummy_array_iterator(pointer ptr, size_t const index)
            : ptr(ptr), index(index){}
    dummy_array_iterator(dummy_array_iterator const &o) = default;
    dummy_array_iterator& operator=(dummy_array_iterator const &o) = default;
    ~dummy_array_iterator() = default;

    self_type& operator++ () {
        if (index >= Size) {
            throw std::out_of_range("Iterator cannot be incremented past the end of range.");
        }
        ++index;
        return *this;
    }

    self_type operator++ (int) {
        self_type tmp = *this;
        ++*this;
        return tmp;
    }
    // 满足输入迭代器要求
    bool operator== (self_type const &other) const {
        assert(compatible(other));
        return index == other.index;
    }

    bool operator!= (self_type const &other) const {
        return !(*this == other);
    }

    reference operator*() const {
        if (ptr == nullptr) {
            throw std::bad_function_call();
        }
        return *(ptr + index);
    }

    pointer operator->() const {
        if (ptr == nullptr) {
            throw std::bad_function_call();
        }
        return ptr + index;
    }
    // 满足双向迭代器要求
    self_type& operator--() {
        if (index <= 0) {
            throw std::out_of_range("Iterator cannot be decremented past the beginning of range.");
        }
        --index;
        return *this;
    }

    self_type operator--(int) {
        self_type tmp = *this;
        --*this;
        return tmp;
    }

    // 满足随机访问迭代器要求
    self_type operator+(difference_type offset) const {
        if (index + offset < 0 || index + offset > Size) {
            throw std::out_of_range("Iterator cannot be incremented past the end of range.");
        }
        return self_type(ptr, index + offset);
    }

    self_type operator-(difference_type offset) const {
        if (index - offset < 0 || index - offset > Size) {
            throw std::out_of_range("Iterator cannot be decremented past the beginning of range.");
        }
        return self_type(ptr, index - offset);
    }

    difference_type operator-(self_type const &other) const {
        assert(compatible(other));
        return index - other.index;
    }
    bool operator<(self_type const &other) const {
        assert(compatible(other));
        return index < other.index;
    }

    bool operator>(self_type const &other) const {
        return other < *this;
    }

    bool operator<=(self_type const &other) const {
        return !(other < *this);
    }

    bool operator>=(self_type const &other) const {
        return !(*this < other);
    }

    self_type& operator+=(difference_type const offset) {
        if (index + offset < 0 || index + offset > Size) {
            throw std::out_of_range("Iterator cannot be incremented past the end of range.");
        }
        index += offset;
        return *this;
    }
    self_type& operator-=(difference_type const offset) {
        return *this += -offset;
    }

    value_type& operator[](difference_type const offset) {
        return *(*this + offset);
    }

    value_type const & operator[](difference_type const offset) const{
        return *(*this + offset);
    }

private:
    bool compatible(self_type const &other) const {
        return ptr == other.ptr;
    }
private:
    pointer ptr = nullptr;
    size_t index = 0;
};

template <typename Iterator>
class dummy_array_reverse_iterator {
public:
    typedef dummy_array_reverse_iterator<Iterator>  self_type;
    typedef typename Iterator::value_type           value_type;
    typedef typename Iterator::reference            reference;
    typedef typename Iterator::pointer              pointer;
    typedef typename Iterator::iterator_category    iterator_category;
    typedef typename Iterator::difference_type      difference_type;

    explicit dummy_array_reverse_iterator(Iterator it) : current(it) {}
    dummy_array_reverse_iterator(dummy_array_reverse_iterator const &o) = default;
    dummy_array_reverse_iterator& operator=(dummy_array_reverse_iterator const &o) = default;
    ~dummy_array_reverse_iterator() = default;

    reference operator*() const {
        Iterator tmp = current;
        return *--tmp;
    }

    pointer operator->() const {
        return &(operator*());
    }

    self_type& operator++() {
        --current;
        return *this;
    }

    self_type operator++(int) {
        self_type tmp = *this;
        --current;
        return tmp;
    }

    self_type& operator--() {
        ++current;
        return *this;
    }

    self_type operator--(int) {
        self_type tmp = *this;
        ++current;
        return tmp;
    }

    self_type operator+(difference_type n) const {
        return self_type(current - n);
    }

    self_type& operator+=(difference_type n) {
        current -= n;
        return *this;
    }

    self_type operator-(difference_type n) const {
        return self_type(current + n);
    }

    self_type& operator-=(difference_type n) {
        current += n;
        return *this;
    }

    reference operator[](difference_type n) const {
        return *(*this + n);
    }

    bool operator==(self_type const &other) const {
        return current == other.current;
    }

    bool operator!=(self_type const &other) const {
        return !(*this == other);
    }

    bool operator<(self_type const &other) const {
        return other.current < current;
    }

    bool operator>(self_type const &other) const {
        return other < *this;
    }

    bool operator<=(self_type const &other) const {
        return !(other < *this);
    }

    bool operator>=(self_type const &other) const {
        return !(*this < other);
    }

private:
    Iterator current;
};

template <typename Type, size_t const SIZE>
class dummy_array {
public:
    typedef dummy_array_iterator<Type, SIZE> iterator;
    typedef dummy_array_iterator<Type const, SIZE> constant_iterator;
    typedef dummy_array_reverse_iterator<iterator> reverse_iterator;
    typedef dummy_array_reverse_iterator<constant_iterator> constant_reverse_iterator;

public:
    Type& operator[](size_t const index) {
        if (index < SIZE) return _data[index];
        throw std::out_of_range("index out of range");
    }

    Type const& operator[](size_t const index) const {
        if (index < SIZE) return _data[index];
        throw std::out_of_range("index out of range");
    }

    Type* data() {
        return _data;
    }
    Type const* data() const {
        return _data;
    }

    size_t size() const {
        return SIZE;
    }
    bool empty() const {
        return SIZE == 0;
    }
public:
    iterator begin() {
        return iterator(_data, 0);
    }
    iterator end() {
        return iterator(_data, SIZE);
    }
    constant_iterator begin() const {
        return constant_iterator(_data, 0);
    }
    constant_iterator end() const {
        return constant_iterator(_data, SIZE);
    }
    constant_iterator cbegin() const {
        return constant_iterator(_data, 0);
    }
    constant_iterator cend() const {
        return constant_iterator(_data, SIZE);
    }
    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    constant_reverse_iterator crbegin() const {
        return constant_reverse_iterator(cend());
    }

    constant_reverse_iterator crend() const {
        return constant_reverse_iterator(cbegin());
    }

    ptrdiff_t ssize() const {
        return static_cast<ptrdiff_t>(SIZE);
    }
private:
    Type _data[SIZE] = {};
};

const size_t SIZE = 5;

void initializeArray(dummy_array<int, SIZE>& arr) {
    for (size_t i = 0; i < SIZE; ++i) {
        arr[i] = i + 1;
    }
}

TEST(DummyArrayTest, TestAssignmentAndAccess) {
    dummy_array<int, SIZE> arr;
    initializeArray(arr);

    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_EQ(arr[i], i + 1);
    }
}

TEST(DummyArrayTest, TestForwardIteration) {
    dummy_array<int, SIZE> arr;
    initializeArray(arr);

    size_t index = 0;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        ASSERT_EQ(*it, arr[index++]);
    }
}

TEST(DummyArrayTest, TestReverseIteration) {
    dummy_array<int, SIZE> arr;
    initializeArray(arr);

    size_t index = SIZE;
    for (auto it = arr.rbegin(); it != arr.rend(); ++it) {
        ASSERT_EQ(*it, arr[--index]);
    }
}

TEST(DummyArrayTest, TestConstForwardIteration) {
    dummy_array<int, SIZE> arr;
    initializeArray(arr);

    const dummy_array<int, SIZE>& carr = arr;
    size_t index = 0;
    for (auto it = carr.begin(); it != carr.end(); ++it) {
        ASSERT_EQ(*it, carr[index++]);
    }
}

TEST(DummyArrayTest, TestConstReverseIteration) {
    dummy_array<int, SIZE> arr;
    initializeArray(arr);

    const dummy_array<int, SIZE>& carr = arr;
    size_t index = SIZE;
    for (auto it = carr.crbegin(); it != carr.crend(); ++it) {
        ASSERT_EQ(*it, carr[--index]);
    }
}

TEST(DummyArrayTest, TestRandomAccessIterator) {
    dummy_array<int, SIZE> arr;
    initializeArray(arr);

    auto it = arr.begin();
    ASSERT_EQ(*(it + 2), arr[2]);
    ASSERT_EQ(*(it - (-2)), arr[2]);
    ASSERT_EQ((it + 2) - it, 2);
    ASSERT_EQ(it[2], arr[2]);

    it += 2;
    ASSERT_EQ(*it, arr[2]);
    it -= 2;
    ASSERT_EQ(*it, arr[0]);
}

TEST(DummyArrayTest, TestOutOfRange) {
    dummy_array<int, SIZE> arr;
    initializeArray(arr);

    try {
        auto bad_it = arr.begin() + SIZE;
        ++bad_it; // Should throw an exception
        FAIL() << "Expected std::out_of_range";
    } catch (const std::out_of_range&) {
        // Expected exception
    }

    try {
        auto bad_it = arr.begin() - 1;
        --bad_it; // Should throw an exception
        FAIL() << "Expected std::out_of_range";
    } catch (const std::out_of_range&) {
        // Expected exception
    }
}

TEST(DummyArrayTest, TestSizeAndEmpty) {
    dummy_array<int, SIZE> arr;
    ASSERT_EQ(arr.size(), SIZE);
    ASSERT_FALSE(arr.empty());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
