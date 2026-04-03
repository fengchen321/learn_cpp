#include "gtest_prompt.h"
#include "Array.h"


void fillArray(Array<int, 32> &a) {
    for (size_t i = 0; i < 32; i++) {
        a[i] = i;
    }
}

TEST(ArrayTest, operatorIndex) {
    Array<int, 32> a{1, 2, 3};
    fillArray(a);
    for (size_t i = 0; i < 32; i++) {
        ASSERT_EQ(a[i], i);
    }
}

void func_const(Array<int, 32> const &a) {}

TEST(ArrayTest, operatorIndexConst) {
    Array<int, 32> a{1, 2, 3};
    func_const(a);
}

TEST(ArrayTest, atMemberFunc) {
    Array<int, 32> a;
    fillArray(a);
    for (size_t i = 0; i < 32; i++) {
        ASSERT_EQ(a.at(i), i);
    }
    ASSERT_THROW(a.at(100), std::out_of_range);
    ASSERT_THROW(a.at(-1), std::out_of_range);
}

void func_size(Array<int32_t, 32> &a) {
    Array<int8_t, std::decay_t<decltype(a)>::size()> b;
    ASSERT_EQ(b.size(), 32);
}

TEST(ArrayTest, GetSize) {
    Array<int, 32> a;
    ASSERT_EQ(a.size(), 32);
    Array<int, 0> b;
    ASSERT_EQ(b.size(), 0);

    func_size(a);
}

TEST(ArrayTest, Iterators) {
    Array<int, 5> nums{1, 2, 4, 8, 16};

    auto print_nums = [&nums]() {
        for (auto it = nums.begin(); it != nums.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        for (auto it = nums.rbegin(); it != nums.rend(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    };

    auto print_const_nums = [&nums]() {
        for (auto it = nums.cbegin(); it != nums.cend(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        for (auto it = nums.crbegin(); it != nums.crend(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    };
    ASSERT_LOGS_STDOUT(print_nums(), "1 2 4 8 16 \n16 8 4 2 1 \n");
    ASSERT_LOGS_STDOUT(print_const_nums(), "1 2 4 8 16 \n16 8 4 2 1 \n");
}

TEST(ArrayTest, Fill) {
    Array<int, 32> a; 
    a.fill(10);
    for (auto it = a.begin(); it != a.end(); ++it) {
        ASSERT_EQ(*it, 10);
    }
}

TEST(ArrayTest, Swap) {
    Array<int, 32> a;
    Array<int, 32> b;
    fillArray(a);
    b.fill(10);

    a.swap(b);

    for (size_t i = 0; i < 32; i++) {
        ASSERT_EQ(a[i], 10);
        ASSERT_EQ(b[i], i);
    }
}

TEST(ArrayTest, FrontBack) {
    Array<int, 32> a; 
    fillArray(a);
    ASSERT_EQ(a.front(), 0);
    ASSERT_EQ(a.back(), 31);
}

TEST(ArrayTest, Empty) {
    Array<int, 32> a; 
    ASSERT_FALSE(a.empty());
}

TEST(ArrayTest, Init) {
    Array a{1, 2, 3};
    ASSERT_EQ(a[0], 1);
    ASSERT_EQ(a[1], 2);
    ASSERT_EQ(a[2], 3);
    ASSERT_EQ(a.size(), 3);
}

TEST(ArrayTest, CArrayToArray) {
    int c_array[4] = {1, 2, 3, 4};
    auto a = make_array(c_array);
    ASSERT_EQ(a.size(), 4);
    ASSERT_EQ(a[0], 1);
    ASSERT_EQ(a[1], 2);
    ASSERT_EQ(a[2], 3);
    ASSERT_EQ(a[3], 4);

    int c1_array[2] = {1, 3};
    auto b = make_array(c1_array);
    ASSERT_EQ(b.size(), 2);
    ASSERT_EQ(b[0], 1);
    ASSERT_EQ(b[1], 3);

    int c2_array[] = {3};
    auto c = make_array(c2_array);
    ASSERT_EQ(c.size(), 1);
    ASSERT_EQ(c[0], 3);

    int c3_array[0] = {}; // c3_array[] = {} 编译错误
    auto d = make_array(c3_array);
    ASSERT_EQ(d.size(), 0);
}

TEST(ArrayTest, ArrayLengthZero) {
    Array<int, 0> emptyArray;
    
    ASSERT_TRUE(emptyArray.empty());
    ASSERT_EQ(emptyArray.size(), 0);
    ASSERT_EQ(emptyArray.max_size(), 0);

    ASSERT_THROW(emptyArray.at(0), std::out_of_range);
    EXPECT_DEATH(emptyArray.front(), "");
    EXPECT_DEATH(emptyArray.back(), "");
    ASSERT_EQ(emptyArray.data(), nullptr);
    ASSERT_EQ(emptyArray.cdata(), nullptr);

    ASSERT_NO_THROW(emptyArray.fill(10)); 

    Array<int, 0> a;
    Array<int, 0> b;
    ASSERT_NO_THROW(a.swap(b));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}