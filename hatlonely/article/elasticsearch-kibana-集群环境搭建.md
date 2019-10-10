---
title: elasticsearch + kibana 集群环境搭建
date: 2018-04-12 22:06:36
tags: [elasticsearch, kibana, 日志收集, 日志分析, 环境搭建]
thumbnail: /img/thumbnail/dusk.jpg
---

elk 提供了完备且成熟的日志存储和分析的解决方案，免费开源，本文主要介绍 elasticsearch 集群以及 kibana 的环境搭建

### elasticsearch

elasticsearch 可以理解为一个支持模糊查询的数据库，用来存储日志

#### 下载 elasticsearch

```
wget https://artifacts.elastic.co/downloads/elasticsearch/elasticsearch-6.2.3.tar.gz
tar -xzvf elasticsearch-6.2.3.tar.gz
```

#### 修改配置文件

修改 `config/elasticsearch.yml`

```
cluster.name: rlogger       # 集群名称，所有节点统一一个
node.name: node-2           # 节点名称，每个节点一个即可
network.host: 0.0.0.0       # 绑定的地址，设置成 0.0.0.0 表示接受来自任何地址的请求
http.port: 9200
discovery.zen.ping.unicast.hosts: ["172.31.22.165", "172.31.17.241", "172.31.30.84", "172.31.18.178"]
discovery.zen.minimum_master_nodes: 3
```

如果需要修改 java 堆栈区大小，可以修改 `config/jvm.options`

```
-Xms8g
-Xmx8g
```

#### 启动 elasticsearch

```
nohup bin/elasticsearch &
```

节点之间的 9200 端口需要互通

#### 检查是否生效

```
curl -XGET 'http://172.31.17.241:9200/_cat/nodes?pretty'
```

会有如下输出

```
172.31.18.178 61 99 0 0.01 0.04 0.09 mdi - node-4
172.31.17.241 27 99 0 0.06 0.04 0.05 mdi - node-1
172.31.30.84  27 99 0 0.06 0.07 0.06 mdi - node-2
172.31.22.165 68 99 0 0.00 0.01 0.05 mdi * node-3
```

### kibana

kibana 通过一个 web 界面提供日志的检索分析功能，还可以绘制各种图表，非常方便

#### 下载 kibana

```
wget https://artifacts.elastic.co/downloads/kibana/kibana-6.2.3-linux-x86_64.tar.gz
tar -xzvf kibana-6.2.3-linux-x86_64.tar.gz
```

#### 修改配置文件

修改 `config/kibana.yml`

```
server.host: "0.0.0.0"      # 绑定的地址，设置成 0.0.0.0 表示接受来自任何地址的请求
elasticsearch.url: "http://localhost:9200"
```

由于我的 kibana 直接部署在 elasticsearch 节点机器上，所以这里 `elasticsearch.url` 配成默认的就行

#### 启动 kibana

```
nohup bin/kibana &
```

注意开放 5601 端口

#### 检查是否生效

在浏览器里面访问 `http://<kibana ip>:5601`，即可

到这里 elasticsearch + kibana 集群环境已经搭建完成了，下面这个 metricbeat 是机器基本指标的一些监控，在上面 kibana 地址的首页就有引导

### metricbeat

metricbeat 会自动收集一些机器指标发到 elasticsearch，并在 kibana 有可视化的图表展示

#### 下载 metricbeat

```
curl -L -O https://artifacts.elastic.co/downloads/beats/metricbeat/metricbeat-6.2.3-x86_64.rpm
sudo rpm -vi metricbeat-6.2.3-x86_64.rpm
```

#### 修改配置文件

修改 `/etc/metricbeat/metricbeat.yml`

```
output.elasticsearch:
  hosts: ["172.31.17.241:9200", "172.31.30.84:9200", "172.31.22.165:9200", "172.31.18.178:9200"]
setup.kibana:
  host: "172.31.17.241:5601"
```

#### 启动 metricbeat

```
sudo metricbeat modules enable system
sudo metricbeat setup
sudo service metricbeat start
```

#### 查看机器状态

现在可以在 kibana 的 `Dashboard` 里面看到 `[Metricbeat System] Host overview`，点进去就可以看到 cpu 内存情况了

### 参考链接

- elasticsearch 下载: <https://www.elastic.co/cn/downloads/elasticsearch>
- kibana 下载: <https://www.elastic.co/cn/downloads/kibana>
