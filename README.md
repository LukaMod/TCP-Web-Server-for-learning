# TCP-Web-Server-for-learning

项目包含一个基于 `kqueue` 的非阻塞 I/O 并发服务器，《UNP》客户/服务器设计范式中介绍的每个连接一个子进程的并发服务器，预先分配线程、每个线程各自 `accept` 的并发服务器以及一个多进程发送多个请求的测试客户端。

服务器处理以 `\n` 结尾的一定数量的字符请求。

# Build

CMake 外部构建：
```shell
$ mkdir build && cd build && cmake ..
```

生成基于 `kqueue` 的服务器：
```shell
$ make kq_server
```

生成多进程服务器：
```shell
$ make serv_fork_per
```

生成多线程服务器：
```shell
$ make serv_prethread
```

生成客户端：
```shell
$ make client
```

# Usage

`kq_server`:
```shell
$ kq_server [ <host> ] <port>
```

`serv_fork_per`:
```shell
$ serv_fork_per [ <host> ] <port>
```

`serv_prethread`:
```shell
$ serv_prethread [ <host> ] <port> <threads>
```

`client`:
```shell
$ client <hostname or IPaddress> <port> <#children> <#loops/child> <#bytes/request>
```
