# A multi-threaded web server
Introduction
-------------
此项目是一个多线程高并发web服务器，并发模型使用了半同步/半反应堆模式（reactor模式）+非阻塞IO+主线程事件循环+工作线程处理客户逻辑，可处理静态资源，解析了httpget等请求，支持HTTP连接。<br>

Start server
-------------
./sever [-t thread_number] [-p port] [-l path]

Technical
----------
* 服务器框架采用Reactor模式，采用epoll边沿触发模式作为IO复用技术作为IO分配器，分发IO事件<br>
* 对于IO密集型请求使用多线程充分利用多核CPU并行处理，创建了线程池避免线程频繁创建销毁的开销<br>
* 主线程只负责accept请求，并以轮询的方式分发给其它IO线程，然后执行read->decode->compute->encode->write<br>
* 主线程维持了一个事件循环(eventloop)，工作线程同步的处理客户逻辑<br>
* 设计了任务队列的缓冲区机制，避免了请求陷入死锁循环<br>
* 线程间的高效通信，使用信号量等锁机制实现了线程的异步唤醒<br>


