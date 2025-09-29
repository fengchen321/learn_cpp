#pragma once

#include <vector>
#include <string>

std::vector<std::string> GenerateKernelNames(int count);
std::vector<std::string> GenerateUniqueKernelNames(int count);
std::vector<std::string> selectRandomElements(
    const std::vector<std::string>& input,
    size_t n);

std::vector<size_t> fixed_indices(int count, size_t max_index);