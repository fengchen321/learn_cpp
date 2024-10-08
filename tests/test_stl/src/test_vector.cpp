#include "gtest_prompt.h"
#if 0
#include <vector>
#define Vector std::vector
#else
#include "Vector.h"
#endif


TEST(VectorTest, Size) {
    Vector<int> vec;
    ASSERT_EQ(vec.size(), 0);
    ASSERT_EQ(vec.capacity(), 0);
    ASSERT_EQ(vec.empty(), true);
}

TEST(VectorTest, OperatorIndex) {
    Vector<int> vec(3);
    for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = i;
    }
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i);
    }

    ASSERT_EQ(vec.at(2), 2);
    ASSERT_THROW(vec.at(3), std::out_of_range);
}

TEST(VectorTest, ResizeAndClear) {
    Vector<int> vec(3);
    for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = i;
    }
    vec.resize(5);
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i < 3) {
            ASSERT_EQ(vec[i], i);
        } else {
            ASSERT_EQ(vec[i], 0);
        }
    }
    vec.clear();
    ASSERT_EQ(vec.size(), 0);

    vec.resize(5, 2);
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], 2);
    }
    std::cout << std::endl;
}

TEST(VectorTest, copyctor) {
    Vector<int> vec(3);
    for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = i;
    }
    Vector<int> vec2 = vec;  // copy constructor
    for (size_t i = 0; i < vec2.size(); ++i) {
        ASSERT_EQ(vec2[i], i);
    }
    Vector<int> vec3;
    vec3 = vec;  // copy assignment operator
    for (size_t i = 0; i < vec3.size(); ++i) {
        ASSERT_EQ(vec3[i], i);
    }
}

TEST(VectorTest, movector) {
    Vector<int> vec(3);
    for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = i;
    }
    Vector<int> vec2 = std::move(vec);  // move constructor
    ASSERT_EQ(vec2.size(), 3);
    for (size_t i = 0; i < vec2.size(); ++i) {
        ASSERT_EQ(vec2[i], i);
    }
    Vector<int> vec3;
    vec3 = std::move(vec2);  // move assignment operator
    for (size_t i = 0; i < vec3.size(); ++i) {
        ASSERT_EQ(vec3[i], i);
    }
}

TEST(VectorTest, PushBack) {
    Vector<int> vec(3);
    for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = i;
    }
    ASSERT_EQ(vec.back(), 2);
    vec.push_back(3);
    ASSERT_EQ(vec.size(), 4);
    ASSERT_EQ(vec.front(), 0);
    ASSERT_EQ(vec.back(), 3);
}

TEST(VectorTest, Reserve) {
    Vector<int> vec;
    for (size_t i = 0; i < 4; ++i) {
        vec.push_back(i);
    }
    vec.reserve(10);
    ASSERT_EQ(vec.capacity(), 10);
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i);
    }
}

TEST(VectorTest, Erase) {
    Vector<int> vec;
    for (int i = 0; i < 8; ++i) {
        vec.push_back(i);
    }
    auto result = vec.erase(vec.begin() + 2);
    ASSERT_EQ(*result, 3);
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i < 2) {
            ASSERT_EQ(vec[i], i);
        }
        else {
            ASSERT_EQ(vec[i], i + 1);
        }
    }

    auto result2 = vec.erase(vec.begin() + 2, vec.end());
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i);
    }
    ASSERT_EQ(result2, vec.end());
}

TEST(VectorTest, Ctor) {
    Vector<int> vec;
    ASSERT_EQ(vec.size(), 0);
    ASSERT_EQ(vec.capacity(), 0);
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], 0);
    }

    Vector<int> vec1(4);
    for (size_t i = 0; i < vec1.size(); ++i) {
        ASSERT_EQ(vec1[i], 0);
    }

    Vector<int> vec2(4, 2);
    for (size_t i = 0; i < vec2.size(); ++i) {
        ASSERT_EQ(vec2[i], 2);
    }
    
    int first[4] = {1, 2, 3, 4};
    auto vec3 = Vector<int>(first, first + 4);
    for (size_t i = 0; i < vec3.size(); ++i) {
        ASSERT_EQ(vec3[i], i + 1);
    }

    auto vec4 = Vector{1, 2, 3, 4};
    for (size_t i = 0; i < vec4.size(); ++i) {
        ASSERT_EQ(vec4[i], i + 1);
    }
}

TEST(VectorTest, Assign) {
    Vector<int> vec;
    vec.assign(4, 2);
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], 2);
    }

    int first[4] = {1, 2, 3, 4};
    vec.assign(first, first + 4);
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i + 1);
    }

    vec.assign({0,2,4,6});
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i * 2);
    }
}

