#include <gtest/gtest.h>
#include <vector>
#include <numeric>
#include <optional>
#include <memory>
#include <list>
#include <mutex>
#include <thread>
#include <future>

int sum_0(std::vector<int> v) {
    int res = 0;
    for (size_t i = 0; i < v.size(); i++) {
        res = res + v[i];
    }
    return res;
}

int product_0(std::vector<int> v) {
    int res = 1;
    for (size_t i = 0; i < v.size(); i++) {
        res = res * v[i];
    }
    return res;
}

TEST(DesignPatternTest, virtualTest_0) {
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 1);
    ASSERT_EQ(sum_0(vec), 55);
    ASSERT_EQ(product_0(vec), 3628800);
}

/*
* 策略模式：定义一系列算法类，将每一个算法封装起来，并让它们可以相互替换
* 客户端代码（reduce 函数）能够使用不同的实现而无需修改自身公共部分 reduce 里实现
*/
struct Reducer {
    virtual int init() = 0;
    virtual int accumulate(int a, int b) = 0;
    virtual ~Reducer() = default;
};

int reduce(std::vector<int> v, std::unique_ptr<Reducer> reducer) {
    int res = reducer->init();
    for (int i = 0; i < v.size(); i++) {
        res = reducer->accumulate(res, v[i]);
    }
    return res;
}

struct SumReducer : Reducer {
    int init() override {
        return 0;
    }

    int accumulate(int a, int b) override {
        return a + b;
    }
};

struct ProductReducer : Reducer {
    int init() override {
        return 1;
    }

    int accumulate(int a, int b) override {
        return a * b;
    }
};

TEST(DesignPatternTest, virtualTest_1) {
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 1);
    ASSERT_EQ(reduce(vec, std::make_unique<SumReducer>()), 55);
    ASSERT_EQ(reduce(vec, std::make_unique<ProductReducer>()), 3628800);
}

/*
* 多重策略：聚合操作策略 和 输入源策略
* Inputer 负责告诉 reduce_2 函数如何读取数据，Reducer 负责告诉 reduce_2 函数如何计算数据
* 依赖倒置原则：
* * 高层模块（reduce_2）不应该依赖于低层模块（具体的 Inputer 和 Reducer 实现），
* * 抽象（Inputer 和 Reducer 类）不应该依赖于细节（具体实现）
*/
struct Inputer {
    virtual std::optional<int> fetch() = 0;
    virtual ~Inputer() = default;
};

int reduce_2(std::unique_ptr<Inputer> inputer, std::unique_ptr<Reducer> reducer) {
    int res = reducer->init();
    while (auto tmp = inputer->fetch()) {
        res = reducer->accumulate(res, *tmp);
    }
    return res;
}

struct CinInputer : Inputer {
    std::optional<int> fetch() override {
        int tmp;
        std::cin >> tmp;
        if (tmp == -1)
            return std::nullopt;
        return tmp;
    }
};

struct VectorInputer : Inputer {
    std::vector<int> v;
    int pos = 0;

    VectorInputer(std::vector<int> v) : v(v) {}

    std::optional<int> fetch() override {
        if (pos == v.size())
            return std::nullopt;
        return v[pos++];
    }
};

TEST(DesignPatternTest, virtualTest_2) {
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 1);
    ASSERT_EQ(reduce_2(std::make_unique<VectorInputer>(vec), std::make_unique<SumReducer>()), 55);
    ASSERT_EQ(reduce_2(std::make_unique<VectorInputer>(vec), std::make_unique<ProductReducer>()), 3628800);
}

TEST(DesignPatternTest, virtualTest_2_cin) {
    std::istringstream input("1 2 3 4 5 -1");
    std::cin.rdbuf(input.rdbuf()); // 重定向 cin 的输入
    ASSERT_EQ(reduce_2(std::make_unique<CinInputer>(), std::make_unique<SumReducer>()), 15);

    std::istringstream input2("1 2 3 4 5 -1"); 
    std::cin.rdbuf(input2.rdbuf());
    ASSERT_EQ(reduce_2(std::make_unique<CinInputer>(), std::make_unique<ProductReducer>()), 120);

    // 恢复 cin 的缓冲区
    std::cin.clear();
    std::cin.rdbuf(nullptr);
}

