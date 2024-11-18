#include <vector>
#include <numeric>
#include <optional>
#include <memory>
#include <list>
#include <mutex>
#include <thread>
#include <future>
#include <typeindex>
#include "gtest_prompt.h"

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
        res = reducer->accumulate(res, tmp.value());
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
            if (strategy->shouldPass(tmp.value())) {
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
        chunk.push_back(tmp.value());
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

/*
* 单例模式 ： 饿汉模式，懒汉模式
*/

// 饿汉式
class Singlehungry {
private:
    Singlehungry() = default;
    Singlehungry(const Singlehungry&) = delete;
    Singlehungry& operator=(const Singlehungry&) = delete;
public:
    static std::shared_ptr<Singlehungry> GetInstance() {
        if (singleton == nullptr) {
            singleton = std::shared_ptr<Singlehungry>(new Singlehungry);
            // 不能std::make_shared，因为构造函数是私有的
        }
        return singleton;
    }
private:
    static std::shared_ptr<Singlehungry> singleton;
};
// 饿汉式初始化
std::shared_ptr<Singlehungry> Singlehungry::singleton = Singlehungry::GetInstance();

// 懒汉式
class Singletonlazy {
private:
    Singletonlazy() = default;
    Singletonlazy(const Singletonlazy&) = delete;
    Singletonlazy& operator=(const Singletonlazy&) = delete;
public:
    static std::shared_ptr<Singletonlazy> GetInstance() {
        if (singleton == nullptr) {
            std::lock_guard<std::mutex> lock(mutex);
            if (singleton == nullptr) {
                singleton = std::shared_ptr<Singletonlazy>(new Singletonlazy);
            }
        }
        return singleton;
}
private:
    static std::shared_ptr<Singletonlazy> singleton;
    static std::mutex mutex;
};
std::shared_ptr<Singletonlazy> Singletonlazy::singleton = nullptr;
std::mutex Singletonlazy::mutex;

// call_once
template <typename T>
class Singleton {
private:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>&) = delete;
public:
    static std::shared_ptr<T> GetInstance() {
        std::call_once(_flag, [&]() {
            singleton = std::shared_ptr<T>(new T);
        });
        return singleton;
    }
private:
    static std::shared_ptr<T> singleton;
    static std::once_flag _flag;
};

template <typename T>
std::shared_ptr<T> Singleton<T>::singleton = nullptr;

template <typename T>
std::once_flag Singleton<T>::_flag;

class TestClass {};

