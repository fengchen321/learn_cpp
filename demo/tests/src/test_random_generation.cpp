#include "gtest_prompt.h"
#include "random_generation.h"
#include <ctime>
#include <random>
#include <algorithm>
#include <thread>
#include <iostream>
#include <numeric>
#include <map>
#include <sstream>

TEST(RandomGeneration, CRandom) {
    srand(time(NULL));
    int num = rand() % 10;
    float f_num = (float)rand() / RAND_MAX;
    EXPECT_IN_RANGE(num, 0, 9);
    EXPECT_IN_RANGE(f_num, 0.0f, 1.0f)
    // printf("int: %d, float: %f\n", num, f_num);
}

TEST(RandomGeneration, CppRandom) {
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 9);
    int num = dis(gen);
    EXPECT_EQ(0, dis.min());
    EXPECT_EQ(9, dis.max());
    EXPECT_IN_RANGE(num, 0, 9);
    // printf("%d\n", num);

    std::uniform_real_distribution<float> f_dis(0, 1);
    float f_num = f_dis(gen);
    EXPECT_EQ(0, f_dis.min());
    EXPECT_EQ(1, f_dis.max());
    EXPECT_IN_RANGE(f_num, 0.0f, 1.0f);
    // printf("%f\n", f_num);

}

template<typename Generator>
class RandomTest : public ::testing::Test {
protected:
    Generator gen{std::random_device{}()};
};

// 生成器范围测试
template<typename Generator>
class RangeTest : public RandomTest<Generator> {
};

// 定义 RangeTest 测试套件的模板实例化
TYPED_TEST_SUITE_P(RangeTest);

TYPED_TEST_P(RangeTest, GeneratesNumbersInRange) {
    std::uniform_int_distribution<int> dis(0, 100);
    std::uniform_real_distribution<float> f_dis(0, 1);

    for (size_t i = 0; i < 10; i++) {
        EXPECT_IN_RANGE(dis(this->gen), 0, 100);
        EXPECT_IN_RANGE(f_dis(this->gen), 0.0f, 1.0f);
    }

    std::vector<std::string> choices = {"apple", "banana", "cherry"};
    std::uniform_int_distribution<int> choice_dis(0, choices.size() - 1);
    for (size_t i = 0; i < 10; i++) {
        EXPECT_IN_RANGE(choice_dis(this->gen), 0, choices.size() - 1);
    }
}

// 相同种子产生相同序列
TYPED_TEST_SUITE_P(RandomTest);

TYPED_TEST_P(RandomTest, SameSeed) {
    TypeParam gen1{12345};
    TypeParam gen1_copy = gen1;

    for (int i = 0; i < 1000; ++i) {
        EXPECT_EQ(gen1(), gen1_copy());
    }
}

// 不同种子产生不同序列
TYPED_TEST_P(RandomTest, DifferentSeeds) {
    TypeParam gen1{12345};
    TypeParam gen2{54321};
    bool sequences_differ = false;
    for (int i = 0; i < 1000; ++i) {
        if (gen1() != gen2()) {
            sequences_differ = true;
            break;
        }
    }
    EXPECT_TRUE(sequences_differ);
}

REGISTER_TYPED_TEST_SUITE_P(RangeTest, GeneratesNumbersInRange); // 注册 RangeTest 测试套件中的测试用例
REGISTER_TYPED_TEST_SUITE_P(RandomTest, SameSeed, DifferentSeeds); // 注册 RandomTest 测试套件中的测试用例

using MyGenerators = ::testing::Types<std::mt19937, xorshift32, wangshash>; // 定义测试套件中的生成器类型
INSTANTIATE_TYPED_TEST_SUITE_P(MyGeneratorsTests, RangeTest, MyGenerators);  // 实例化 RangeTest 测试套件
INSTANTIATE_TYPED_TEST_SUITE_P(MyGeneratorsTests, RandomTest, MyGenerators); // 实例化 RandomTest 测试套件

TEST(WANGSHash, GeneratesNumbers) {
    size_t within_range_count = 0;
    size_t total_samples = 1000;
    #pragma omp parallel for
    for (size_t i = 0; i < total_samples; i++) {
        wangshash gen(i);
        std::normal_distribution<float> norm_dis(0, 1); // 均值为0，标准差为1的正态分布
        float sample = norm_dis(gen);
        if (sample >= -3.0f && sample <= 3.0f) { // 0.99759的样本在[-3, 3]范围内
            within_range_count++;
        }
        // printf("%f\n", norm_dis(gen));
    }
    float proportion_within_range = static_cast<float>(within_range_count) / total_samples;
    EXPECT_GT(proportion_within_range, 0.99f);
}

