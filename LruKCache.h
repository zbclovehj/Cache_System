#include "LruCache.h"
#include <memory>
#include <iostream>
#include <unordered_map>
// LRU-K算法是对LRU算法的改进，将原先进入缓存队列的评判标准从访问一次改为访问K次，可以说朴素的LRU算法为LRU-1。
//  LRU优化：Lru-k版本。 通过继承的方式进行再优化 子类只需要再维护一个数据访问次数的缓存器即可
namespace KamaCache
{
    template <typename Key, typename Value>
    class KLruCache : public LruCache<Key, Value>
    {
    public:
        KLruCache(int capacity, int historyCapacity, int k)
            : LruCache<Key, Value>(capacity),
              historyList_(std::make_unique<LruCache<Key, size_t>>(historyCapacity)),
              k_(k)
        {
        }
        // 从缓存系统中获取数据,访问失败也就是说缓存系统中没有这个数据就会去从内存读取
        Value get(Key key)
        {
            // 获取该数据访问次数
            int historyCount = historyList_->get(key);
            // 如果访问到数据，则更新历史访问记录节点值count++
            historyList_->put(key, ++historyCount);
            return LruCache<Key, Value>::get(key);
        }
        void put(Key key, Value value)
        {
            if (LruCache<Key, Value>::get(key) != "")
                LruCache<Key, Value>::put(key, value);
            int historyCount = historyList_->get(key);
            historyList_->put(key, ++historyCount);
            if (historyCount >= k_)
            {
                historyList_->remove(key);
                LruCache<Key, Value>::put(key, value);
            }
        }

    private:
        int k_;                                              // 进入缓存队列的评判标准
        std::unique_ptr<LruCache<Key, size_t>> historyList_; // 访问数据历史记录(value为访问次数)
    };
}