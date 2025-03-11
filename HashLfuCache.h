#pragma once

#include <cmath>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include "LfuCache.h"

namespace KamaCache
{
    template <typename Key, typename Value>
    class HashLfuCache : public KICachePolicy<Key, Value>
    {
    private:
        size_t Hash(Key key)
        {
            std::hash<Key> hash_f;
            return hash_f(key) % sliceNum_;
        }

    private:
        size_t capacity_;
        int sliceNum_;
        std::vector<std::unique_ptr<LfuCache<Key, Value>>> hashLfu;

    public:
        HashLfuCache(size_t capacity, int sliceNum, int maxAverageNum = 10)
            : sliceNum_(sliceNum > 0 ? sliceNum : std::thread::hardware_concurrency()), capacity_(capacity)
        {
            size_t sliceSize = std::ceil(capacity_ / static_cast<double>(sliceNum_)); // 每个lfu分片的容量
            for (int i = 0; i < sliceNum_; ++i)
            {
                hashLfu.emplace_back(std::make_unique<LfuCache<Key, Value>>(sliceSize, maxAverageNum));
            }
        }

        void put(Key key, Value value) override
        {
            size_t slice_index = Hash(key);
            return hashLfu[slice_index]->put(key, value);
        }
        bool get(Key key, Value &value) override
        {
            size_t slice_index = Hash(key);
            return hashLfu[slice_index]->get(key, value);
        }
        Value get(Key key) override
        {
            Value value;
            get(key, value);
            return value;
        }

        // 清除缓存
        void purge()
        {
            for (auto &lfuSliceCache : hashLfu)
            {
                lfuSliceCache->purge();
            }
        }
    };

}