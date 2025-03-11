#pragma once

#include <cstring>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "KICachePolicy.h"
namespace KamaCache
{
    // 前向声明一个类
    template <typename Key, typename Value>
    class LruCache;
    // 定义结点，双向链表
    template <typename Key, typename Value>
    class LruNode
    {
    private:
        Key key_;
        Value value_;
        size_t accessCount_;
        std::shared_ptr<LruNode<Key, Value>> prev_;
        std::shared_ptr<LruNode<Key, Value>> next_;

    public:
        LruNode(Key key, Value value)
            : key_(key),
              value_(value),
              accessCount_(1),
              prev_(nullptr),
              next_(nullptr)
        {
        }
        Key getKey() const { return key_; };
        Value getValue() const { return value_; };
        void setValue(const Value &value) { value_ = value; };
        size_t getAccessCount() const { return accessCount_; };
        void incrementAccessCount() { ++accessCount_; };
        // 每个lru缓存器都可以访问结点类
        friend class LruCache<Key, Value>; // 添加朋友类，可以随意访问node类
    };
    // 定义lru缓存器继承抽象类，必须实现纯虚函数，不然子类也是抽象类，不能被实例化
    template <typename Key, typename Value>
    class LruCache : public KICachePolicy<Key, Value>
    {
        using LruNodeType = LruNode<Key, Value>;
        using NodePtr = std::shared_ptr<LruNodeType>;
        using NodeMap = std::unordered_map<Key, NodePtr>;

    private:
        int capacity_;
        NodeMap nodeMap_;
        std::mutex mutex_;
        // 定义缓存系统的头尾结点
        NodePtr dummyHead_;
        NodePtr dummyTail_;

    public:
        // 定义别名 定义内存智能指针 以及map可以快速定位到缓存中内存的位置

        // 构造缓存空间
        LruCache(int capacity) : capacity_(capacity)
        {
            initializeList();
        }
        // 重写父类的虚函数，实现lru缓存器的put方法
        void put(Key key, Value value) override
        {
            if (capacity_ <= 0)
                return;
            // 条件变量的锁，每次只能有一个线程去put新的页面数据到缓存系统中
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                // 新需要传入的数据在缓存中存在则更新数据
                updateExistingNode(it->second, value);
                return;
            }
            // 否则加入新的数据
            addNewNode(key, value);
        }
        bool get(Key key, Value &value) override
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                // 将这个key对应得结点移动到最新的位置尾部
                moveToMostRecent(it->second);
                // 记录当前的value值
                value = it->second->getValue();
                return true;
            }
            return false;
        }
        Value get(Key key) override
        {
            Value value{};
            // 去缓存系统中查找key对应的value值
            get(key, value);
            return value;
        }
        void remove(Key key)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                removeNode(it->second); // 删除双向链表中得结点
                nodeMap_.erase(it);     // 并删除映射
            }
        }

    private:
        void initializeList()
        {
            // 创建首尾虚拟节点 创建一个共享指针指向得对象
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyHead_->next_ = dummyTail_;
            dummyTail_->prev_ = dummyHead_;
        }
        void updateExistingNode(NodePtr node, const Value &value)
        {
            node->setValue(value);
            // 移动结点
            moveToMostRecent(node);
        }
        void addNewNode(const Key &key, const Value &value)
        {
            // 如果缓存已满，则移动出最近最少访问的结点
            if (nodeMap_.size() >= capacity_)
            {
                evictLeastNodePtr();
            }
            NodePtr newNode = std::make_shared<LruNodeType>(key, value);
            // 将新的结点加入到缓存系统中
            insertNode(newNode);
            // 将新的结点加入到map中，并相当于引用了node
            nodeMap_[key] = newNode;
        }
        void moveToMostRecent(NodePtr node)
        {
            // 先删除结点
            removeNode(node);
            // 再插入结点
            insertNode(node);
        }
        void removeNode(NodePtr node)
        {
            node->prev_->next_ = node->next_;
            node->next_->prev_ = node->prev_;
        }
        void insertNode(NodePtr node)
        {
            node->next_ = dummyTail_;
            node->prev_ = dummyTail_->prev_;
            dummyTail_->prev_->next_ = node;
            dummyTail_->prev_ = node;
        }
        void evictLeastNodePtr()
        {
            NodePtr leastRecentNode = dummyHead_->next_;
            removeNode(leastRecentNode);
            // leastRecent引用计数为0，自动释放内存
            nodeMap_.erase(leastRecentNode->getKey());
        }
    };

}