/*
* 适配器模式：允许在不修改原有类的情况下扩展其功能，使其能够与其他接口兼容
*/
// 问题：CinInputer 想读取到 0 截止，而不是 -1 呢？
// 答：StopInputerAdapter 负责处理截断问题，CinInputer 只是负责读取 cin 输入
struct StopInputerAdapter : Inputer {
    std::unique_ptr<Inputer> inputer;
    int stopMark;

    StopInputerAdapter(std::unique_ptr<Inputer> inputer, int stopMark) 
        : inputer(std::move(inputer)), stopMark(stopMark) {}
    
    std::optional<int> fetch() override {
        auto tmp = inputer->fetch();
        if (!tmp.has_value() || tmp.value() == stopMark) {
            return std::nullopt;
        }
        return tmp;
    }
};

TEST(DesignPatternTest, virtualTest_3_vec) {
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 1);
    ASSERT_EQ(reduce_2(std::make_unique<VectorInputer>(vec), std::make_unique<SumReducer>()), 55);
    ASSERT_EQ(reduce_2(std::make_unique<StopInputerAdapter>(std::make_unique<VectorInputer>(vec), 10), std::make_unique<SumReducer>()), 45); // 从 vec 读到 10 为止

    ASSERT_EQ(reduce_2(std::make_unique<VectorInputer>(vec), std::make_unique<ProductReducer>()), 3628800);
    ASSERT_EQ(reduce_2(std::make_unique<StopInputerAdapter>(std::make_unique<VectorInputer>(vec), 10), std::make_unique<ProductReducer>()), 362880);
}

TEST(DesignPatternTest, virtualTest_3_cin) {
    std::istringstream input("1 2 3 4 5 -1");
    std::cin.rdbuf(input.rdbuf()); // 重定向 cin 的输入
    ASSERT_EQ(reduce_2(std::make_unique<StopInputerAdapter>(std::make_unique<CinInputer>(), 5), std::make_unique<SumReducer>()), 10); // 从 cin 读到 5 为止

    std::istringstream input2("1 2 3 4 5 -1"); 
    std::cin.rdbuf(input2.rdbuf());
    ASSERT_EQ(reduce_2(std::make_unique<StopInputerAdapter>(std::make_unique<CinInputer>(), 5), std::make_unique<ProductReducer>()), 24);

    // 恢复 cin 的缓冲区
    std::cin.clear();
    std::cin.rdbuf(nullptr);
}

// 实现过滤输入
struct FilterStrategy {
    virtual bool shouldPass(int value) = 0;  // 返回 true 表示该值应该被保留
    virtual ~FilterStrategy() = default;
};

struct FilterStrategyAbove : FilterStrategy { // 大于一定值（threshold）才能通过
    int threshold;
    FilterStrategyAbove(int threshold) : threshold(threshold) {}
    bool shouldPass(int value) override {
        return value > threshold;
    }
};

struct FilterStrategyBelow : FilterStrategy { // 小于一定值（threshold）才能通过
    int threshold;
    FilterStrategyBelow(int threshold) : threshold(threshold) {}
    bool shouldPass(int value) override {
        return value < threshold;
    }
};

struct FilterStrategyAnd : FilterStrategy {  // 要求 a 和 b 两个过滤策略都为 true，才能通过
    std::unique_ptr<FilterStrategy> a;
    std::unique_ptr<FilterStrategy> b;

    FilterStrategyAnd(std::unique_ptr<FilterStrategy> a, std::unique_ptr<FilterStrategy> b)
        : a(std::move(a)), b(std::move(b)) {}

    bool shouldPass(int value) override {
        return a->shouldPass(value) && b->shouldPass(value);
    }
};

