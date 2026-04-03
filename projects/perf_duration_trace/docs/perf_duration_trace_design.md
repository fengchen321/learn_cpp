# perf_duration_trace Design

## 概述

`perf_duration_trace` 是一个单头文件、可直接拷贝接入运行时库的低开销耗时采样模块。

真实部署目标是只带走一个头文件，不依赖单独的 `.cpp` 编译单元。这也是接口设计和内部实现必须保持 header-only 的原因。

它提供两种互补模式：

- 自动模式：通过 RAII 在作用域进入时记录开始时间，在析构时提交样本。
- 手动模式：通过 `PERF_BEGIN()` / `PERF_END()` 显式控制起止点，适合跨函数、异步回调和复杂控制流。

C++ 端只负责采集原始样本和导出元数据，不在热路径内做聚合统计。异常值处理、分位数统计和回归判断交给 Python 端完成。

## 架构设计

### 部署形态

- 最终交付物是单个头文件 [perf_duration_trace.h](include/perf_duration_trace.h)
- 所有运行时代码都以内联函数和头文件内部静态对象形式存在
- 测试编译方式只验证 `#include` 该头文件，不依赖额外实现文件
- 为了规避全局析构顺序问题，运行时单例和内部状态单例采用“故意不析构”的进程级对象模式

### 前端接口

前端暴露两套等价宏：

- 业务友好别名：`PERF_SCOPE`、`PERF_BEGIN`、`PERF_END`
- 明确前缀版本：`PERF_TRACE_SCOPE`、`PERF_TRACE_BEGIN`、`PERF_TRACE_END`

同时暴露两类导出接口：

- 同步导出：`finalize()`
- 异步导出：`start_async_export()` / `stop_async_export()`

所有埋点都会被编译期宏 `PERF_ENABLED` 控制。关闭时这些接口退化为 no-op，不引入运行时分支。

### 站点注册

每个埋点站点第一次命中时会注册一个稳定的 `site_id`，并持久保存以下信息：

- 标签名
- 源文件
- 函数名
- 行号

`SiteRef` 只保存两部分数据：`id` 和 `SiteDesc`。不再重复平铺元数据字段，避免模型冗余。

### 采集后端

采集核心使用“分片有界队列”模型：

- 运行时根据配置创建多个 shard。
- 每个线程只缓存很小的 TLS 路由信息，例如 `tid_hash` 和 `shard_id`。
- 真正的样本存储在全局 shard 内的有界 MPMC 环形队列中。
- 当前实现采用 `drop newest when full` 策略，避免队列满时阻塞业务线程。

### 导出

导出阶段生成两份文件：

- `*.perfbin`：固定宽度二进制样本文件
- `*.sites.tsv`：站点元数据

同步模式下，`finalize()` 会一次性 drain 所有 shard，把样本合并并按 `start_ns` 排序后落盘。

异步模式下，后台线程会按配置周期增量 drain 分片队列并追加写入 `perfbin`，`stop_async_export()` 负责回写最终文件头并输出站点元数据。这样做可以把 I/O 从业务线程剥离出去。

## 平台假设

- **目标平台**：x86-64 架构（确保 `uint64_t` 读写原子）
- **编译器要求**：C++17 或更高
- **操作系统**：Linux/Unix（使用 POSIX `clock_gettime`）

## 线程安全模型

### 线程安全接口
- `Runtime::instance()`：完全线程安全
- `register_site()`：线程安全（内部使用 mutex）
- `begin()` / `end()`：完全线程安全
- `start_async_export()` / `stop_async_export()`：线程安全，支持并发调用

### Token 线程限制
- **不应跨线程传递**
- 虽然 x86-64 上 `uint64_t` 读写原子，但缺少内存序保证
- 如需跨函数传递，仅限创建和结束在同一线程时安全

### 全局状态
- `detail::state()` 单例全进程共享
- Shard 队列全局共享，通过 thread_local 路由访问
- 采用 Leaky Singleton 模式，永不析构

## 接口定义

```cpp
#include "perf_duration_trace.h"

PERF_SCOPE("parse_frame");

auto token = PERF_BEGIN("rpc_decode");
// ...
PERF_END(token);

perf_duration_trace::Runtime::instance().start_async_export({
    {"run.perfbin", "run.sites.tsv"},
    50,
});

perf_duration_trace::Runtime::instance().stop_async_export();
```

## 已知限制

### Token 内存序
在 x86-64 平台 + Token 不跨线程的使用模式下，当前实现的内存序是正确的。如果未来需要支持 Token 跨线程传递，需要添加内存序保证。

### 单例析构顺序
采用 Leaky Singleton 模式，全局单例永不析构。这是有意的设计，避免析构顺序竞争。内存检查工具可能报告泄漏，这是正常的。

## 数据流图

```mermaid
flowchart LR
    A[C++ Runtime Code] --> B[Single Header API]
    B --> C[Static Site Registry]
    B --> D[Shard Router hash(tid)]
    D --> E[Shard 0 MPMC Queue]
    D --> F[Shard 1 MPMC Queue]
    D --> G[Shard N MPMC Queue]
    E --> H[Async or Final Drain]
    F --> H
    G --> H
    C --> H
    H --> I[perfbin Sample File]
    H --> J[sites TSV Metadata]
    I --> K[Python Analyzer]
    J --> K
    K --> L[Outlier Filtering]
    L --> M[Statistics Summary]
    M --> N[Regression Report]
```

## 异常处理策略

- 热路径不抛异常。
- 时钟优先使用 `CLOCK_MONOTONIC_RAW`，失败时退回到 `CLOCK_MONOTONIC`，最后退回 `std::chrono::steady_clock`。
- 队列满时不阻塞生产者，而是丢弃新样本并累计 `dropped_samples`。
- 异步导出线程只承担 drain 和文件追加写入；业务线程不做同步刷盘。
- 导出失败通过 `ExportStats.success` 返回，不在析构阶段抛异常。
- 并发调用 `stop_async_export()` 时，只有第一个调用者执行真正的停止流程，其他调用者会等待并复用最终统计结果。

## 最佳实践

- 如果 `Token` 要跨线程传递，必须依赖调用方自己的同步机制来发布和消费它，例如线程启动、互斥量、条件变量、promise/future 或有正确内存序的无锁队列。工具本身不会为“无同步发布”兜底。
- 推荐在 `main()` 的业务线程启动前调用 `start_async_export()`，并在所有生产者线程退出后再调用 `stop_async_export()` 或 `finalize()`。
- 不建议在全局/静态对象析构函数或动态库卸载回调里继续打点；最佳做法是在宿主明确的生命周期边界内启动和停止采集。
- 动态库场景下应由宿主显式控制开始和停止，避免把导出时序交给卸载顺序。

### 关于“永不析构”的单例

为了规避 header-only 场景下难以控制的全局析构顺序问题，`Runtime` 和内部 `state()` 采用“进程级对象，故意不析构”的策略。

- 这会导致内存检查工具在进程退出时报告 `still reachable` 或类似的“泄漏”提示。
- 这是有意的设计取舍：如果在退出阶段析构单例，就无法阻止其他全局/静态析构函数继续调用 `PERF_SCOPE` 进而产生 use-after-free 风险。