TEST(WANGSHash, GenerateSample) {
    std::vector<int> a(100);
    std::generate(a.begin(), a.end(), 
        [gen = wangshash(0), dis = std::uniform_int_distribution<int>(0, 100)]
        () mutable {
            return dis(gen);
        });
    for (auto i : a) {
        EXPECT_IN_RANGE(i, 0, 100);
        // printf("%d\n", i);
    }
}

TEST(WANGSHash, GenerateSample2) {
    std::vector<int> b;
    b.reserve(100);
    std::generate_n(std::back_inserter(b), 100, 
        [gen = wangshash(0), dis = std::uniform_int_distribution<int>(0, 100)]
        () mutable {
            return dis(gen);
        });
    for (auto i : b) {
        EXPECT_IN_RANGE(i, 0, 100);
        // printf("%d\n", i);
    }
}

TEST(WANGSHash, Shuffle) {
    std::vector<int> original(10);
    std::iota(original.begin(), original.end(), 0);

    std::vector<int> shuffled = original;
     // std::mt19937 gen;
    // xorshift32 gen;
    wangshash gen;
    std::shuffle(shuffled.begin(), shuffled.end(), gen);

    bool is_different = false;
    for (size_t i = 0; i < original.size(); ++i) {
        if (original[i] != shuffled[i]) {
            is_different = true;
            break;
        }
    }

    EXPECT_TRUE(is_different) << "The shuffle operation did not change the order of elements.";
}

TEST(RandomTest, Simulation) {
    std::vector<float> prob = {0.45f, 0.25f, 0.15f, 0.1f, 0.05f};
    std::vector<float> expected_prob_scanned = {0.45f, 0.7f, 0.85f, 0.95f, 1.0f}; // prob的前缀和 

    // std::vector<float> prob_scanned(5);
    // std::inclusive_scan(prob.begin(), prob.end(), prob_scanned.begin());

    std::vector<float> prob_scanned;
    std::inclusive_scan(prob.begin(), prob.end(), std::back_inserter(prob_scanned));
    for (size_t i = 0; i < prob_scanned.size(); i++) {
        ASSERT_NEAR(expected_prob_scanned[i], prob_scanned[i], 1e-6) << "Prefix sum mismatch at index " << i;
        // printf("%f\n", prob_scanned[i]);
    }
    std::vector<std::string> face = {"Support", "Tank", "Mage", "Assassin", "Shooter"};
    std::mt19937 gen{std::random_device{}()};
    std::uniform_real_distribution<float> dis(0, 1);
    auto gen_face = [&]() -> std::string{
        float r = dis(gen);
        // prob_scanned 是前缀和值，递增，可以二分查找进行优化 O(n)-> O(logn)
        auto it = std::lower_bound(prob_scanned.begin(), prob_scanned.end(), r);
        if (it == prob_scanned.end()) { return ""; }
        return face[it - prob_scanned.begin()];
        /*
        for (size_t i = 0; i < prob_scanned.size(); i++) {
            if (r < prob_scanned[i]) {
                return face[i];
            }
        }
        return "";
        */
    };
    std::map<std::string, int> counts;
    const int total_samples = 1000000;
    for (size_t i = 0; i < total_samples; i++) {
        std::string result = gen_face();
        ASSERT_NE(result, ""); // Ensure result is not empty
        ASSERT_TRUE(std::find(face.begin(), face.end(), result) != face.end()) << "Unexpected result: " << result;
        // printf("%s\n", result.c_str());
        counts[result]++;
    }
    // auto it = counts.begin();
    // while (it != counts.end()) {
    //     if (std::next(it) != counts.end()) {
    //         printf("%s: %d, ", it->first.c_str(), it->second);
    //     }
    //     else {
    //         printf("%s: %d", it->first.c_str(), it->second);
    //     }
    //     ++it;
    // }
    // printf("\n");

    // std::ostringstream oss;
    // for (auto it = counts.begin(); it != counts.end(); ++it) {
    //     if (it != counts.begin()) {
    //         oss << ", ";
    //     }
    //     oss << it->first << ": " << it->second;
    // }
    // std::cout << oss.str() << std::endl;
    

    std::ostringstream oss;
    oss << "total samples: " << total_samples << "\n";
    for (size_t i = 0; i < face.size(); ++i) {
        if (i != 0) {
            oss << ", ";
        }
        oss << face[i] << ": " << counts[face[i]];
    }
    std::cout << oss.str() << std::endl;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}