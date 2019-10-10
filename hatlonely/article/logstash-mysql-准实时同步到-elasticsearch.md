---
title: logstash mysql 准实时同步到 elasticsearch
date: 2018-04-14 21:19:26
tags: [logstash, mysql, elasticsearch, 数据同步]
thumbnail: /img/thumbnail/ship2.jpg
---

mysql 作为成熟稳定的数据持久化解决方案，广泛地应用在各种领域，但是在数据分析方面稍有不足，而 elasticsearch 作为数据分析领域的佼佼者，刚好可以弥补这项不足，而我们要做的只需要将 mysql 中的数据同步到 elasticsearch 中即可，而 logstash 刚好就可以支持，所有你需要做的只是写一个配置文件而已

### logstash 获取

获取 logstash

```
wget https://artifacts.elastic.co/downloads/logstash/logstash-6.2.3.zip
unzip logstash-6.2.3.zip && cd logstash-6.2.3
```

安装 jdbc 和 elasticsearch 插件

```
bin/logstash-plugin install logstash-input-jdbc
bin/logstash-plugin install logstash-output-elasticsearch
```

获取 jdbc mysql 驱动

```
wget https://cdn.mysql.com//Downloads/Connector-J/mysql-connector-java-5.1.46.zip
unzip mysql-connector-java-5.1.46.zip
```

### 编写配置文件

#### logstash-input-jdbc

使用 logstash-input-jdbc 插件读取 mysql 的数据，这个插件的工作原理比较简单，就是定时执行一个 sql，然后将 sql 执行的结果写入到流中，增量获取的方式没有通过 binlog 方式同步，而是用一个递增字段作为条件去查询，每次都记录当前查询的位置，由于递增的特性，只需要查询比当前大的记录即可获取这段时间内的全部增量，一般的递增字段有两种，`AUTO_INCREMENT` 的主键 `id` 和 `ON UPDATE CURRENT_TIMESTAMP` 的 `update_time` 字段，`id` 字段只适用于那种只有插入没有更新的表，`update_time` 更加通用一些，建议在 mysql 表设计的时候都增加一个 `update_time` 字段

``` ruby
input {
  jdbc {
    jdbc_driver_library => "../mysql-connector-java-5.1.46/mysql-connector-java-5.1.46-bin.jar"
    jdbc_driver_class => "com.mysql.jdbc.Driver"
    jdbc_connection_string => "jdbc:mysql://<mysql_host>:3306/rta"
    jdbc_user => "<username>"
    jdbc_password => "<password>"
    schedule => "* * * * *"
    statement => "SELECT * FROM table WHERE update_time >= :sql_last_value"
    use_column_value => true
    tracking_column_type => "timestamp"
    tracking_column => "update_time"
    last_run_metadata_path => "syncpoint_table"
  }
}
```

- `jdbc_driver_library`: jdbc mysql 驱动的路径，在上一步中已经下载
- `jdbc_driver_class`: 驱动类的名字，mysql 填 `com.mysql.jdbc.Driver` 就好了
- `jdbc_connection_string`: mysql 地址
- `jdbc_user`: mysql 用户
- `jdbc_password`: mysql 密码
- `schedule`: 执行 sql 时机，类似 crontab 的调度
- `statement`: 要执行的 sql，以 ":" 开头是定义的变量，可以通过 parameters 来设置变量，这里的 `sql_last_value` 是内置的变量，表示上一次 sql 执行中 update_time 的值，这里 `update_time` 条件是 `>=` 因为时间有可能相等，没有等号可能会漏掉一些增量
- `use_column_value`: 使用递增列的值
- `tracking_column_type`: 递增字段的类型，`numeric` 表示数值类型, `timestamp` 表示时间戳类型
- `tracking_column`: 递增字段的名称，这里使用 update_time 这一列，这列的类型是 `timestamp`
- `last_run_metadata_path`: 同步点文件，这个文件记录了上次的同步点，重启时会读取这个文件，这个文件可以手动修改

#### logstash-output-elasticsearch

``` ruby
output {
  elasticsearch {
    hosts => ["172.31.22.165", "172.31.17.241", "172.31.30.84", "172.31.18.178"]
    user => "<user>"
    password => "<password>"
    index => "table"
    document_id => "%{id}"
  }
}
```

- `hosts`: es 集群地址
- `user`: es 用户名
- `password`: es 密码
- `index`: 导入到 es 中的 index 名，这里我直接设置成了 mysql 表的名字
- `document_id`: 导入到 es 中的文档 id，这个需要设置成主键，否则同一条记录更新后在 es 中会出现两条记录，`%{id}` 表示引用 mysql 表中 `id` 字段的值

### 运行

把上面的代码保存到一个配置文件里面 `sync_table.cfg`，执行下面命令即可

```
cd logstash-6.2.3 && bin/logstash -f config/sync_table.cfg
```

如果成功了会在标准输出输出执行的 sql 语句

```
[2018-04-14T18:12:00,278][INFO ][logstash.inputs.jdbc     ] (0.001011s) SELECT version()
[2018-04-14T18:12:00,284][INFO ][logstash.inputs.jdbc     ] (0.000723s) SELECT * FROM table WHERE update_time > '2018-04-14 17:55:00'
```

### 其他问题

#### 多表同步

一个 logstash 实例可以借助 pipelines 机制同步多个表，只需要写多个配置文件就可以了，假设我们有两个表 table1 和 table2，对应两个配置文件 `sync_table1.cfg` 和 `sync_table2.cfg`

在 `config/pipelines.yml` 中配置

```
- pipeline.id: table1
  path.config: "config/sync_table1.cfg"
- pipeline.id: table2
  path.config: "config/sync_table2.cfg"
```

直接 `bin/logstash` 启动即可

#### `@timestamp` 字段

默认情况下 `@timestamp` 字段是 logstash-input-jdbc 添加的字段，默认是当前时间，这个字段在数据分析的时候非常有用，但是有时候我们希望使用数据中的某些字段来指定这个字段，这个时候可以使用 filter.date, 这个插件是专门用来设置 `@timestamp` 这个字段的

比如我有我希望用字段 `timeslice` 来表示 `@timestamp`，`timeslice` 是一个字符串，格式为 `%Y%m%d%H%M`

``` ruby
filter {
  date {
    match => [ "timeslice", "yyyyMMddHHmm" ]
    timezone => "Asia/Shanghai"
  }
}
```

把这一段配置加到 `sync_table.cfg` 中，现在 `@timestamp` 和 `timeslice` 一致了

### 参考链接

- logstash-input-jdbc 插件: <https://www.elastic.co/guide/en/logstash/current/plugins-inputs-jdbc.html>
- logstash-output-elasticsearch 插件: <https://www.elastic.co/guide/en/logstash/current/plugins-outputs-elasticsearch.html>
- logstash-multiple-piplines: <https://www.elastic.co/blog/logstash-multiple-pipelines>
- logstash-filter-date 插件: <https://www.elastic.co/guide/en/logstash/current/plugins-filters-date.html>
