#include <algorithm>
#include <map>
#include <queue>
#include <cmath>
#include <utility>
#include <numeric>

// 支持迭代和元素赋值的容器
template <typename F, typename R>
R mapf(F&& func, R range) {
    std::transform(std::begin(range), std::end(range), std::begin(range), std::forward<F>(func));
    return range;
}

// 对不支持元素赋值的容器, 特化
template <typename F, typename T, typename U>
std::map<T, U> mapf(F&& func, std::map<T, U> const &m) {
    std::map<T, U> r;
    for (auto const kvp : m) r.insert(func(kvp));
    return r;
}

template <typename F, typename T>
std::queue<T> mapf(F&& func, std::queue<T> q) {
    std::queue<T> r;
    while (!q.empty()) {
        r.push(func(q.front()));
        q.pop();
    }
    return r;
}

template <typename F, typename R, typename T>
constexpr T foldl(F&& func, R&& range, T init) {
    return std::accumulate(std::begin(range), std::end(range), std::move(init), std::forward<F>(func));
}

template <typename F, typename R, typename T>
constexpr T foldr(F&& func, R&& range, T init) {
    return std::accumulate(std::rbegin(range), std::rend(range), std::move(init), std::forward<F>(func));
}

template <typename F, typename T>
constexpr T foldl(F&& func, std::queue<T> q, T init) {
    while (!q.empty()) {
        init = func(init, q.front());
        q.pop();
    }
    return init;
}

template <typename F, typename T1, typename T2>
auto foldx(F&& func, T1 arg1, T2 arg2) {
    return func(arg1, arg2);
}

template <typename F, typename T, typename... Ts>
auto foldx(F&& func, T head, Ts... rest) {
    return func(head, foldx(std::forward<F>(func), rest...));
}