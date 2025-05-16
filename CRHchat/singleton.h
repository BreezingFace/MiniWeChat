#ifndef SINGLETON_H
#define SINGLETON_H
#include "global.h"
//using namespace std;

template <typename T>
class Singleton{
protected:
    Singleton()=default;
    Singleton(const Singleton<T>& st)=delete;
    Singleton& operator=(const Singleton<T>& st)=delete;
    static std::shared_ptr<T> _instance;//静态的_instance，表明_instance属于类，该类所有对象共享一个_instance

public:
    static std::shared_ptr<T> GetInstance(){
        static std::once_flag s_flag;//只会在第一次调用函数时初始化,初始值为false
        std::call_once(s_flag,[&](){
            //思考，为什么不用make_shared？因为构造函数的访问权限是protected。std::make_shared<T> 需要直接访问 T 的构造函数。
            //但单例模式通常把构造函数设为 protected 或 private（防止外部创建实例）。
            /// 然而，make_shared 是 外部函数，不能访问受保护或私有的构造函数
            //  而用 std::shared_ptr<T>(new T) 直接用 new 创建对象，是在 Singleton 类内部，可以访问受保护或私有的构造函数！
            _instance = std::shared_ptr<T>(new T);
        });
        return _instance;
    }

    ~Singleton(){
        std::cout<<"this is singleton destruct."<<std::endl;
    }

    void PrintAddress(){
        std::cout<<_instance.get()<<std::endl;
    }

};
template <typename T>
std::shared_ptr<T> Singleton<T> ::_instance=nullptr;
#endif // SINGLETON_H
