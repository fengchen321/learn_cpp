#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <random>
#include <algorithm>
#include <thread>
#include <iostream>
#include <numeric>

void c_random() {
    // C random
    srand(time(NULL));
    int num = rand() % 10;
    float f_num = (float)rand() / RAND_MAX;
    printf("int: %d, float: %f\n", num, f_num);
}

void cpp_random() {
    // C++ random
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 9);
    int num = dis(gen);
    printf("min: %d, max: %d\n", dis.min(), dis.max());
    printf("%d\n", num);

    std::uniform_real_distribution<float> f_dis(0, 1);
    float f_num = f_dis(gen);
    printf("min: %f, max: %f\n", f_dis.min(), f_dis.max());
    printf("%f\n", f_num);
}

void multi_thread_c_random() {
    std::thread t1([]() {
        printf("t1: %d\n", rand());
        printf("t1: %d\n", rand());
    });
    std::thread t2([]() {
        printf("t2: %d\n", rand());
        printf("t2: %d\n", rand());
    });
    t1.join();
    t2.join();
}
void multi_thread_cpp_random() {
    std::thread t1([&]() {
        std::mt19937 gen1(1);
        printf("t1: %ld\n", gen1());
        printf("t1: %ld\n", gen1());
    });
    std::thread t2([&]() {
        std::mt19937 gen2;
        printf("t1: %ld\n", gen2());
        printf("t1: %ld\n", gen2());
    });
    t1.join();
    t2.join();
}
/*
* xorshift32 random number generator
*/
struct xorshift32 {
    uint32_t a;
    explicit xorshift32(size_t seed = 0) : a(static_cast<uint32_t>(seed + 1)) {}
    using result_type = uint32_t;
    /* The state must be initialized to non-zero */
    constexpr uint32_t operator()() noexcept{
        /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
        uint32_t x = a;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return a = x;
    }
    
    static constexpr uint32_t min() noexcept {
        return 1;
    }

    static constexpr uint32_t max() noexcept {
        return UINT32_MAX;
    }
};

void xorshift32_random() {
    xorshift32 gen{std::random_device{}()};
    std::uniform_int_distribution<int> dis(0, 100);     // 生成0到100之间的随机整数
    std::uniform_real_distribution<float> f_dis(0, 1); // 生成0到1之间的随机浮点数
    for (size_t i = 0; i < 10; i++) {
        printf("%u\n", dis(gen));
        printf("%f\n", f_dis(gen));
    }

    std::vector<std::string> choices = {"apple", "banana", "cherry"};
    std::uniform_int_distribution<int> choice_dis(0, choices.size() - 1);
    for (size_t i = 0; i < 10; i++) {
        printf("%s\n", choices[choice_dis(gen)].c_str());
    }
}
/*
* wangshash random number generator
*/
struct wangshash {
    uint32_t a;
    explicit wangshash(size_t seed = 0) : a(static_cast<uint32_t>(seed)) {}
    using result_type = uint32_t;
    constexpr uint32_t operator()() noexcept {
        uint32_t x = a;
        x = (x ^ 61) ^ (x >> 16);
        x *= 9;
        x = x ^ (x >> 4);
        x *= 0x27d4eb2d;
        x = x ^ (x >> 15);
        return a = x;
    }

    static constexpr uint32_t min() noexcept {
        return 0;
    }

    static constexpr uint32_t max() noexcept {
        return UINT32_MAX;
    }
};

void wangshash_random() {
    #pragma omp parallel for
    for (size_t i = 0; i < 100; i++) {
        wangshash gen(i);
        std::normal_distribution<float> norm_dis(0, 1); // 均值为0，标准差为1的正态分布
        printf("%f\n", norm_dis(gen));
    }

    std::vector<int> a(100);
    std::generate(a.begin(), a.end(), 
        [gen = wangshash(0), dis = std::uniform_int_distribution<int>(0, 100)]
        () mutable {
            return dis(gen);
        });
    for (auto i : a) {
        printf("%d\n", i);
    }

    std::vector<int> b;
    b.reserve(100);
    std::generate_n(std::back_inserter(b), 100, 
        [gen = wangshash(0), dis = std::uniform_int_distribution<int>(0, 100)]
        () mutable {
            return dis(gen);
        });
    for (auto i : b) {
        printf("%d\n", i);
    }
}

void shuffle_cpp_random() {
    std::vector<int> c;
    std::iota(c.begin(), c.end(), 0);
    // std::mt19937 gen;
    // xorshift32 gen;
    wangshash gen;
    std::shuffle(c.begin(), c.end(), gen); // 随机洗牌
    for (auto i : c) {
        printf("%d\n", i);
    }
}
#include <map>
#include <sstream>
void simulation() {
    std::vector<float> prob = {0.45f, 0.25f, 0.15f, 0.1f, 0.05f};
    // std::vector<float> prob_scanned = {0.45f, 0.7f, 0.85f, 0.95f, 1.0f}; // prob的前缀和

    // std::vector<float> prob_scanned(5);
    // std::inclusive_scan(prob.begin(), prob.end(), prob_scanned.begin());

    std::vector<float> prob_scanned;
    std::inclusive_scan(prob.begin(), prob.end(), std::back_inserter(prob_scanned));
    for (size_t i = 0; i < prob_scanned.size(); i++) {
        printf("%f\n", prob_scanned[i]);
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
    for (size_t i = 0; i < 100; i++) {
        std::string result = gen_face();
        printf("%s\n", result.c_str());
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
    for (size_t i = 0; i < face.size(); ++i) {
        if (i != 0) {
            oss << ", ";
        }
        oss << face[i] << ": " << counts[face[i]];
    }
    std::cout << oss.str() << std::endl;
}

int main() {
    // c_random();
    // cpp_random();
    // multi_thread_c_random();
    // multi_thread_cpp_random();
    // xorshift32_random();
    // wangshash_random();
    // shuffle_cpp_random();
    simulation();
    return 0;
}