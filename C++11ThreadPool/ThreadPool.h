#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

class ThreadPool{
public:
    ThreadPool(int num);

    void init();

    template<class Fn, class...Args>
    auto push(Fn&& fn, Args&&... args) ->
        std::future<typename std::result_of<Fn(Args...)>::type>;

    ~ThreadPool();
private:
    int thN;                                  //池中的线程数

    std::vector<std::thread> workers;         //工作线程队列
    std::queue<std::function<void()>> tasks;  //具体任务队列

    std::mutex qLock;                         //访问队列时用的锁，队列是公共资源
    std::condition_variable cond;             //用户判断worker线程(工作\休眠)的条件变量

    bool terminal;                            //线程池的运行状态
};

ThreadPool::ThreadPool(int num):thN(num),terminal(false) {
    std::cout << "ThreadPool Init : threadNum = " << this->thN << std::endl;
}

void ThreadPool::init() {
    //循环初始化每一个线程对象
    for(int i=0;i<this->thN;i++) {
        workers.emplace_back([this,i]{
            std::cout << "Create ThreadId = " << i << std::endl;
            while( !terminal ) {
                std::function<void()> task;
                {
                    //上锁检查当前状态
                    std::unique_lock<std::mutex> uniLock(this->qLock);
                    this->cond.wait(uniLock,[this]{
                        return ( this->terminal || !this->tasks.empty() );
                    });

                    //worker取任务
                    if( !tasks.empty() ) {
                        task=std::move(tasks.front());
                        //为什么需要使用std::move？
                        //因为task是一个指针对象，是原本的用户任务对象传递下来的，需要保证对象是原来那一个
                        this->tasks.pop();
                    }
                }
                if( task != nullptr ) task();
                std::cout << "ThreadId = " << i << ", Dealing with the Work!" << std::endl;
            }
        });
    }
}

template<class Fn, class... Args>
auto ThreadPool::push(Fn&& fn, Args&&... args) ->
    std::future<typename std::result_of<Fn(Args...)>::type> {
    using return_type = typename std::result_of<Fn(Args...)>::type;
    //1. 把用户的任务转换成一个task
    auto task=std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<Fn>(fn),std::forward<Args>(args)...)
            );
    //为什么需要使用（share_ptr）?
    //因为task需要获取到对应的返回值，需要对原来的对象进行操作。所以把它设置为指针对象。
    //而且这个对象会经历：push(加入tasks队列) -> handler(从task队列下来) -> 把结果放入future中

    //2. 把task加入到tasks中，并通知worker
    std::future<return_type> res=task->get_future();  //记录task的返回值，用于最后返回
    {
        std::unique_lock<std::mutex> uniLock(this->qLock);

        if( this->terminal )
            throw std::runtime_error("Push On Stopped ThreadPool！");

        this->tasks.emplace([task]{(*task)();});
    }
    this->cond.notify_one();

    //3. 返回运行结果
    return res;
}

ThreadPool::~ThreadPool() {
    //1.结束标志置位，并且通知全部 worker 线程结束
    this->terminal=true;
    this->cond.notify_all();

    //2.等待回收所有线程
    for(auto& worker: this->workers)
        worker.join();
}

#endif /*_THREADPOOL_H*/
