---
title: docker consul 环境搭建
date: 2019-11-07 16:58:32
tags: [docker, consul, 环境搭建, 分布式]
---

和 etcd 类似，consul 也是一个高可用的分布式 kv 系统，并且提供了比 etcd 更丰富的功能，自带 ui 界面

consul 有客服端模式（client）和服务端模式（server），在实际的部署中会在每个客户端机器上部署一个本地 client, server 的数据会同步到 client，服务直接和本地 client 交互，有效地减少了网络通信

## 搭建 consul 服务

``` yml
version: "3.5"
services:
  consul:
    hostname: consul
    image: consul:1.6.1
    deploy:
      replicas: 1
      restart_policy:
        condition: on-failure
    # ports:
    #   - "8500:8500"
    #   - "8600:8600"
    user: root
    volumes:
      - "/var/docker/consul/data:/consul/data"
    environment:
      - CONSUL_BOOTSTRAP_EXPECT=1
      - CONSUL_CLIENT_LAN_ADDRESS=0.0.0.0
      - CONSUL_ENABLE_UI=true
      - CONSUL_ENABLE_SERVER=true
    networks:
      - consulnet

networks:
  consulnet:
    name: consulnet
```

## 链接

- github consul: <https://github.com/hashicorp/consul>
- docker consul 镜像: <https://hub.docker.com/_/consul>
- docker consul github: <https://github.com/hashicorp/docker-consul>
- 参考代码: <https://github.com/hpifu/docker-consul>
- docker consul 集群参考: <https://github.com/bitnami/bitnami-docker-consul/blob/master/docker-compose-cluster.yml>
