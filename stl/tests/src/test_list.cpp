#include "gtest_prompt.h"
#define STL_STANDARD 0
#if STL_STANDARD
#include <list>
#define List std::list
#else
#include "List.h"
#endif

void print_list(List<int> &list) {
    for (auto it = list.begin(); it != list.end(); ++it) {
        printf("%d ", *it);
    }
    printf("\n");
}

TEST(ListTest, Ctor) {
    List<int> list1;
    ASSERT_EQ(list1.size(), 0);

    List<int> list2(4);
    ASSERT_EQ(list2.size(), 4);
   
    List<int> list3(4, 2);
    ASSERT_EQ(list3.size(), 4);
#if !STL_STANDARD
    size_t i = 0;
    list3.foreach([&](int &val) { printf("arr[%zd] = %d\n", i, val); ++i;});
#endif
    
    int first[4] = {1, 2, 3, 4};
    auto list4 = List<int>(first, first + 4);
    ASSERT_EQ(list4.size(), 4);

    auto list5 = List{4, 5, 6, 7};
    ASSERT_EQ(list5.size(), 4);
}

TEST(ListTest, copyctor) {
    List<int> list1(4, 2);
    for (auto it = list1.begin(); it != list1.end(); ++it) {
        ASSERT_EQ(*it, 2);
    }
    
    List<int> list2 = list1;  // copy constructor
    for (auto it = list2.begin(); it != list2.end(); ++it) {
        ASSERT_EQ(*it, 2);
    }
    ASSERT_EQ(list1.size(), 4);
    ASSERT_EQ(list2.size(), 4);
    
    List<int> list3;
    list3 = list1;  // copy assignment operator
    for (auto it = list3.begin(); it != list3.end(); ++it) {
        ASSERT_EQ(*it, 2);
    }
    ASSERT_EQ(list1.size(), 4);
    ASSERT_EQ(list3.size(), 4);
}

TEST(ListTest, movector) {
    List<int> list1(4, 2);
    
    List<int> list2 = std::move(list1);  // move constructor
    for (auto it = list2.begin(); it != list2.end(); ++it) {
        ASSERT_EQ(*it, 2);
    }
    ASSERT_EQ(list1.size(), 0);
    ASSERT_EQ(list2.size(), 4);


    List<int> list3;
    list3 = std::move(list2);  // move assignment operator
    for (auto it = list3.begin(); it != list3.end(); ++it) {
        ASSERT_EQ(*it, 2);
    }
    ASSERT_EQ(list2.size(), 0);
    ASSERT_EQ(list3.size(), 4);
}

TEST(ListTest, Assign) {
    List<int> list1;
    list1.assign(4, 2);
    ASSERT_LOGS_STDOUT(print_list(list1), "2 2 2 2 \n");

    int first[4] = {1, 2, 3, 4};
    list1.assign(first, first + 4);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 2 3 4 \n");

    list1.assign({0,2,4,6});
    ASSERT_LOGS_STDOUT(print_list(list1), "0 2 4 6 \n");
}

TEST(ListTest, ElementAccess) {
    List<int> list1 = {1, 2, 3, 4};
    ASSERT_EQ(list1.size(), 4);
    ASSERT_EQ(list1.front(), 1);
    ASSERT_EQ(list1.back(), 4);
}

TEST(ListTest, Capacity) {
    List<int> list1 = {1, 2, 3, 4};
    ASSERT_EQ(list1.size(), 4);
    ASSERT_EQ(list1.empty(), false);
    list1.clear();
    ASSERT_EQ(list1.size(), 0);
    ASSERT_EQ(list1.empty(), true);
}

TEST(ListTest, Erase) {
    List<int> list1;
    for (int i = 0; i < 8; ++i) {
        list1.push_back(i);
    }
    auto it = list1.begin();
    std::advance(it, 2);  // 移动到第3个元素的位置
    auto result = list1.erase(it);
    ASSERT_EQ(*result, 3);
    ASSERT_LOGS_STDOUT(print_list(list1), "0 1 3 4 5 6 7 \n");

    it = list1.begin();
    std::advance(it, 2);
    auto result1 = list1.erase(it, list1.end());
    ASSERT_EQ(*result1, 2);
    ASSERT_EQ(list1.size(), 2);
    list1.pop_front();
    ASSERT_EQ(list1.size(), 1);
    ASSERT_EQ(list1.front(), 1);
    list1.pop_back();
    ASSERT_EQ(list1.size(), 0);
}

