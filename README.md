# Multi-threaded tcp network server
Introduction
-------------
此项目是一个多线程高并发web服务器，并发模型使用了半同步/半反应堆模式（reactor模式）+非阻塞IO+主线程事件循环负责监听所有socket上的事件+工作线程处理客户逻辑；可处理静态资源，解析了httpget等请求，持HTTP连接。<br>

Start server
-------------
./sever [-t thread_number] [-p port] []


