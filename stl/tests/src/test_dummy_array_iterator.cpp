#include "gtest_prompt.h"
#include "dummy_array_iterator.h"

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
