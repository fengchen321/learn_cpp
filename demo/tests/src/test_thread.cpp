#include "gtest_prompt.h"
#include "mt_queue.h"

TEST(MTQueueTest, PushPop) {
    mt_queue<int> queue(5);

    queue.push(1);
    queue.push(2);
    queue.push(3);

    EXPECT_EQ(queue.pop(), 1);
    EXPECT_EQ(queue.pop(), 2);
    EXPECT_EQ(queue.pop(), 3);
}

TEST(MTQueueTest, TryPush) {
    mt_queue<int> queue(2);

    EXPECT_TRUE(queue.try_push(1));
    EXPECT_TRUE(queue.try_push(2));
    EXPECT_FALSE(queue.try_push(3));

    EXPECT_EQ(queue.pop(), 1);
    EXPECT_TRUE(queue.try_push(3));
    EXPECT_EQ(queue.pop(), 2);
    EXPECT_EQ(queue.pop(), 3);
}

TEST(MTQueueTest, TryPop) {
    mt_queue<int> queue(2);
    
    EXPECT_EQ(queue.try_pop(), std::nullopt);

    queue.push(1);
    EXPECT_EQ(queue.try_pop(), 1);
    EXPECT_EQ(queue.try_pop(), std::nullopt);
}

TEST(MTQueueTest, PushPopWithTimeout) {
    mt_queue<int> queue(1);

    queue.push(1);
    EXPECT_FALSE(queue.try_push_for(2, std::chrono::milliseconds(100)));

    int value = queue.pop();
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(queue.try_push_for(2, std::chrono::milliseconds(100)));
}

TEST(MTQueueTest, ConcurrentAccess) {
    mt_queue<int> queue(10);
    std::atomic<int> count{0};
    std::vector<std::thread> threads;

    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 10; ++j) {
                queue.push(count.fetch_add(1));
            }
        });
    }

    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 10; ++j) {
                auto value = queue.pop();
                EXPECT_GE(value, 0); // 确保取出的值是非负的
            }
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }
}

using namespace std::chrono_literals;
TEST(MTQueueTest, Testlimit) {
    mt_queue<std::string> queue;
    std::vector<std::thread> threads;

    threads.emplace_back([&]() {
        queue.push("1");
        std::cout << "1 over\n";
        queue.push("2");
        std::cout << "2 over\n";
        queue.push("3");
        std::cout << "3 over\n";
        queue.push("4");
        std::cout << "4 over\n";
        std::this_thread::sleep_for(1s);
        queue.push("5");
        std::this_thread::sleep_for(1s);
        queue.push("6");
        std::this_thread::sleep_for(1s);
        queue.push("EXIT");
    });

    threads.emplace_back([&]() {
        while (true) {
            if (auto msg = queue.try_pop_for(100ms)) {
                if (*msg == "EXIT") break;
                std::cout << "t2 收到消息：" << *msg << '\n';
                std::this_thread::sleep_for(300ms);
            }
        }
    });

    for (auto &thread : threads) {
        thread.join();
    }
}

TEST(MTQueueTest, CountTest_raw) {
    int counter = 0;
    std::mutex mutex_counter;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < 10000; i += 1000) {
        threads.emplace_back([&](int beg, int end) {
            for (size_t j = beg; j < end; ++j) {
                std::lock_guard<std::mutex> lock(mutex_counter);
                counter += j;
            }
        }, i, i + 1000);
    }

    for (auto &thread : threads) {
        thread.join();
    }
    EXPECT_EQ(counter, 49995000);
}

TEST(MTQueueTest, CountTest_mt_queue) {
    mt_queue<std::optional<int>> counter_queue;
    std::vector<std::thread> threads;
    const int total_elements = 10000;
    const int elements_per_thread = 1000;
    const int num_threads = total_elements / elements_per_thread;

    for (size_t i = 0; i < total_elements; i += elements_per_thread) {
        threads.emplace_back([&](int beg, int end) {
            for (size_t j = beg; j < end; ++j) {
               counter_queue.push(j);
            }
            counter_queue.push(std::nullopt);
        }, i, i + elements_per_thread);
    }
    // 计算线程
    threads.emplace_back([&](){
        int counter = 0;
        int complete_count = 0;
        while(complete_count < num_threads) {
            auto i = counter_queue.pop();
            if (i == std::nullopt) {
                complete_count++;
            } else {
                counter += i.value();
            }
        }
        EXPECT_EQ(counter, 49995000);
    });
    for (auto &thread : threads) {
        thread.join();
    }
}