struct FilterInputerAdapter : Inputer {
    std::unique_ptr<Inputer> inputer;
    std::unique_ptr<FilterStrategy> strategy;

    FilterInputerAdapter(std::unique_ptr<Inputer> inputer, std::unique_ptr<FilterStrategy> strategy)
        : inputer(std::move(inputer)), strategy(std::move(strategy)) {}

    std::optional<int> fetch() override {
        while (true) {
            auto tmp = inputer->fetch();
            if (!tmp.has_value()) {
                return std::nullopt;
            }
            if (strategy->shouldPass(*tmp)) {
                return tmp;
            }
        }
    }
};

TEST(DesignPatternTest, virtualTest_4_vec) {
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 1);
    ASSERT_EQ(reduce_2(
        std::make_unique<FilterInputerAdapter>(
            std::make_unique<StopInputerAdapter>(
                std::make_unique<VectorInputer>(vec),
                10
            ),
            std::make_unique<FilterStrategyAnd>(
                std::make_unique<FilterStrategyAbove>(3),
                std::make_unique<FilterStrategyBelow>(8)
            )
        ),
        std::make_unique<SumReducer>()),
    22);
}

/*
* 跨接口的适配器
*/
// 第三方库poost
namespace poost {
struct PoostInputer {
    virtual bool hasNext() = 0;
    virtual int getNext() = 0;
    virtual ~PoostInputer() = default;
};

struct VectorPoostInputer : PoostInputer {
    std::vector<int> v;
    int pos = 0;

    VectorPoostInputer(std::vector<int> v) : v(v) {}

    bool hasNext() override {
        return pos < v.size();
    }

    int getNext() override {
        return v[pos++]; 
    }
};

std::unique_ptr<PoostInputer> getVectorInput(const std::vector<int>& inputVector) {
    return std::make_unique<VectorPoostInputer>(inputVector);
}

} // poost

struct PoostInputerAdapter : Inputer {
    std::unique_ptr<poost::PoostInputer> poostIn;

    PoostInputerAdapter(std::unique_ptr<poost::PoostInputer> poostIn)
        : poostIn(std::move(poostIn)) {}

    std::optional<int> fetch() override {
        if (poostIn->hasNext()) {
            return poostIn->getNext();
        } else {
            return std::nullopt;
        }
    }
};

TEST(DesignPatternTest, virtualTest_5_vec) {
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 1);
    auto poostStdIn = poost::getVectorInput(vec);
    ASSERT_EQ(reduce_2(std::make_unique<PoostInputerAdapter>(std::move(poostStdIn)), std::make_unique<SumReducer>()), 55);
}

/*
* 工厂方法
*/
struct Bullet {
    virtual void explode() = 0;
    virtual ~Bullet() = default;
};

struct AK47Bullet: Bullet {
    void explode() override {
        puts("---");
    }
};

struct MagicBullet: Bullet {
    void explode() override {
        puts("***");
    }
};

struct Gun {
    virtual std::unique_ptr<Bullet> shoot() = 0;
    virtual ~Gun() = default;
};

// 利用模板自动为不同的子弹类型批量定义工厂
template<class B>
struct GunWithBullet: Gun {
    static_assert(std::is_base_of<Bullet, B>::value, "B must be a subclass of Bullet");

    std::unique_ptr<Bullet> shoot() override {
        return std::make_unique<B>();
    }
};

void player(std::unique_ptr<Gun> gun) {
    for (size_t i = 0; i < 10; i++) {
        std::unique_ptr<Bullet> bullet = gun->shoot();
        bullet->explode();
    }
}

std::unique_ptr<Gun> getGun(std::string name) {
    if (name == "AK47") {
        return std::make_unique<GunWithBullet<AK47Bullet>>();
    } else if (name == "Magic") {
        return std::make_unique<GunWithBullet<MagicBullet>>();
    } else {
        throw std::runtime_error("unkown Gun");
    }
}