template <typename SingletonType>
void TestSingletonThreadSafe() {
    // const int numThreads = std::thread::hardware_concurrency();
    const int numThreads = 10;
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([](){
            auto instance = SingletonType::GetInstance();
            ASSERT_EQ(instance, SingletonType::GetInstance());
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

TEST(DesignPatternTest, Singleton_thread_safe) {
    TestSingletonThreadSafe<Singletonlazy>();
    TestSingletonThreadSafe<Singlehungry>();
    TestSingletonThreadSafe<Singleton<TestClass>>();
}

/*
* 模板模式 ： 共同的部分集中到一个基类，把不同的细节部分留给子类实现。
* 当一个对象涉及很多策略时，用策略模式；
* 当只需要一个策略，且需要用到基类的成员时，用模板模式。
*/

class Character {
protected: // 子类可以访问,外部不可访问
    virtual void draw() const = 0;
    virtual void move() const = 0;
public:
    void update() const {
        move();
        move();
        draw();
    }
};

class Player : public Character {
protected: 
    void draw() const override {
        puts(__PRETTY_FUNCTION__);
    }
    void move() const override {
        puts(__PRETTY_FUNCTION__);
    }
};
class Enemy : public Character {
protected: 
    void draw() const override {
        puts(__PRETTY_FUNCTION__);
    }
    void move() const override {
        puts(__PRETTY_FUNCTION__);
    }
};

class Game {
public:
    std::vector<std::shared_ptr<Character>> characters;
    void update() {
        for (auto& character : characters) {
            character->update();
        }
    }
};

TEST(DesignPatternTest, TemplatePattern) {
    Game game;
    game.characters.emplace_back(std::make_shared<Player>());
    game.characters.emplace_back(std::make_shared<Enemy>());
    game.update();
}

/*
* 状态模式：将不同状态的处理逻辑分离到不同的类中，用状态对象的虚函数来表示状态的处理逻辑。
*/
class Context;
class State {
public:
    virtual void update(std::shared_ptr<Context> context) = 0;
};
class ConcreteStateA : public State {
public:
    void update(std::shared_ptr<Context> context) override;
};

class ConcreteStateB : public State {
public:
    void update(std::shared_ptr<Context> context) override;
};

class Context : public std::enable_shared_from_this<Context> {
private:
    std::shared_ptr<State> state;
public:
    Context() : state(std::make_shared<ConcreteStateA>()) {}
    void setState(std::shared_ptr<State> state) {
        this->state = state;
    }
    void update() {
        state->update(shared_from_this());
    }
};

void ConcreteStateA::update(std::shared_ptr<Context> context) {
    puts(__PRETTY_FUNCTION__);
    if (true) { // 根据条件切换状态
        context->setState(std::make_shared<ConcreteStateB>());
    }
}

void ConcreteStateB::update(std::shared_ptr<Context> context) {
    puts(__PRETTY_FUNCTION__);
    if (true) {
        context->setState(std::make_shared<ConcreteStateA>());
    }
}

TEST(DesignPatternTest, StatePattern) {
    auto context = std::make_shared<Context>();
    context->setState(std::make_shared<ConcreteStateA>());
    context->update();
    context->setState(std::make_shared<ConcreteStateB>());
    context->update();
}

/*
* 原型模式：复制现有的对象，且新对象的属性和类型与原来相同。
* * 原型模式将对象的拷贝方法作为虚函数，返回一个虚接口的指针，避免了直接拷贝类型。
* * 但虚函数内部会调用子类真正的构造函数，实现深拷贝
* 为什么拷贝构造函数不行? 拷贝构造函数只能用于类型确定的情况，对于具有虚函数，可能具有额外成员的多态类型，会发生 object-slicing，导致拷贝出来的类型只是基类的部分，而不是完整的子类对象
* 为什么拷贝指针不行? 指针的拷贝是浅拷贝，指向的仍然是同一对象
*/

class Ball {
    virtual std::unique_ptr<Ball> clone() const = 0;
};

class RedBall  : public Ball {
public:
    std::unique_ptr<Ball> clone() const override {
        return std::make_unique<RedBall>(*this);
    }
};

class BlueBall  : public Ball {
public:
    std::unique_ptr<Ball> clone() const override {
        return std::make_unique<BlueBall>(*this);
    }
    int somedate; // 额外成员
};

TEST(DesignPatternTest, PrototypePattern) {
    auto redBall = std::make_unique<RedBall>();
    auto redBall2 = redBall->clone();
    ASSERT_NE(redBall2, nullptr);
    ASSERT_EQ(typeid(*redBall2), typeid(RedBall));
    EXPECT_NE(redBall2.get(), redBall.get());

    auto blueBall = std::make_unique<BlueBall>();
    blueBall->somedate = 42;
    auto blueBall2 = blueBall->clone();
    ASSERT_NE(redBall2, nullptr);
    ASSERT_EQ(typeid(*blueBall2), typeid(BlueBall));
    EXPECT_NE(blueBall2.get(), blueBall.get());

    auto blueBall2Ptr = dynamic_cast<BlueBall*>(blueBall2.get());
    EXPECT_EQ(blueBall2.get(), blueBall2Ptr);
    EXPECT_EQ(blueBall2Ptr->somedate, 42);
}

/*
* CRTP 版本原型模式
*/
class CRTPBall {
    virtual std::unique_ptr<CRTPBall> clone() const = 0;
};

template <typename Derived>
class CRTPBallImpl : public CRTPBall {
public:
    std::unique_ptr<CRTPBall> clone() const override {
        Derived* that =  const_cast<Derived*>(static_cast<const Derived*>(this));
        return std::make_unique<Derived>(*that);
    }
};
class CRTPRedBall  : public CRTPBallImpl<CRTPRedBall> {  
};
class CRTPBlueBall  : public CRTPBallImpl<CRTPBlueBall> {
public:
    int somedate;
};

TEST(DesignPatternTest, PrototypePatternCRTP) {
    auto redBall = std::make_unique<CRTPRedBall>();
    auto redBall2 = redBall->clone();
    ASSERT_NE(redBall2, nullptr);
    ASSERT_EQ(typeid(*redBall2), typeid(CRTPRedBall));
    EXPECT_NE(redBall2.get(), redBall.get());

    auto blueBall = std::make_unique<CRTPBlueBall>();
    blueBall->somedate = 42;
    auto blueBall2 = blueBall->clone();
    ASSERT_NE(redBall2, nullptr);
    ASSERT_EQ(typeid(*blueBall2), typeid(CRTPBlueBall));
    EXPECT_NE(blueBall2.get(), blueBall.get());

    auto blueBall2Ptr = dynamic_cast<CRTPBlueBall*>(blueBall2.get());
    EXPECT_EQ(blueBall2.get(), blueBall2Ptr);
    EXPECT_EQ(blueBall2Ptr->somedate, 42);
}

/*
* 组合模式
*
*/
class GameObject;
class MoveMessage;
class HealthMessage;
class MessageVisitor {
public:
    virtual void visit(MoveMessage *mm) = 0;
    virtual void visit(HealthMessage *hm) = 0;
};

class Message {
public:
    virtual void accept(MessageVisitor *visitor) = 0;
    virtual ~Message() = default;
};

template <class Derived>
class MessageImpl : public Message {
public:
    void accept(MessageVisitor *visitor) override {
        static_assert(std::is_base_of_v<MessageImpl, Derived>);
        visitor->visit(static_cast<Derived*>(this));
    }
};
class MoveMessage : public MessageImpl<MoveMessage> {
public:
    int velocitychange = 1;
    // void accept(MessageVisitor *visitor) override {
    //     visitor->visit(this); // visit(MoveMessage *mm)
    // }
};

class HealthMessage : public MessageImpl<HealthMessage> {
public:
    int healthchange = 1;
    // void accept(MessageVisitor *visitor) override {
    //     visitor->visit(this); // visit(HealthMessage *hm)
    // }
};

class Component : public MessageVisitor {
public:
    virtual void update(GameObject *g_obj) = 0;
    virtual void handleMessage(Message *msg) = 0;
    virtual void subscribeMessages(GameObject *g_obj) = 0;
    virtual ~Component() = default; // 确保析构函数是虚函数,以便在删除派生类对象时调用正确的析构函数
};


class GameObject {
public:
    std::unordered_map<std::type_index, std::vector<Component*>> subscribers;  // 事件总线
    std::vector<Component*> components; // 存储组件的更新顺序

    void add(Component* component) {
        components.push_back(component); // 将组件添加到更新顺序中
        component->subscribeMessages(this); // 订阅组件的消息
    }
    
    void update() {
        for (auto&& component : components) {
            component->update(this);
        }
    }

    template<typename T>
    T* getComponent() {
        for (auto&& component : components) {
            if (auto ptr = dynamic_cast<T*>(component)) {
                return ptr;
            }
        }
        return nullptr;
    }

    template <typename EventType>
    void subscribe(Component *component) {
        subscribers[std::type_index(typeid(EventType))].push_back(component);
    }

    template <typename EventType>
    void send(EventType *msg) {
        for (auto &&c: subscribers[std::type_index(typeid(EventType))]) {
            c->handleMessage(msg);
        }
    }
};

class Movable : public Component {
public:
    int position = 0;
    int velocity = 1;
    int testmessage = 1;
    void update(GameObject* g_obj) override {
        position += velocity;
    }
    void subscribeMessages(GameObject* g_obj) {
        g_obj->subscribe<MoveMessage>(this);
    }
    // void handleMessage(Message *msg) override {
    //     if (MoveMessage *mm = dynamic_cast<MoveMessage*>(msg)) {
    //         testmessage += mm->velocitychange;
    //     }
    // }
    void handleMessage(Message *msg) override {
        msg->accept(this);
    }
    void visit(MoveMessage *mm) override {
        testmessage  += mm->velocitychange;
    }
    void visit(HealthMessage *hm) override {}
};

class LivingBeing : public Component {
public:
    int health = 100;
    void update(GameObject* g_obj) override {
        health--;
    }
    void subscribeMessages(GameObject* g_obj) {
         g_obj->subscribe<HealthMessage>(this);
    }
    // void handleMessage(Message *msg) override {
    //     if (HealthMessage* hm = dynamic_cast<HealthMessage*>(msg)) {
    //         health += hm->healthchange;
    //     }
    // }

    void handleMessage(Message *msg) override {
        msg->accept(this);
    }
    void visit(MoveMessage *mm) override {}
    void visit(HealthMessage *hm) override {
        health += hm->healthchange;
    }
};

// 组件之间通信
class PlayerController : public Component {
public:
    void update(GameObject* g_obj) override {
        Movable* movable = g_obj->getComponent<Movable>();
        if (!movable) {
            throw std::runtime_error("No Movable component found in GameObject");
        }
        if (movable->testmessage == 1) {
            MoveMessage mm;
            mm.velocitychange += 1;
            g_obj->send(&mm);
        }
        if (movable->position > 3) {
            movable->velocity = -1;
        }
    }
    void subscribeMessages(GameObject* g_obj) {}
    void handleMessage(Message *msg) override {}
    void visit(MoveMessage *mm) override {}
    void visit(HealthMessage *hm) override {}
};
class CPlayer : public GameObject {
public:
    Movable* movable;
    LivingBeing* livingBeing;
    CPlayer() {
        movable = new Movable();
        livingBeing = new LivingBeing();
        add(movable);
        add(livingBeing);
    }
};

GameObject* createPlayer() {
    GameObject* player = new GameObject();
    player->add(new Movable());
    player->add(new LivingBeing());
    return player;
}

TEST(DesignPatternTest, CompositePattern_createPlayer1) {
    CPlayer player;
    player.update();
    ASSERT_EQ(player.getComponent<Movable>()->position, 1);
    ASSERT_EQ(player.getComponent<LivingBeing>()->health, 99);
}

TEST(DesignPatternTest, CompositePattern_createPlayer2) {
    GameObject* player = createPlayer();
    player->update();
    ASSERT_EQ(player->getComponent<Movable>()->position, 1);
    ASSERT_EQ(player->getComponent<LivingBeing>()->health, 99);
}

TEST(DesignPatternTest, CompositePattern_PlayerControllerTest) {
    GameObject* player = createPlayer();
    player->add(new PlayerController());

    // 更新 3 次，使 position 达到 3
    for (int i = 0; i < 3; ++i) {
        player->update();
    }
    ASSERT_EQ(player->getComponent<Movable>()->position, 3);
    ASSERT_EQ(player->getComponent<Movable>()->velocity, 1);


    player->update();
    ASSERT_EQ(player->getComponent<Movable>()->position, 4);
    ASSERT_EQ(player->getComponent<Movable>()->velocity, -1);
}

TEST(DesignPatternTest, ObservePattern) {
    GameObject* player = createPlayer();
    player->add(new PlayerController());
    player->update();
    ASSERT_EQ(player->getComponent<Movable>()->testmessage, 3);

    player->update();
    ASSERT_EQ(player->getComponent<Movable>()->testmessage, 3);

    MoveMessage mm;
    mm.velocitychange = 2;
    player->send(&mm);
    ASSERT_EQ(player->getComponent<Movable>()->testmessage, 5);

    ASSERT_EQ(player->getComponent<LivingBeing>()->health, 98);
    HealthMessage hm;
    hm.healthchange = 5;
    player->send(&hm);
    ASSERT_EQ(player->getComponent<LivingBeing>()->health, 103);
}

/*
* 类型擦除
*/
#include "type_erase_msglib.h"
#include <vector>
#include <memory>
struct MsgBase {
    virtual void speak() = 0;
    virtual void load() = 0;
    virtual std::shared_ptr<MsgBase> clone() const = 0;
    virtual ~MsgBase() = default;
    using Ptr = std::shared_ptr<MsgBase>;
};
namespace msg_extra_funcs {
    void load(MoveMsg &msg) {
        std::cin >> msg.x >> msg.y;
    }

    void load(JumpMsg &msg) {
        std::cin >> msg.height;
    }

    void load(SleepMsg &msg) {
        std::cin >> msg.time;
    }

    void load(ExitMsg &) {
    }
}
template <class Msg>
struct MsgImpl : MsgBase {
    Msg msg;
    template <class ...Ts>
    MsgImpl(Ts &&... ts) : msg{std::forward<Ts>(ts)...} {}

    void speak() override {
        msg.speak();
    }

    void load() override {
        msg_extra_funcs::load(msg);
    }
    std::shared_ptr<MsgBase> clone() const override {
        return std::make_shared<MsgImpl<Msg>>(msg);
    }
};

struct MsgFactoryBase {
    virtual MsgBase::Ptr create() = 0;
    virtual ~MsgFactoryBase() = default;
    using Ptr = std::shared_ptr<MsgFactoryBase>;
};

template <class Msg>
struct MsgFactoryImpl : MsgFactoryBase {
    MsgBase::Ptr create() override {
        return std::make_shared<MsgImpl<Msg>>();
    }
};

template <class Msg>
MsgFactoryBase::Ptr makeFactory() {
    return std::make_shared<MsgFactoryImpl<Msg>>();
}

template <class Msg, class ...Ts>
std::shared_ptr<MsgBase> makeMsg(Ts && ...ts) {
    return std::make_shared<MsgImpl<Msg>>(std::forward<Ts>(ts)...);
}

TEST(DesignPatternTest, TypeErase) {
    std::vector<std::shared_ptr<MsgBase>> msgs;
    msgs.push_back(makeMsg<MoveMsg>(5, 10));
    msgs.push_back(makeMsg<JumpMsg>(20));
    msgs.push_back(makeMsg<SleepMsg>(8));
    msgs.push_back(makeMsg<ExitMsg>());

    for (auto &msg : msgs) {
        msg->speak();
    }
}

struct RobotClass {
    inline static const std::map<std::string, MsgFactoryBase::Ptr> factories = {
#define PER_MSG(Type) {#Type, makeFactory<Type##Msg>()},
    PER_MSG(Move)
    PER_MSG(Jump)
    PER_MSG(Sleep)
    PER_MSG(Exit)
#undef PER_MSG
    };

    void recv_data() {
        std::string type;
        std::cin >> type;

        try {
            msg = factories.at(type)->create();
        } catch (std::out_of_range &) {
            std::cout << "no such msg type!\n";
            return;
        }

        msg->load();
    }

    void update() {
        if (msg)
            msg->speak();
    }

    MsgBase::Ptr msg;
};

TEST(DesignPatternTest, TypeErase1) {
    std::istringstream input("Move\n5 10\n");
    std::cin.rdbuf(input.rdbuf());
    RobotClass robot;
    robot.recv_data();
    ASSERT_LOGS_STDOUT(robot.update(), "Move 5, 10");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}