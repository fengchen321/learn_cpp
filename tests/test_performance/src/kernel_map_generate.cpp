#include "kernel_map_generate.h"
#include <random>
#include <unordered_set>
#include <algorithm>

std::vector<std::string> GenerateKernelNames(int count) {
    std::vector<std::string> names;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> size_dist(10, 500); // 名称长度范围 100-1000
    std::uniform_int_distribution<> char_dist(0, 25);    // 随机字母
    
    // 常见深度学习操作前缀
    const char* ops[] = {
        "conv2d", "matmul", "batch_norm", "relu", "gelu", 
        "layer_norm", "attention", "softmax", "dropout", "embedding",
        "reduce_sum", "scatter_add", "gather", "transpose", "reshape"
    };
    
    // 常见后缀/参数模式
    const char* suffixes[] = {
        "_f32", "_f16", "_bf16", "_i32", "_i64",
        "_tile32x32", "_tile64x64", "_tile128x128", "_vec4", "_vec8",
        "_sm80", "_sm90", "_grad", "_backward", "_optimizer",
        "_kernel", "_device", "_host", "_stream", "_pipeline"
    };

    // 特殊长字符串模式
    const char* long_patterns[] = {
        "_with_very_long_and_complex_parameters_that_include_",
        "_and_special_features_like_autotune_and_optimization_flags_",
        "_including_all_possible_configurations_and_edge_cases_"
    };

    for (int i = 0; i < count; ++i) {
        std::string name;
        
        // 随机选择操作
        name += ops[i % (sizeof(ops)/sizeof(ops[0]))];
        
        // 添加随机维度信息
        name += "_" + std::to_string(1 << (5 + i % 7)) + 
                "x" + std::to_string(1 << (5 + (i+2) % 7));
        
        // 添加随机后缀
        name += suffixes[i % (sizeof(suffixes)/sizeof(suffixes[0]))];
        
        // 添加长模式
        name += long_patterns[i % (sizeof(long_patterns)/sizeof(long_patterns[0]))];
        
        // 确保达到最小长度要求
        while (name.size() < size_dist.min()) {
            name += "_" + std::to_string(i * 12345 % 100000);
        }
        
        // 随机扩展至目标长度
        int target_length = size_dist(gen);
        while (name.size() < target_length) {
            name += ('a' + char_dist(gen));
        }
        // 添加随机唯一标识
        name += "_uid" + std::to_string(i);
        // 如果超过长度则截断
        if (name.size() > size_dist.max()) {
            name = name.substr(0, size_dist.max());
        }
        names.push_back(name);
    }
    
    return names;
}

std::vector<std::string> GenerateUniqueKernelNames(int count) {
    std::vector<std::string> names;
    std::unordered_set<std::string> name_set; // 用于去重
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> size_dist(10, 500); // 名称长度范围
    std::uniform_int_distribution<char> char_dist('a', 'z'); // 随机字母

    // 操作前缀和后缀（扩展更多选项）
    const std::vector<std::string> ops = {
        "conv2d", "matmul", "batch_norm", "relu", "gelu", 
        "layer_norm", "attention", "softmax", "dropout", "embedding",
        "reduce_sum", "scatter_add", "gather", "transpose", "reshape",
        "conv1d", "maxpool", "avgpool", "leaky_relu", "silu"
    };

    const std::vector<std::string> suffixes = {
        "_f32", "_f16", "_bf16", "_i32", "_i64",
        "_tile32x32", "_tile64x64", "_tile128x128", "_vec4", "_vec8",
        "_sm80", "_sm90", "_grad", "_backward", "_optimizer",
        "_kernel", "_device", "_host", "_stream", "_pipeline",
        "_v1", "_v2", "_v3", "_async", "_sync"
    };

    const std::vector<std::string> long_patterns = {
        "_with_params_", "_optimized_", "_parallel_", "_padded_",
        "_with_shared_mem_", "_unrolled_", "_vectorized_"
    };

    while (names.size() < static_cast<size_t>(count)) {
        std::string name;

        // 随机选择组件
        std::uniform_int_distribution<size_t> op_dist(0, ops.size() - 1);
        std::uniform_int_distribution<size_t> suffix_dist(0, suffixes.size() - 1);
        std::uniform_int_distribution<size_t> pattern_dist(0, long_patterns.size() - 1);

        name += ops[op_dist(gen)];
        name += "_" + std::to_string((1 << (5 + op_dist(gen) % 7))); // 随机维度
        name += "x" + std::to_string((1 << (5 + suffix_dist(gen) % 7)));
        name += suffixes[suffix_dist(gen)];
        name += long_patterns[pattern_dist(gen)];

        // 添加随机字符直到达到目标长度
        int target_length = size_dist(gen);
        while (name.size() < target_length) {
            name += char_dist(gen);
        }

        // 添加唯一标识（确保唯一性）
        name += "_uid" + std::to_string(names.size());

        // 去重检查
        if (name_set.insert(name).second) {
            names.push_back(name);
        }
    }

    return names;
}

std::vector<std::string> selectRandomElements(
    const std::vector<std::string>& input,
    size_t n
) {
    std::vector<std::string> result;
    if (input.empty() || n == 0) return result;

    // 确保不超出范围
    n = std::min(n, input.size());

    // 使用随机设备初始化生成器
    std::random_device rd;
    std::mt19937 gen(rd());

    // 从 input 中随机选取 n 个元素到 result
    std::sample(
        input.begin(), input.end(),
        std::back_inserter(result),
        n,
        gen
    );

    return result;
}
std::vector<size_t> fixed_indices(int count, size_t max_index) {
    std::vector<size_t> indices(count);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, max_index - 1);
    
    for (int i = 0; i < count; ++i) {
        indices[i] = dist(gen);
    }
    return indices;
}