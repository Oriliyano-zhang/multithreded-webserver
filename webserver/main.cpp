#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <iostream>
#include "http.h"
#include "threadpool.h"
#include "locker.h"

#define MAX_FD 65536
#define MAXSIZE 2000

void show_error(int connfd, const char* info)
{
    printf("%s", info);
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int main(int argc, const char *argv[])
{
    if (argc<3)
    {
        printf("./a.out port path\n");
        return -1;
    }
    int port = atoi(argv[1]);

    int ret = chdir(argv[2]);
    if (ret == -1)
    {
        perror("chdir error");
        exit(1);
    }

    /*创建线程池*/
    threadpool<Http>* pool = NULL;
    try
    {
    	pool = new threadpool<Http>;
    }
    catch (...)
    {
    	return 1;
    }
    /*预先为每个可能的客户连接分配一个Http对象*/
    Http* users = new Http[MAX_FD];
    assert(users);

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket error");
        exit(1);
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /*端口复用套接字选项*/
    int flag = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    ret = bind(lfd, (struct sockaddr*)&server, sizeof(server));
    if (ret == -1)
    {
        perror("bind error");
        exit(1);
    }
    ret = listen(lfd, 64);
    if (ret == -1)
    {
        perror("listen error");
        exit(1);
    }
    epoll_event ev;
    epoll_event all[MAXSIZE];
    int epfd = epoll_create(MAXSIZE);
    if (epfd == -1)
    {
        perror("epoll_create error");
        exit(1);
    }
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if (ret == -1)
    {
        perror("epoll_ctl error");
        exit(1);
    }
    Http::m_epollfd = epfd;
    while (true)
    {
        int ret = epoll_wait(epfd, all, MAXSIZE, -1);
        if (ret == -1)
        {
            perror("epoll_wait error");
            exit(1);
        }
        for (int i = 0; i<ret; i++)
        {
            int fd = all[i].data.fd;
            if (fd == lfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(lfd, (struct sockaddr*)&client_address, &client_addrlength);
                if (connfd < 0)
                {
                    printf("errno is: %d\n", errno);
                    continue;
                }
                if( Http::m_user_count >= MAX_FD  )
                {
                    show_error( connfd, "Internal server busy"  );
                    continue;
                }
                /*初始化客户连接*/
                users[connfd].init(connfd, client_address);
            }
            else if(all[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                /*如果有异常，直接关闭客户连接*/
                users[fd].disconnected(fd,epfd);
            }
            else if (!(all[i].events&EPOLLIN))
            {
                continue;
            }
            else
            {
				        pool->append(users + fd);
            }
        }
    }
    close(epfd);
    close(lfd);
    delete[] users;
    delete pool;
    return 0;
}
