#include "generate_name.h"
#include "kernel_map.h"
#include <iostream>


int COUNT;
size_t SELECT_COUNT;
std::vector<std::string> kernel_names;
std::vector<std::string> select_name;
std::vector<size_t> fix_index;

template <typename MapType>
void TestLargeScale() {
    MapType kmap;
    for (int i = 0; i < COUNT; ++i) {
        uint32_t id = kmap.AddFuncPtrAndNameStr(kernel_names[i].c_str());
    }
    printf("map_kernel_ptr.size: %zu, map_kernel_name.size: %zu\n",
           kmap.GetMapSize().first, kmap.GetMapSize().second);
}

template <typename MapType>
void TestSelectScale() {
    MapType kmap;
    for (size_t i = 0; i < COUNT; ++i) {
            size_t idx = fix_index[i];
            kmap.AddFuncPtrAndNameStr(select_name[idx].c_str());
        }
    printf("map_kernel_ptr.size: %zu, map_kernel_name.size: %zu\n",
           kmap.GetMapSize().first, kmap.GetMapSize().second);
}

template <typename MapType>
void TestSamePointerDifferentString() {

    MapType kernelMap;
    // ✅ 动态分配可修改的内存（避免修改字符串常量）
    char* name1 = new char[10];
    strcpy(name1, "KernelA");  // 写入 "KernelA"
    uint32_t id1 = kernelMap.AddFuncPtrAndNameStr(name1);
    std::cout << "Inserted: " << name1 << " -> ID: " << id1 << std::endl;
    // ✅ 验证：第一次插入的 ID
    std::cout << "ID1: " << id1 << " " << kernelMap.GetKernelNameByIdx(id1) << std::endl;

    // ✅ 复用 name1 的指针，但修改其内容为 "KernelB"
    strcpy(name1, "KernelB");  // 现在 name1 指向 "KernelB"
    uint32_t id2 = kernelMap.AddFuncPtrAndNameStr(name1);
    std::cout << "Inserted: " << name1 << " -> ID: " << id2 << std::endl;
    std::cout << "ID2: " << id2 << " " << kernelMap.GetKernelNameByIdx(id2) << std::endl;
    delete[] name1;  // 释放内存
    printf("map_kernel_ptr.size: %zu, map_kernel_name.size: %zu\n",
           kernelMap.GetMapSize().first, kernelMap.GetMapSize().second);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <COUNT> <SELECT_COUNT>\n";
        return 1;
    }

    COUNT = std::stoi(argv[1]);
    SELECT_COUNT = std::stoi(argv[2]);

    // 初始化测试数据
    kernel_names = GenerateUniqueKernelNames(COUNT);
    select_name = selectRandomElements(kernel_names, SELECT_COUNT);
    fix_index = fixed_indices(COUNT, SELECT_COUNT);

    // 运行基准测试
    printf("Running large scale tests...\n");
    // TestLargeScale<KernelNameMap_PtrVersion>();
    // TestLargeScale<KernelNameMap_StrVersion>();
    // TestLargeScale<KernelNameMap_memcpyVersion>();
    // TestLargeScale<KernelNameUnorderMap_StrVersion>();
    printf("Running select scale tests...\n");
    TestSelectScale<KernelNameMap_PtrVersion>();
    TestSelectScale<KernelNameMap_StrVersion>();
    TestSelectScale<KernelNameMap_memcpyVersion>();
    TestSelectScale<KernelNameUnorderMap_StrVersion>();

    // printf("Testing same pointer with different strings...\n");
    // TestSamePointerDifferentString<KernelNameMap_PtrVersion>();
    // TestSamePointerDifferentString<KernelNameMap_StrVersion>(); 
    // TestSamePointerDifferentString<KernelNameMap_memcpyVersion>();
    // TestSamePointerDifferentString<KernerNameMap_FixedPtrVersion>();
    return 0;
}