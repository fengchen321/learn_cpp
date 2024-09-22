//
// Created by lsfco on 2024/9/7.
//
#include <iostream>
#include <vector>
#include "print.h"
#include "ScopeProfiler.h"

int main() {
    int sum = 0;
    std::vector<int> arr;
    arr.resize(1'000'000); //1 million elements
    for (size_t i = 0; i < arr.size(); i++) {
        arr[i] = i % 10;
    }
    {
        sum = 0;
        ScopeProfiler _("下标遍历");
        for (size_t i = 0; i < arr.size(); i++) {
            sum += arr[i] % 10;
        }
        print(sum);
    }
    {
        sum = 0;
        ScopeProfiler _("range遍历");
        for (auto const &a : arr) {
            sum += a % 10;
        }
        print(sum);
    }
    {
        sum = 0;
        ScopeProfiler _("迭代器遍历");
        for (auto it = arr.begin(); it!= arr.end(); ++it) {
            sum += *it %10;
        }
        print(sum);
    }
    {
        sum = 0;
        ScopeProfiler _("ָ指针遍历");
        int *p = arr.data(), *end = p + arr.size();
        while (p != end) {
            sum += *p % 10;
            p++;
        }
        print(sum);
    }
    printScopeProfiler();
    return 0;
}