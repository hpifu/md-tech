---
title: docker etcd 环境搭建
date: 2019-11-07 16:58:32
tags: [docker, etcd, 环境搭建]
---

etcd 是用 golang 编写，raft 协议实现的分布式高可靠的 kv 存储系统，常用来作为配置共享和服务注册发现，Kubernetes 就使用 etcd 来保存整个集群的状态

etcd 主要关注以下四点:

1. 简单: 定义良好，面向用户的 api (使用 grpc)
2. 安全: 可选的 tls 用户认证方式
3. 快速: 支持每秒 10000 次的写入
4. 可靠: raft 协议实现分布式

## 搭建 etcd 服务

``` yml
version: "3.5"
services:
  etcd:
    hostname: etcd
    image: bitnami/etcd:3
    deploy:
      replicas: 1
      restart_policy:
        condition: on-failure
    # ports:
    #   - "2379:2379"
    #   - "2380:2380"
    #   - "4001:4001"
    #   - "7001:7001"
    user: root
    volumes:
      - "/var/docker/etcd/data:/opt/bitnami/etcd/data"
    environment:
      - "ETCD_ADVERTISE_CLIENT_URLS=http://etcd:2379"
      - "ETCD_LISTEN_CLIENT_URLS=http://0.0.0.0:2379"
      - "ETCD_LISTEN_PEER_URLS=http://0.0.0.0:2380"
      - "ETCD_INITIAL_ADVERTISE_PEER_URLS=http://0.0.0.0:2380"
      - "ALLOW_NONE_AUTHENTICATION=yes"
      - "ETCD_INITIAL_CLUSTER=node1=http://0.0.0.0:2380"
      - "ETCD_NAME=node1"
      - "ETCD_DATA_DIR=/opt/bitnami/etcd/data"
    networks:
      - etcdnet

networks:
  etcdnet:
    name: etcdnet
```

客户端通过 `ETCD_ADVERTISE_CLIENT_URLS` 这个地址和 etcd 建立连接

## etcd 命令行界面 etcdctl

etcdctl 是 etcd 提供的命令行界面，可以方便地管理 etcd 服务

``` sh
docker exec $(docker ps -a --filter name=etcd -q) etcdctl put root/service/svc1/key1 val1
docker exec $(docker ps -a --filter name=etcd -q) etcdctl get root/service/svc1/key1
```

## etcd ui 界面 e3w

e3w 是一个 etcd 的 ui 界面，可以方便地查看和管理当前的 kv 和各节点的状态

``` yml
version: "3.5"
services:
  e3w:
    hostname: e3w
    image: soyking/e3w:latest
    deploy:
      replicas: 1
      restart_policy:
        condition: on-failure
    # ports:
    #   - "8080:8080"
    volumes:
      - "/var/docker/e3w/conf/config.ini:/app/conf/config.default.ini"
    networks:
      - e3wnet
      - etcdnet

networks:
  e3wnet:
    name: e3wnet
  etcdnet:
    external: true
    name: etcdnet
```

e3w 管理的 kv 是目录格式的，且所有的 key 都带有根目录前缀，根目录可以在 `config.ini` 中配置

比如 `root_key=root`，创建目录 `service/svc1`，创建 `key1 => val1`，实际在 etcd 的存储的 key 为 `root/service/svc1/key1`

## 链接

- etcd 简介 + 使用: <https://blog.csdn.net/bbwangj/article/details/82584988>
- github etcd: <https://github.com/etcd-io/etcd>
- github ui 界面 e3w: <https://github.com/soyking/e3w>
- etcd 配置: <https://github.com/etcd-io/etcd/blob/master/Documentation/op-guide/configuration.md>
- etcd 命令行界面: <https://github.com/etcd-io/etcd/tree/master/etcdctl>
- etcd 环境完整代码参考: <https://github.com/hatlonely/docker-etcd>
- e3w 环境完整代码参考: <https://github.com/hatlonely/docker-e3w>
- docker etcd 镜像: <https://hub.docker.com/r/bitnami/etcd>
- docker e3w 镜像: <https://hub.docker.com/r/soyking/e3w>