TEST(ListTest, PushBack) {
    List<int> list1(3);
    list1.push_back(3);
    int x = 4;
    list1.push_back(x);
    list1.push_back(std::move(x));
    ASSERT_LOGS_STDOUT(print_list(list1), "0 0 0 3 4 4 \n");
}

TEST(ListTest, EmplacebackAndPop) {
    List<int> list1;
    list1.emplace_back(1);
    list1.emplace_back(2);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 2 \n");
    list1.pop_back();
    list1.pop_back();
    ASSERT_EQ(list1.size(), 0);
}

TEST(ListTest, frontTest) {
    List<int> list1 = {1, 2, 3, 4};
    list1.push_front(0);

    int x = 1;
    list1.push_front(x);
    list1.push_front(std::move(x));
    list1.emplace_front(x);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 1 0 1 2 3 4 \n");
}

TEST(ListTest, Insert1) {
    List<int> list1;
    for (int i = 0; i < 8; ++i) {
        list1.push_back(i);
    }
    ASSERT_LOGS_STDOUT(print_list(list1), "0 1 2 3 4 5 6 7 \n");
    // 常量引用
    auto it = list1.begin();
    std::advance(it, 2);
    auto result = list1.insert(it, 5);
    ASSERT_EQ(*result, 5);
    ASSERT_EQ(list1.size(), 9);
    ASSERT_LOGS_STDOUT(print_list(list1), "0 1 5 2 3 4 5 6 7 \n");
    // 右值引用
    int b = 30;
    auto result2 = list1.insert(list1.end(), std::move(b));
    ASSERT_EQ(std::distance(list1.begin(), result2), 9);
    ASSERT_EQ(*result2, 30);
    ASSERT_EQ(list1.back(), 30);
    ASSERT_EQ(list1.size(), 10);
}

TEST(ListTest, Insert2) {
    List<int> list1;
    for (int i = 0; i < 8; ++i) {
        list1.push_back(i);
    }
    ASSERT_LOGS_STDOUT(print_list(list1), "0 1 2 3 4 5 6 7 \n");
   
    // 常量引用 n个元素
    auto it = list1.begin();
    std::advance(it, 2);
    auto result = list1.insert(it, 0, 6);
    ASSERT_EQ(*result, 2);
    ASSERT_EQ(list1.size(), 8);
    ASSERT_EQ(std::distance(list1.begin(), result), 2);

    it = list1.begin();
    std::advance(it, 2);
    auto result1 = list1.insert(it, 2, 6);
    ASSERT_EQ(*result1, 6);
    ASSERT_EQ(list1.size(), 10);
    ASSERT_EQ(std::distance(list1.begin(), result1), 2);
    ASSERT_LOGS_STDOUT(print_list(list1), "0 1 6 6 2 3 4 5 6 7 \n");
}

TEST(ListTest, Insert3) {
    List<int> list1;
    List<int> list2(6, 4);
    for (int i = 0; i < 8; ++i) {
        list1.push_back(i);
    }
    ASSERT_LOGS_STDOUT(print_list(list1), "0 1 2 3 4 5 6 7 \n");

    auto insert_pos = list1.begin();
    std::advance(insert_pos, 1);

    auto from_begin = list2.begin();
    auto from_end = list2.end();
    std::advance(from_begin, 2);
    auto result = list1.insert(insert_pos, from_begin, from_end);

    ASSERT_EQ(*result, 4);
    ASSERT_EQ(list1.size(), 12);
    ASSERT_EQ(std::distance(list1.begin(), result), 1);
    ASSERT_LOGS_STDOUT(print_list(list1), "0 4 4 4 4 1 2 3 4 5 6 7 \n");
}


