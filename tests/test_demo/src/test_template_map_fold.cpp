#include "gtest_prompt.h"
#include "template_map_fold.h"
#include <iostream>
#include <list>
#include <vector>
#include <array>

TEST(FoldMapTest, mapf_abs) {
    auto vnums = std::vector<int> {0, 2, -3, 5, -1, 6, 8, -4, 9};
    auto r = mapf([](int const i) {return std::abs(i);}, vnums);
    EXPECT_EQ(r, std::vector<int>({0, 2, 3, 5, 1, 6, 8, 4, 9}));
}

TEST(FoldMapTest, mapf_squr) {
    auto lnums = std::list<int> {1, 2, 3, 4, 5};
    auto l = mapf([](int const i){return i * i;}, lnums);
    EXPECT_EQ(l, std::list<int>({1, 4, 9, 16, 25}));
}

template <class T = double>
struct fround {
    typename std::enable_if_t<std::is_floating_point_v<T>, T>
    operator()(const T& value) const {
        return std::round(value);
    }
};

TEST(FoldMapTest, mapf_float) {
    auto amounts = std::array<double, 5> {10.42, 2.50, 100.0, 23.75, 12.99};
    auto a = mapf(fround<>(), amounts);

    auto b = std::array<double, 5> {10.0, 3.0, 100.0, 24.0, 13.0};
    for (int i = 0; i < a.size(); ++i) {
        EXPECT_DOUBLE_EQ(a[i], b[i]) << "array a and b differ at index " << i;
    }  
}

TEST(FoldMapTest, mapf_words) {
    auto words = std::map<std::string, int> {{"one", 1},{"two", 2},{"three", 3}}; 
    auto m = mapf([](std::pair<std::string, int> const kvp){return std::make_pair(mapf(toupper, kvp.first), kvp.second);}, words);
    EXPECT_EQ(m, (std::map<std::string, int>{{"ONE", 1}, {"TWO", 2}, {"THREE", 3}}));
}

TEST(FoldMapTest, mapf_reduce_level) {
    auto priorities = std::queue<int>();
    priorities.push(10);
    priorities.push(20);
    priorities.push(30);
    priorities.push(40);
    priorities.push(50);
    auto p = mapf([](int const i) {return i > 30 ? 2 : 1;}, priorities);
    std::vector<int> result;
    while(!p.empty()) {
        result.push_back(p.front());
        p.pop();
    }
    EXPECT_EQ(result, std::vector<int>({1, 1, 1, 2, 2}));
}

TEST(FoldMapTest, fold_sum) {
    auto vnums = std::vector<int> {0, 2, -3, 5, -1, 6, 8, -4, 9};
    auto s1 = foldl([](const int s, const int n) {return s + n;}, vnums, 0);
    EXPECT_EQ(s1, 22);
    auto s2 = foldl(std::plus<>(), vnums, 0);
    EXPECT_EQ(s2, 22);
    auto s3 = foldr([](const int s, const int n) {return s + n;}, vnums, 0);
    EXPECT_EQ(s3, 22);
    auto s4 = foldr(std::plus<>(), vnums, 0);
    EXPECT_EQ(s4, 22);
}

TEST(FoldMapTest, fold_sum_1) {
    auto s1 = foldx(std::plus<>(), 1, 2, 3, 4, 5);
    EXPECT_EQ(s1, 15);

    auto vnums = std::vector<int> {1, 2, 3, 4, 5};
    auto s2 = foldl(std::plus<>(), vnums, 0);
    EXPECT_EQ(s1, s1);
}


TEST(FoldMapTest, fold_pipefunc) {
    auto vnums = std::vector<int> {0, 2, -3, 5, -1, 6, 8, -4, 9};
    auto s1 = foldl(std::plus<>(),
        mapf([](int const i) {return i * 2;}, 
            mapf([](int const i){return std::abs(i);},
            vnums)),
            0);
    EXPECT_EQ(s1, 76);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}