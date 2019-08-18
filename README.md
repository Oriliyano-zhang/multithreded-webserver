# Multi-threaded tcp network server
Introduction
-------------
此项目是一个多线程高并发web服务器，并发模型使用了半同步/半反应堆模式（reactor模式）+非阻塞IO+主线程事件循环负责监听所有socket上的事件+工作线程处理客户逻辑；可处理静态资源，解析了httpget等请求，持HTTP连接。<br>

Start server
-------------
./sever [-t thread_number] [-p port] []

Technical
----------
* 服务器框架采用Reactor模式，采用epoll边沿触发模式作为IO复用技术作为IO分配器，分发IO事件<br>
* 对于IO密集型请求使用多线程充分利用多核CPU并行处理，创建了线程池避免线程频繁创建销毁的开销<br>
* 主线程只负责accept请求，并以轮回的方式分发给其它IO线程，然后执行read->decode->compute->encode->write<br>
* 使用基于小根堆的定时器关闭超时请求<br>
* 主线程和工作线程各自维持了一个事件循环(eventloop)<br>
* TLS，使用了线程的本地局部存储功能，维护各个线程的运行状态以及运行信息等<br>
* 设计了任务队列的缓冲区机制，避免了请求陷入死锁循环<br>
* 线程间的高效通信，使用eventfd实现了线程的异步唤醒<br>
* 为减少内存泄漏的可能，使用智能指针等RAII机制<br>
* 使用状态机解析了HTTP请求,支持http管线化请求<br>