TEST(DesignPatternTest, FactoryTest) {
    player(std::make_unique<GunWithBullet<AK47Bullet>>());
    player(std::make_unique<GunWithBullet<MagicBullet>>());

    player(getGun("AK47"));
    player(getGun("Magic"));
}

/*
* 平均值的函数 average:需要允许 Reducer 的 init() 返回 “任意数量的状态变量”！ 之前只能返回单个 int
*/
struct ReducerState {
    virtual void accumulate(int val) = 0;
    virtual double result() = 0;
    virtual void merge(const ReducerState& other) = 0;
};

struct Reducer1 {
    virtual std::unique_ptr<ReducerState> init() = 0;
};

struct SumReducerState : ReducerState {
    long long res;
    SumReducerState() : res(0) {}

    void accumulate(int val) override {
        res += val;
    }

    double result() override {
        return static_cast<double>(res);
    }

    void merge(const ReducerState& other) override {
        const SumReducerState* other_sum = dynamic_cast<const SumReducerState*>(&other);
        if (other_sum) {
            res += other_sum->res;
        }
    }
};

struct ProductReducerState : ReducerState {
    long long res;
    ProductReducerState() : res(1) {}

    void accumulate(int val) override {
        res *= val;
    }

    double result() override {
        return static_cast<double>(res);
    }

    void merge(const ReducerState& other) override {
        const ProductReducerState* other_product = dynamic_cast<const ProductReducerState*>(&other);
        if (other_product) {
            res *= other_product->res;
        }
    }
};

struct AverageReducerState : ReducerState {
    long long res;
    int count;
    AverageReducerState() : res(0), count(0) {}

    void accumulate(int val) override {
        res += val;
        count++;
    }

    double result() override {
        return count > 0 ? res / static_cast<double>(count) : 0.0;
    }

    void merge(const ReducerState& other) override {
        const AverageReducerState* other_avg = dynamic_cast<const AverageReducerState*>(&other);
        if (other_avg) {
            res += other_avg->res;
            count += other_avg->count;
        }
    }
};

template<class R>
struct RudecerwithState: Reducer1 {
    static_assert(std::is_base_of<ReducerState, R>::value, "R must be a subclass of ReducerState");

    std::unique_ptr<ReducerState> init() override {
        return std::make_unique<R>();
    }
};

// struct SumReducer1 : Reducer1 {
//     std::unique_ptr<ReducerState> init() override {
//         return std::make_unique<SumReducerState>();
//     }
// };

// struct ProductReducer1 : Reducer1 {
//     std::unique_ptr<ReducerState> init() override {
//         return std::make_unique<ProductReducerState>();
//     }
// };

// struct AverageReducer1 : Reducer1 {
//     std::unique_ptr<ReducerState> init() override {
//         return std::make_unique<AverageReducerState>();
//     }
// };

double reduce_3(std::unique_ptr<Inputer> inputer, std::unique_ptr<Reducer1> reducer) {
    std::unique_ptr<ReducerState> state = reducer->init();
    while (auto val = inputer->fetch()) {
        state->accumulate(*val);
    }
    return state->result();
}

TEST(DesignPatternTest, virtualTest_6_vec) {
    std::vector<int> vec(10);
    std::iota(vec.begin(), vec.end(), 1);
    ASSERT_DOUBLE_EQ(reduce_3(std::make_unique<VectorInputer>(vec), std::make_unique<RudecerwithState<SumReducerState>>()), 55.0);
    ASSERT_DOUBLE_EQ(reduce_3(std::make_unique<StopInputerAdapter>(std::make_unique<VectorInputer>(vec), 10.0), 
        std::make_unique<RudecerwithState<SumReducerState>>()), 45);  // std::make_unique<SumReducer1>()

    ASSERT_DOUBLE_EQ(reduce_3(std::make_unique<VectorInputer>(vec), std::make_unique<RudecerwithState<ProductReducerState>>()), 3628800.0);
    ASSERT_DOUBLE_EQ(reduce_3(std::make_unique<StopInputerAdapter>(std::make_unique<VectorInputer>(vec), 10.0), 
        std::make_unique<RudecerwithState<ProductReducerState>>()), 362880.0);

    ASSERT_DOUBLE_EQ(reduce_3(std::make_unique<VectorInputer>(vec), std::make_unique<RudecerwithState<AverageReducerState>>()), 5.5);
    ASSERT_DOUBLE_EQ(reduce_3(std::make_unique<StopInputerAdapter>(std::make_unique<VectorInputer>(vec), 10.0), 
        std::make_unique<RudecerwithState<AverageReducerState>>()), 5.0);
}

