#pragma once

#include "LruCache.h"
#include <memory>
#include <iostream>
#include <unordered_map>
#include <thread>
#include <vector>
// 采用排除局部最近最少使用
namespace KamaCache
{
    template <typename Key, typename Value>
    class HashLruCache : public KICachePolicy<Key, Value>
    {
    private:
        size_t capacity_;
        int sliceNum_;
        std::vector<std::unique_ptr<LruCache<Key, Value>>> LruHash;
        size_t Hash(Key key)
        {
            std::hash<Key> hash_f;
            return hash_f(key) % sliceNum_;
        }

    public:
        HashLruCache(size_t capacity, int sliceNum) : capacity_(capacity),
                                                      sliceNum_(sliceNum > 0 ? sliceNum : std::thread::hardware_concurrency())
        {
            // 每个Lru的大小
            size_t sliceSize = std::ceil(capacity / static_cast<double>(sliceNum_));
            for (int i = 0; i < sliceNum_; ++i)
            {
                LruHash.emplace_back(std::make_unique<LruCache<Key, Value>>(sliceSize));
            }
        }
        void put(Key key, Value value) override
        {
            size_t index_Lru = Hash(key);
            return LruHash[index_Lru]->put(key, value);
        }
        bool get(Key key, Value &value) override
        {
            size_t index_Lru = Hash(key);
            return LruHash[index_Lru]->get(key, value);
        }
        Value get(Key key) override
        {
            Value value;
            memset(&value, 0, sizeof(value));
            get(key, value);
            return value;
        }
    };
}