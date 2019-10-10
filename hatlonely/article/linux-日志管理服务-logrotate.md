---
title: linux 日志管理服务 logrotate
date: 2018-04-16 14:14:54
tags: [linux, logrotate, 日志]
---

logrotate 是操作系统用来管理日志的工具，支持日志的切割、压缩、清理以及邮件报警等，通过 crontab 服务定时运行，也可以用这个工具来管理我们自己的服务日志，只需要简单配置下配置文件即可

### logrotate 配置

`helloworld.conf`，多份日志可以直接写在一个配置文件里面，也可以分成多个文件 `include` 进来

```
/path/to/log/hello.log
/path/to/log/world.err
{
    hourly
    rotate 24
    notifempty
    nocompress
    missingok
    copytruncate
    dateext
    dateformat .%Y%m%d%H
    olddir /path/to/log/bk/
    postrotate
        echo "update to aws s3"
    endscript
}
```

- `hourly`: 日志切割按照小时来切割，logrotate 3.9.0 以后版本才支持，但是其实真正的调度是在 crontab 里面配置的
- `rotate`: 保留多少个历史文件
- `notifempty`: 如果文件为空，则不切分文件，默认是 `ifempty`
- `nocompress`: 不压缩文件
- `missingok`: 忽略文件缺失信息
- `copytruncate`: 创建一个日志的拷贝，并且截断老的日志，适合那种一直写一个文件，也不关闭的服务。由于这个阶段的过程会有一小段时间，可能会有数据丢失
- `dateext`: 使用日期后缀
- `dateformat`: 日期后缀的格式
- `olddir`: 备份目录
- `postrotate/endscript`: 切割后执行的命令，这个选项非常灵活，可以实现一些强大的功能，比如日志上传，日志分析等等

### 运行

直接执行 `logrotate -f helloworld.conf` 可以强制执行切割，测试是否生效

### 加入到 crontab

上面测试没有问题之后，`crontab -e` 加入到 crontab 里面即可

```
59 * * * * /path/to/logrotate /path/to/helloworld.conf >/dev/null 2>&1
```

### 参考链接

- logrotate man: <https://linux.die.net/man/8/logrotate>
- logrotate 日期后缀支持小时: <https://stackoverflow.com/questions/31132995/logrotate-dateformat-seems-not-supporting-hms>