#if __cplusplus >= 202002L
#include <barrier>
TEST(MTQueueTest, CountTest_barrier) {
    mt_queue<std::optional<int>> counter_queue;

    std::vector<std::thread> threads;
    const int total_elements = 10000;
    const int elements_per_thread = 1000;
    const int num_threads = total_elements / elements_per_thread;
    std::barrier finish(num_threads + 1);

    for (size_t i = 0; i < total_elements; i += elements_per_thread) {
        threads.emplace_back([&](int beg, int end) {
            for (size_t j = beg; j < end; ++j) {
               counter_queue.push(j);
            }
            // finish.arrive_and_wait();
            finish.arrive_and_drop();
        }, i, i + elements_per_thread);
    }
    // 计算线程
    threads.emplace_back([&](){
        int counter = 0;
        while(true) {
            if (auto i = counter_queue.pop()) {
                counter += i.value();
            } else {
                break;
            }
        }
        EXPECT_EQ(counter, 49995000);
    });
    // 完成线程
    threads.emplace_back([&]() {
        finish.arrive_and_wait();
        counter_queue.push(std::nullopt);
    });

    for (auto &thread : threads) {
        thread.join();
    }
}
#endif
#include <variant>
TEST(MTQueueTest, CountTest_mt_queue_1) {
    mt_queue<std::optional<int>> counter_queue;
    mt_queue<std::monostate> finish_queue;

    std::vector<std::thread> threads;
    const int total_elements = 10000;
    const int elements_per_thread = 1000;
    const int num_threads = total_elements / elements_per_thread;

    for (size_t i = 0; i < total_elements; i += elements_per_thread) {
        threads.emplace_back([&](int beg, int end) {
            for (size_t j = beg; j < end; ++j) {
               counter_queue.push(j);
            }
           finish_queue.push(std::monostate());
        }, i, i + elements_per_thread);
    }
    // 计算线程
    threads.emplace_back([&](){
        int counter = 0;
        while(true) {
            if (auto i = counter_queue.pop()) {
                counter += i.value();
            } else {
                break;
            }
        }
        EXPECT_EQ(counter, 49995000);
    });
    // 完成线程
    threads.emplace_back([&]() {
        for (size_t i = 0; i < num_threads; ++i) {
            finish_queue.pop();
        }
        counter_queue.push(std::nullopt);
    });

    for (auto &thread : threads) {
        thread.join();
    }
}

#include <sstream>
TEST(MTQueueTest, CountTest_function) {
    struct CounterState {
        int counter = 0;
        std::string msg = "";
        bool finish = false;
    };
    mt_queue<std::function<void(CounterState &)>> counter_queue;
    mt_queue<int> result_queue;

    std::vector<std::thread> computer_threads;
    std::vector<std::thread> counter_threads;

    const int total_elements = 10000;
    const int elements_per_thread = 1000;
    const int num_threads = total_elements / elements_per_thread;

    auto counter_thread = [&](mt_queue<std::function<void(CounterState &)>> &counter_queue) {
        CounterState state;
        while (!state.finish) {
            auto task = counter_queue.pop();
            task(state);
        }
        std::ostringstream ss;
        ss << "msg: " << state.msg << '\n';
        ss << "counter: " << state.counter << '\n';
        std::cout << ss.str();
        result_queue.push(state.counter);
    };

    auto compute = [&](mt_queue<std::function<void(CounterState &)>> &counter_queue, int beg, int end) {
        for (int i = beg; i < end; ++i) {
            counter_queue.push([i](CounterState &state) {
                state.counter += i;
            });
        }
        counter_queue.push([](CounterState &state) {
            state.msg += "OK";
        });
    };

    for (int i = 0; i < 10000; i += 1000) {
        computer_threads.push_back(std::thread(compute, std::ref(counter_queue), i, i + 1000));
    }

    for (int i = 0; i < 3; ++i) {
        counter_threads.push_back(std::thread(counter_thread, std::ref(counter_queue)));
    }

    for (auto &&t : computer_threads) {
        t.join();
    }

    // 通知计数线程结束
    for (int i = 0; i < 3; ++i) {
        counter_queue.push([](CounterState &state) {
            state.finish = true;
        });
    }

    for (auto &&t : counter_threads) {
        t.join();
    }

    int result = 0;
    for (int i = 0; i < 3; i++) {
        result += result_queue.pop();
    }
    EXPECT_EQ(result, 49995000);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
