#pragma once

namespace KamaCache
{
    // 这些纯虚函数使得 CachePolicy 成为一个抽象类，不能直接实例化，必须由派生类实现这些方法。
    //  缓存策略接口 相当于一个抽象类，派生类必须实现纯虚函数
    template <typename Key, typename Value>
    class KICachePolicy
    {
    public:
        // 基类虚析构函数，确保通过基类指针删除派生类对象可以正确的执行派生类等饿析构函数
        virtual ~KICachePolicy() {};

        // 添加缓存接口
        virtual void put(Key key, Value value) = 0;

        // key是传入参数  访问到的值以传出参数的形式返回 | 访问成功返回true
        virtual bool get(Key key, Value &value) = 0;
        // 如果缓存中能找到key，则直接返回value
        virtual Value get(Key key) = 0;
    };

} // namespace KamaCache