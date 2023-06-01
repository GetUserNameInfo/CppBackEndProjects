基于C++11新特性写的线程池。

```c++
#include "ThreadPool.h"

ThreadPool* pool=new ThreadPool(4);

pool->init();

auot ret=pool->push([](int a,int n)->int{
    int sum=0;
    while( n-- ) sum+=a++;
    return sum;
},1,100);

std::cout << ret.get() << std::endl;
```





**学习笔记**

1. 线程池结构

![Thread_Pool](.imgs\Thread_Pool.jpg)

2. 重点

   - User给的任务与Pool内的Task需要进行适配。（通过std::bind）

   - std::packaged_task<type>的使用

     > packaged_task与function类似，但会把可调用对象的返回值返回到shared_state中，再通过 .get_future() 使得与 shared_state 进行共享。

   

