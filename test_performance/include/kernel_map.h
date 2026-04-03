
#pragma once
#include <map>
#include <unordered_map>
#include <list>
#include <mutex>
#include <cstring>
#include <string>

#define IPC_BUFFER_SIZE (0x2000000)

struct StrMapBuf_t {
    uint32_t flushed_size;
    uint32_t valid_size;
    char buffer[IPC_BUFFER_SIZE];
};


template <typename KeyType>
class KernelNameMapBase {
protected:
    std::mutex mutex_;
    uint32_t index = 0;
    StrMapBuf_t* buffer_ptr = nullptr;
    std::list<StrMapBuf_t*> list_buffer;
    std::map<uint32_t, char*> map_kernel_name;
    std::map<KeyType, uint32_t> map_kernel_ptr;

public:
    KernelNameMapBase() {
        buffer_ptr = static_cast<StrMapBuf_t*>(malloc(sizeof(StrMapBuf_t)));
        buffer_ptr->flushed_size = 0;
        buffer_ptr->valid_size = 0;
        list_buffer.push_back(buffer_ptr);
    }

    virtual ~KernelNameMapBase() {
        std::lock_guard<std::mutex> lck(mutex_);
        auto itor = list_buffer.begin();
        while (itor != list_buffer.end()) {
            if (*itor != nullptr) {
                free(*itor);
            }
            ++itor;
        }
        list_buffer.clear();
    }

    const char* GetKernelNameByIdx(uint32_t index) {
        std::lock_guard<std::mutex> lck(mutex_);
        auto it = map_kernel_name.find(index);
        return it != map_kernel_name.end() ? it->second : "UNKNOWN_KERNEL_NAME";
    }

    std::pair<size_t, size_t> GetMapSize() {
        std::lock_guard<std::mutex> lck(mutex_);
        return {map_kernel_ptr.size(), map_kernel_name.size()};
    }
};

// 指针版本特化
class KernelNameMap_PtrVersion : public KernelNameMapBase<const char*> {
public:
    uint32_t AddFuncPtrAndNameStr(const char* name) {
        std::lock_guard<std::mutex> lck(mutex_);
        auto kernel_itor = map_kernel_ptr.find(name);
        if(kernel_itor != map_kernel_ptr.end()) {
            return kernel_itor->second;
        }
        ++index;
        int str_size = strlen(name) + 1;
        int need_size = str_size + sizeof(uint32_t);
        
        if(need_size + buffer_ptr->valid_size > IPC_BUFFER_SIZE) {
            buffer_ptr = (StrMapBuf_t *)malloc(sizeof(StrMapBuf_t));
            buffer_ptr->flushed_size = 0;
            buffer_ptr->valid_size = 0;
            list_buffer.push_back(buffer_ptr);
        }

        char * ptr = buffer_ptr->buffer + buffer_ptr->valid_size;
        *(uint32_t *)ptr = index;
        ptr += sizeof(uint32_t);
        memcpy(ptr, name, str_size);
        map_kernel_ptr[name] = index;
        map_kernel_name[index] = ptr;
        buffer_ptr->valid_size += need_size;
        return index;
    }
};

// 字符串版本特化
class KernelNameMap_StrVersion : public KernelNameMapBase<std::string> {
public:
     uint32_t AddFuncPtrAndNameStr(const char *name) {
        std::lock_guard<std::mutex> lck(mutex_);
        auto kernel_itor = map_kernel_ptr.find(name);
        if(kernel_itor != map_kernel_ptr.end()) {
            return kernel_itor->second;
        }

        ++index;
        int str_size = strlen(name) + 1;
        int need_size = str_size + sizeof(uint32_t);
        
        if(need_size + buffer_ptr->valid_size > IPC_BUFFER_SIZE) {
            buffer_ptr = (StrMapBuf_t *)malloc(sizeof(StrMapBuf_t));
            buffer_ptr->flushed_size = 0;
            buffer_ptr->valid_size = 0;
            list_buffer.push_back(buffer_ptr);
        }

        char * ptr = buffer_ptr->buffer + buffer_ptr->valid_size;
        *(uint32_t *)ptr = index;
        ptr += sizeof(uint32_t);
        memcpy(ptr, name, str_size);
        map_kernel_ptr[std::string(name)] = index;
        map_kernel_name[index] = ptr;
        buffer_ptr->valid_size += need_size;
        return index;
    }
};

// 内存拷贝版本（无反向映射）最终耗时会比实际低：因为另两个版本也要最终也要push进buffer也是要拷贝
class KernelNameMap_memcpyVersion : public KernelNameMapBase<void*> {
public:
    uint32_t AddFuncPtrAndNameStr(const char *name) {
        std::lock_guard<std::mutex> lck(mutex_);
        ++index;
        int str_size = strlen(name) + 1;
        int need_size = str_size + sizeof(uint32_t);

        if (need_size + buffer_ptr->valid_size > IPC_BUFFER_SIZE) {
            buffer_ptr = (StrMapBuf_t *)malloc(sizeof(StrMapBuf_t));
            buffer_ptr->flushed_size = 0;
            buffer_ptr->valid_size = 0;
            list_buffer.push_back(buffer_ptr);
        }

        char *ptr = buffer_ptr->buffer + buffer_ptr->valid_size;
        *(uint32_t *)ptr = index;
        ptr += sizeof(uint32_t);
        memcpy(ptr, name, str_size);
        map_kernel_name[index] = ptr;
        buffer_ptr->valid_size += need_size;
        return index;
    }
};
// unorder
class KernelNameUnorderMap_StrVersion : public KernelNameMapBase<std::string> {
    std::unordered_map<std::string, uint32_t> map_kernel_ptr;
public:
     uint32_t AddFuncPtrAndNameStr(const char *name) {
        std::lock_guard<std::mutex> lck(mutex_);
        auto kernel_itor = map_kernel_ptr.find(name);
        if(kernel_itor != map_kernel_ptr.end()) {
            return kernel_itor->second;
        }

        ++index;
        int str_size = strlen(name) + 1;
        int need_size = str_size + sizeof(uint32_t);
        
        if(need_size + buffer_ptr->valid_size > IPC_BUFFER_SIZE) {
            buffer_ptr = (StrMapBuf_t *)malloc(sizeof(StrMapBuf_t));
            buffer_ptr->flushed_size = 0;
            buffer_ptr->valid_size = 0;
            list_buffer.push_back(buffer_ptr);
        }

        char * ptr = buffer_ptr->buffer + buffer_ptr->valid_size;
        *(uint32_t *)ptr = index;
        ptr += sizeof(uint32_t);
        memcpy(ptr, name, str_size);
        map_kernel_ptr[std::string(name)] = index;
        map_kernel_name[index] = ptr;
        buffer_ptr->valid_size += need_size;
        return index;
    }
};