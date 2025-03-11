#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <iomanip>
#include <random>
#include <algorithm>

#include "KICachePolicy.h"
#include "LfuCache.h"
#include "LruCache.h"
#include "LruKCache.h"
#include "HashLfuCache.h"
#include "HashLruCache.h"
#include "ArcCache.h"
class Timer
{
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    double elapsed()
    {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

// 辅助函数：打印结果
void printResults(const std::string &testName, int capacity,
                  const std::vector<int> &get_operations,
                  const std::vector<int> &hits)
{
    std::cout << "缓存大小: " << capacity << std::endl;
    std::cout << "LRU - 命中率: " << std::fixed << std::setprecision(2)
              << (100.0 * hits[0] / get_operations[0]) << "%" << std::endl;
    std::cout << "LFU - 命中率: " << std::fixed << std::setprecision(2)
              << (100.0 * hits[1] / get_operations[1]) << "%" << std::endl;
    std::cout << "klru - 命中率: " << std::fixed << std::setprecision(2)
              << (100.0 * hits[2] / get_operations[2]) << "%" << std::endl;
    std::cout << "hashlfu - 命中率: " << std::fixed << std::setprecision(2)
              << (100.0 * hits[3] / get_operations[3]) << "%" << std::endl;
    std::cout << "hashlru - 命中率: " << std::fixed << std::setprecision(2)
              << (100.0 * hits[4] / get_operations[4]) << "%" << std::endl;
    std::cout << "arc - 命中率: " << std::fixed << std::setprecision(2)
              << (100.0 * hits[5] / get_operations[5]) << "%" << std::endl;
}

void testHotDataAccess()
{
    std::cout << "\n=== 测试场景1：热点数据访问测试 ===" << std::endl;

    const int CAPACITY = 50;       // 增加缓存容量
    const int OPERATIONS = 500000; // 增加操作次数
    const int HOT_KEYS = 100;      // 增加热点数据的数量
    const int COLD_KEYS = 100;

    KamaCache::LruCache<int, std::string> lru(CAPACITY);
    KamaCache::LfuCache<int, std::string> lfu(CAPACITY);
    KamaCache::KLruCache<int, std::string> Klru(CAPACITY, 25, 2);
    KamaCache::HashLfuCache<int, std::string> HashLfu(CAPACITY, 5);
    KamaCache::HashLruCache<int, std::string> HashLRu(CAPACITY, 5);
    KamaCache::ArcCache<int, std::string> arc(CAPACITY, 3);
    std::random_device rd;
    std::mt19937 gen(rd());

    std::array<KamaCache::KICachePolicy<int, std::string> *, 6> caches = {&lru, &lfu, &Klru, &HashLfu, &HashLRu, &arc};
    std::vector<int> hits(6, 0);
    std::vector<int> get_operations(6, 0);

    // 先进行一系列put操作
    for (int i = 0; i < caches.size(); ++i)
    {
        for (int op = 0; op < OPERATIONS; ++op)
        {
            int key;
            if (op % 100 < 70)
            { // 70%热点数据
                key = gen() % HOT_KEYS;
            }
            else
            { // 30%冷数据
                key = HOT_KEYS + (gen() % COLD_KEYS);
            }
            std::string value = "value" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // 然后进行随机get操作
        for (int get_op = 0; get_op < OPERATIONS; ++get_op)
        {
            int key;
            if (get_op % 100 < 70)
            { // 70%概率访问热点
                key = gen() % HOT_KEYS;
            }
            else
            { // 30%概率访问冷数据
                key = HOT_KEYS + (gen() % COLD_KEYS);
            }

            std::string result;
            get_operations[i]++;
            if (caches[i]->get(key, result))
            {
                hits[i]++;
            }
        }
    }

    printResults("热点数据访问测试", CAPACITY, get_operations, hits);
}

int main()
{
    testHotDataAccess();

    return 0;
}