TEST(DesignPatternTest, virtualTest_6_cin) {
    std::istringstream input("1 2 3 4 5 -1");
    std::cin.rdbuf(input.rdbuf()); // 重定向 cin 的输入
    ASSERT_DOUBLE_EQ(reduce_3(std::make_unique<CinInputer>(), std::make_unique<RudecerwithState<SumReducerState>>()), 15.0); 

    std::istringstream input2("1 2 3 4 5 -1"); 
    std::cin.rdbuf(input2.rdbuf());
    ASSERT_DOUBLE_EQ(reduce_3(std::make_unique<CinInputer>(), std::make_unique<RudecerwithState<ProductReducerState>>()), 120.0);

    std::istringstream input3("1 2 3 4 5 -1"); 
    std::cin.rdbuf(input3.rdbuf());
    ASSERT_DOUBLE_EQ(reduce_3(std::make_unique<CinInputer>(), std::make_unique<RudecerwithState<AverageReducerState>>()), 3.0);

    // 恢复 cin 的缓冲区
    std::cin.clear();
    std::cin.rdbuf(nullptr);
}
// https://godbolt.org/z/8GW3zqPrY
double reduce_4(std::unique_ptr<Inputer> inputer,std::unique_ptr<Reducer1> reducer) {
    std::list<std::unique_ptr<ReducerState>> local_states; // Store local states
    std::vector<int> chunk;          // Buffer for the current chunk
    std::mutex mutex;               // Protect local states with a mutex
    const size_t chunk_size = 64;   // Chunk size
    std::vector<std::future<void>> futures; // Store futures for asynchronous tasks

    // Process a chunk of data
    auto process_chunk = [&local_states, &mutex, &reducer](std::vector<int> chunk) {
        auto local_state = reducer->init();
        for (int value : chunk) {
            local_state->accumulate(value);
        }

        std::lock_guard<std::mutex> lock(mutex);
        local_states.emplace_back(std::move(local_state));
    };

    while (auto tmp = inputer->fetch()) {
        if (chunk.size() >= chunk_size) {
            futures.push_back(std::async(std::launch::async, process_chunk, std::move(chunk)));
            chunk.clear();
        }
        chunk.push_back(*tmp);
    }

    if (!chunk.empty()) {
        futures.push_back(std::async(std::launch::async, process_chunk, std::move(chunk)));
    }

     for (auto& future : futures) {
        future.get();
    }

    auto final_state = reducer->init();
    for (const auto& local_state : local_states) {
        // final_state->accumulate(local_state->result()); // 合并结果的时候应该要合并 ReduceSate 本身而不是合并 ReduceState 的结果
        final_state->merge(*local_state);
    }
    return final_state->result();
}

TEST(DesignPatternTest, virtualTest_7_vec) {
    double res = 0;
    std::vector<int> vec(66);
    std::iota(vec.begin(), vec.end(), 1);
    for (int i = 0; i < vec.size(); i++) {
        res += vec[i];
    }
    std::cout << res << " " << res / vec.size() << std::endl;
    ASSERT_DOUBLE_EQ(reduce_4(std::make_unique<VectorInputer>(vec), std::make_unique<RudecerwithState<SumReducerState>>()), res);
    ASSERT_DOUBLE_EQ(reduce_4(std::make_unique<VectorInputer>(vec), std::make_unique<RudecerwithState<AverageReducerState>>()), res / vec.size());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}