TEST(ListTest, Emplace) {
    List<int> list1;
    list1.emplace(list1.end(), 1);
    int b = 2, c = 3;
    list1.emplace(list1.end(), b);
    list1.emplace(list1.end(), std::move(c));
    ASSERT_LOGS_STDOUT(print_list(list1), "1 2 3 \n");
}

TEST(ListTest, Iterators) {
    List<int> nums{1, 2, 4, 8, 16};

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

TEST(ListTest, Remove) {
    List<int> nums{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    nums.remove(5);
    ASSERT_EQ(nums.size(), 9);
    ASSERT_LOGS_STDOUT(print_list(nums), "1 2 3 4 6 7 8 9 10 \n");
}

TEST(ListTest, RemoveIf) {
    List<int> nums{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    nums.remove_if([](int n) { return n % 2 == 0; });
    ASSERT_LOGS_STDOUT(print_list(nums), "1 3 5 7 9 \n");
}

TEST(ListTest, Resize) {
    List<int> list1;
    for (int i = 0; i < 8; ++i) {
        list1.push_back(i);
    }
    list1.resize(5);
    ASSERT_EQ(list1.size(), 5);
    ASSERT_LOGS_STDOUT(print_list(list1), "0 1 2 3 4 \n");
    
    list1.resize(8);
    ASSERT_EQ(list1.size(), 8);
    ASSERT_LOGS_STDOUT(print_list(list1), "0 1 2 3 4 0 0 0 \n");
    
    list1.clear();
    ASSERT_EQ(list1.size(), 0);

    list1.resize(5, 2);
    ASSERT_EQ(list1.size(), 5);
    ASSERT_LOGS_STDOUT(print_list(list1), "2 2 2 2 2 \n");
}

TEST(ListTest, Swap) {
    List<int> list1 = {1, 2, 3, 4};
    List<int> list2 = {5, 6, 7, 8, 9, 10};
    list1.swap(list2);
    ASSERT_EQ(list1.size(), 6);
    ASSERT_EQ(list2.size(), 4);
    ASSERT_LOGS_STDOUT(print_list(list1), "5 6 7 8 9 10 \n");
    ASSERT_LOGS_STDOUT(print_list(list2), "1 2 3 4 \n");
}

TEST(ListTest, Merge) {
    List<int> list1 = {1, 2, 4, 3};
    List<int> list2 = {5, 6, 8, 7, 9, 10};
    list1.merge(list2);
    ASSERT_EQ(list1.size(), 10);
    ASSERT_EQ(list2.size(), 0);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 2 4 3 5 6 8 7 9 10 \n");
}

TEST(ListTest, Merge1) {
    List<int> list1 = {1, 2, 4, 3, 5, 6, 8, 7, 9, 10};
    List<int> list3 = {11, 12};
    list1.merge(std::move(list3));
    ASSERT_EQ(list1.size(), 12);
    ASSERT_EQ(list3.size(), 0);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 2 4 3 5 6 8 7 9 10 11 12 \n");
}

TEST(ListTest, Merge2) {
    List<int> list1 = {1, 2, 4, 3, 5, 6, 8, 7, 9, 10};
    List<int> list3 = {11};
    list1.merge(std::move(list3));
    print_list(list1);
    print_list(list3);
    ASSERT_EQ(list1.size(), 11);
    ASSERT_EQ(list3.size(), 0);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 2 4 3 5 6 8 7 9 10 11 \n");
}

TEST(ListTest, Splice1) {
    List<int> list1 = {1, 2, 3, 4};
    List<int> list2 = {5, 6, 7, 8, 9, 10};
    auto insert_pos = list1.begin();
    std::advance(insert_pos, 1);
    list1.splice(insert_pos, list2);
    print_list(list1);
    print_list(list2);
    ASSERT_EQ(list1.size(), 10);
    ASSERT_EQ(list2.size(), 0);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 5 6 7 8 9 10 2 3 4 \n");
}

TEST(ListTest, Splice1_2) {
    List<int> list1 = {1, 2, 3, 4};
    List<int> list2 = {5};
    auto insert_pos = list1.begin();
    std::advance(insert_pos, 1);
    list1.splice(insert_pos, list2);
    print_list(list1);
    print_list(list2);
    ASSERT_EQ(list1.size(), 5);
    ASSERT_EQ(list2.size(), 0);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 5 2 3 4 \n");
}

TEST(ListTest, Splice1_3) {
    List<int> list1 = {1, 2, 3, 4};
    List<int> list2 = {};
    auto insert_pos = list1.begin();
    std::advance(insert_pos, 1);
    list1.splice(insert_pos, list2);
    print_list(list1);
    print_list(list2);
    ASSERT_EQ(list1.size(), 4);
    ASSERT_EQ(list2.size(), 0);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 2 3 4 \n");
}

TEST(ListTest, Splice2) {
    List<int> list1 = {1, 2, 3, 4};
    List<int> list2 = {5, 6, 7, 8, 9, 10};
    auto insert_pos = list1.begin();
    std::advance(insert_pos, 1);
    list1.splice(insert_pos, list2, list2.begin());
    ASSERT_EQ(list1.size(), 5);
    ASSERT_EQ(list2.size(), 5);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 5 2 3 4 \n");
    ASSERT_LOGS_STDOUT(print_list(list2), "6 7 8 9 10 \n");
}

TEST(ListTest, Splice3_1) {
    List<int> list1 = {1, 2, 3, 4};
    List<int> list2 = {5, 6, 7, 8, 9, 10};
    auto insert_pos = list1.begin();
    std::advance(insert_pos, 1);

    auto list2_begin = list2.begin();
    auto list2_end = list2.end();
    std::advance(list2_end, -2);
    list1.splice(insert_pos, list2, list2_begin, list2_end);
    ASSERT_EQ(list1.size(), 8);
    ASSERT_EQ(list2.size(), 2);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 5 6 7 8 2 3 4 \n");
    ASSERT_LOGS_STDOUT(print_list(list2), "9 10 \n");
}

TEST(ListTest, Splice3_2) {
    List<int> list1 = {1, 2, 3, 4};
    List<int> list2 = {5, 6, 7, 8, 9, 10};
    auto insert_pos = list1.begin();
    std::advance(insert_pos, 1);

    auto list2_begin = list2.begin();
    auto list2_end = list2.end();
    list1.splice(insert_pos, list2, list2_begin, list2_end);
    ASSERT_EQ(list1.size(), 10);
    ASSERT_EQ(list2.size(), 0);
    ASSERT_LOGS_STDOUT(print_list(list1), "1 5 6 7 8 9 10 2 3 4 \n");
}

TEST(ListTest, SpliceSelf) {
    List<int> list1 = {5, 6, 7, 8, 9, 10, 11};
    auto insert_pos = list1.begin();
    std::advance(insert_pos, 1);

    auto list1_begin = list1.begin();
    std::advance(list1_begin, 3);
    auto list1_end = list1.end();
    std::advance(list1_end, -2);
    list1.splice(insert_pos, list1, list1_begin, list1_end);
    ASSERT_EQ(list1.size(), 7);

    ASSERT_LOGS_STDOUT(print_list(list1), "5 8 9 6 7 10 11 \n");
}

TEST(ListTest, Reverse) {
    List<int> nums{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    nums.reverse();
    ASSERT_LOGS_STDOUT(print_list(nums), "10 9 8 7 6 5 4 3 2 1 \n");
}

TEST(ListTest, Unique) {
    List<int> nums{1, 2, 2, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10};
    nums.unique();
    ASSERT_LOGS_STDOUT(print_list(nums), "1 2 3 4 5 6 7 8 9 10 \n");
}

// TEST(ListTest, Sort) {
//     List<int> nums{1, 2, 3, 11, 7, 6, 9, 4, 8, 5, 10};
//     nums.sort();
//     ASSERT_LOGS_STDOUT(print_list(nums), "1 2 3 4 5 6 7 8 9 10 11 \n");
// }

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}