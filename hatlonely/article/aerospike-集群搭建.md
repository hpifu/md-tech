---
title: aerospike 集群搭建
date: 2018-06-02 18:36:09
tags: [aerospike, kv存储, 数据库]
---

aerospike 是一个分布式的 kv 存储服务，与 redis，memcached 等相比，最大的特点是支持内存和磁盘的混合存储，并且对 ssd 的支
持非常好，将索引存在内存中，数据存在 ssd 中，保持极高性能的同时，能有效的节约成本

### 单机安装

安装

```
wget -O aerospike.tgz 'https://www.aerospike.com/download/server/latest/artifact/el6'
tar -xvf aerospike.tgz
cd aerospike-server-community-*-el6
sudo ./asinstall
```

启动

```
sudo service aerospike start
```

查看日志

```
tail -f /var/log/aerospike/aerospike.log
```

### 集群安装

首先用上面的步骤在每个节点安装 aerospike，再修改配置文件 `/etc/aerospike/aerospike.conf`，添加集群的地址列表，启动服务即可

下面是一个配置文件样例

```
service {
    user root
    group root
    paxos-single-replica-limit 1
    pidfile /var/run/aerospike/asd.pid
    proto-fd-max 15000
}

logging {
    file /var/log/aerospike/aerospike.log {
        context any info
    }
}

network {
    service {
        address 0.0.0.0
        port 3000
        # add current node address here
        access-address 172.31.25.40 3002
    }

    heartbeat {
        mode mesh
        # add current node address here        
        address 172.31.25.40
        port 3002
        # add all cluster node address here
        mesh-seed-address-port 172.31.25.40 3002
        mesh-seed-address-port 172.31.23.48 3002
        mesh-seed-address-port 172.31.19.27 3002
        interval 150
        timeout 10
    }

    fabric {
        address any
        port 3001
    }

    info {
        address any
        port 3003
    }
}

namespace test {
    memory-size 8G
    storage-engine memory
}
```

### 参考链接

- Install on Red Hat: <https://www.aerospike.com/docs/operations/install/linux/el6>
- Network Heartbeat: <https://www.aerospike.com/docs/operations/configure/network/heartbeat/index.html>