TEST(VectorTest, Insert1) {
    Vector<int> vec;
    for (int i = 0; i < 8; ++i) {
        vec.push_back(i);
    }
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i);
    }
    // 常量引用
    auto result = vec.insert(vec.begin() + 2, 5);
    ASSERT_EQ(*result, 5);
    ASSERT_EQ(vec.size(), 9);
    ASSERT_EQ(std::distance(vec.begin(), result), 2);
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i == 2) {
            ASSERT_EQ(vec[i], 5);
        }
        else if (i < 2) {
            ASSERT_EQ(vec[i], i);
        }
        else {
            ASSERT_EQ(vec[i], i - 1);
        }
    }
    // 右值引用
    int b = 30;
    auto result2 = vec.insert(vec.end(), std::move(b));
    ASSERT_EQ(std::distance(vec.begin(), result2), 9);
    ASSERT_EQ(*result2, 30);
    ASSERT_EQ(vec.back(), 30);
    ASSERT_EQ(vec.size(), 10);
}

TEST(VectorTest, Insert2) {
    Vector<int> vec;
    for (int i = 0; i < 8; ++i) {
        vec.push_back(i);
    }
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i);
    }
   
    // 常量引用 n个元素
    auto result = vec.insert(vec.begin() + 2, 0, 6);
    ASSERT_EQ(*result, 2);
    ASSERT_EQ(vec.size(), 8);
    ASSERT_EQ(std::distance(vec.begin(), result), 2);
    auto result1 = vec.insert(vec.begin() + 2, 2, 6);
    ASSERT_EQ(*result1, 6);
    ASSERT_EQ(vec.size(), 10);
    ASSERT_EQ(std::distance(vec.begin(), result1), 2);
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i >= 2 && i < 4) {
            ASSERT_EQ(vec[i], 6);
        }
        else if (i < 2) {
            ASSERT_EQ(vec[i], i);
        }
        else {
            ASSERT_EQ(vec[i], i - 2);
        }
    }
}

TEST(VectorTest, Insert3) {
    Vector<int> vec;
    Vector<int> vec1(6, 4);
    for (int i = 0; i < 8; ++i) {
        vec.push_back(i);
    }
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i);
    }

    auto result = vec.insert(vec.begin() + 1, vec1.begin() + 2, vec1.end());
    ASSERT_EQ(*result, 4);
    ASSERT_EQ(vec.size(), 12);
    ASSERT_EQ(std::distance(vec.begin(), result), 1);
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i >= 1 && i < 5) {
            ASSERT_EQ(vec[i], 4);
        }
        else if (i < 1) {
            ASSERT_EQ(vec[i], i);
        }
        else {
            ASSERT_EQ(vec[i], i - 4);
        } 
    }
}

TEST(VectorTest, EmplacebackAndPop) {
    Vector<int> vec;
    vec.emplace_back(1);
    vec.emplace_back(2);
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i + 1);
    }
    vec.pop_back();
    vec.pop_back();
    ASSERT_EQ(vec.size(), 0);
}

TEST(VectorTest, Swap) {
    Vector<int> vec = {1, 2, 3, 4};
    Vector<int> vec1 = {5, 6, 7, 8, 9, 10};
    vec.swap(vec1);
    ASSERT_EQ(vec.size(), 6);
    ASSERT_EQ(vec1.size(), 4);
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i + 5);
    }
    for (size_t i = 0; i < vec1.size(); ++i) {
        ASSERT_EQ(vec1[i], i + 1);
    }
}

TEST(VectorTest, Emplace) {
    Vector<int> vec;
    vec.emplace(vec.end(), 1);
    int b = 2, c = 3;
    vec.emplace(vec.end(), b);
    vec.emplace(vec.end(), std::move(c));
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], i + 1);
    }
}

TEST(VectorTest, ElementAccess) {
    Vector<int> vec(3);
    for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = i;
    }
    ASSERT_EQ(vec.back(), 2);
    vec.push_back(3);
    ASSERT_EQ(vec.size(), 4);
    ASSERT_EQ(vec.front(), 0);
    ASSERT_EQ(vec.back(), 3);
    ASSERT_EQ(vec.data()[1], 1);
}

TEST(VectorTest, ShrinkToFit) {
    Vector<int> v;
    ASSERT_EQ(v.capacity(), 0);
    v.resize(100);
    ASSERT_EQ(v.capacity(), 100);
    v.resize(50);
    ASSERT_EQ(v.capacity(), 100);
    v.shrink_to_fit();
    ASSERT_EQ(v.capacity(), 50);
    v.clear();
    ASSERT_EQ(v.capacity(), 50);
    v.shrink_to_fit();
    ASSERT_EQ(v.capacity(), 0);
    for (int i = 1000; i < 1300; ++i)
        v.push_back(i);
    ASSERT_EQ(v.capacity(), 512);
    v.shrink_to_fit();
    ASSERT_EQ(v.capacity(), 300);
}

TEST(VectorTest, Iterators) {
    Vector<int> nums{1, 2, 4, 8, 16};

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

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}