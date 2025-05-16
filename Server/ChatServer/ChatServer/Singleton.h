#pragma once
#include <memory>
#include <mutex>
#include <iostream>
using namespace std;
template <typename T>
/*
单例模式（Singleton）的核心作用：
全局唯一实例：保证一个类只有一个实例，避免重复创建。
全局访问点：通过 GetInstance() 方法获取该实例，方便统一管理。
节省资源：某些对象（如配置管理器、日志系统、数据库连接池）只需要一个实例。
*/
class Singleton {
protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>& st) = delete;
    static std::shared_ptr<T> _instance;
public:
    /*
    * std::once_flag + std::call_once 确保线程安全（即使多线程同时调用 GetInstance()，实例只会创建一次）。
    延迟初始化（Lazy Initialization）：实例在第一次调用 GetInstance() 时才创建。
    */
    static std::shared_ptr<T> GetInstance() {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            _instance = shared_ptr<T>(new T);
            });
        return _instance;
    }
    void PrintAddress() {
        std::cout << _instance.get() << endl;
    }
    ~Singleton() {
        std::cout << "this is singleton destruct" << std::endl;
    }
};
template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;