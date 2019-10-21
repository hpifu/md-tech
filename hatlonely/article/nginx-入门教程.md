---
title: nginx 入门教程
date: 2019-10-21 10:08:26
tags: [nginx, 反向代理, gzip]
---

nginx 是一个开源的高性能 web 服务器（可能是性能最好的），使用非常广泛，既可以用来部署静态资源，也可以用来作为反向代理，甚至可以作为负载均衡服务器。

## 安装和启动

``` bash
# 安装
yum install nginx

# 启动
service start nginx

# 重新加载配置
nginx -s reload
```

## 配置文件

默认的配置文件在 `/etc/nginx/nginx.conf`，这个文件是配置文件的入口，一般配置一些全局信息

```
user nginx;
worker_processes auto;

error_log /var/log/nginx/error.log warn;
pid /var/run/nginx.pid;

events {
    worker_connections 1024;
}

http {
    include /etc/nginx/mime.types;
    default_type application/octet-stream;

    log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
                      '$status $body_bytes_sent "$http_referer" '
                      '"$http_user_agent" "$http_x_forwarded_for"';

    access_log /var/log/nginx/access.log main;
    sendfile on;
    keepalive_timeout 65;

    include /etc/nginx/conf.d/*.conf;
}
```

这个配置文件里面一般会有一句 `include /etc/nginx/conf.d/*.conf`，包含各个子服务的配置信息

## 静态网站

只需要在 `/etc/nginx/conf.d` 中新增一个文件 `example.com.conf`

```
server {
    listen 80;
    # 这里设置服务的名字
    server_name example.com;

    access_log /var/log/nginx/example.log main;
    error_log /var/log/nginx/example.err main;

    location / {
        # 这里设置静态资源的路径
        root /var/www/example;
        try_files $uri $uri/ /index.html;
        index index.html index.htm;
    }
}
```

- `server_name`: 服务的名字，用户通过这个名字访问服务，不同的服务使用不同的 server_name 区分，同一个 nginx 实例下可部署多个服务
- `root`: 静态资源路径
- `access_log`: 日志输出路径，日志格式通过最后一个参数指定，这里的 main 为日志格式名，来自于上一个配置文件中的 `log_format`

## 反向代理

同样在 `/etc/nginx/conf.d` 中新增一个文件 `proxy.conf`

```
upstream yourservice {
    keepalive 32;
    server yourserver:<port>;
}

server {
    listen       80;
    server_name  proxy;

    access_log /var/log/nginx/proxy.log access;
    error_log /var/log/nginx/proxy.err;

    location / {
        proxy_pass http://yourservice;
        proxy_redirect off;
        proxy_set_header Host $host;
        proxy_set_header User-Agent $http_user_agent;
        proxy_set_header X-Real-IP $http_x_real_ip;
        proxy_set_header X-Forwarded-For $http_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

- `upstream`: 定义一个服务，`server` 里面可以指定多个后端服务地址，利用 nginx 作负载均衡
- `proxy_pass`: 反向代理到我们定义的服务中
- `proxy_set_header`: 请求服务时设置一些头部字段，比如 userAgent 和 客户端 ip

客户端 ip 一般设置在 X-Real-IP 和 X-Forwarded-For

## 内置变量

nginx 的配置文件中提供了大量的内置变量

- `$romote_addr`: 客户端 ip 地址
- `$remote_user`: 客户端用户名称
- `$time_local`: local 格式时间
- `$time_iso8601`: iso8601 格式时间
- `$scheme`: http 协议
- `$request`: 请求的 url 和 http 协议
- `$status`: 返回的状态码
- `$body_bytes_sent`: 返回的包体大小
- `$http_referer`: 请求页面来源，header["Referer"]
- `$http_user_agent`: 浏览器信息，header["User-Agent"]
- `$http_x_real_ip`: 用户真实 ip，header["X-Real-IP"]
- `$http_x_forwarded_for`: 代理过程，header["X-Forwarded-For"]

## json 日志

如果有日志分析的需求，最好使用 json 格式的日志，可以通过 `log_format` 命令自定义日志格式

```
http {
    log_format access escape=json
            '{'
                '"@timestamp":"$time_iso8601",'
                '"remoteAddr":"$remote_addr",'
                '"remoteUser":"$remote_user",'
                '"xRealIP":"$http_x_real_ip",'
                '"xForwardedFor":"$http_x_forwarded_for",'
                '"request":"$request",'
                '"status": $status,'
                '"bodyBytesSent":"$body_bytes_sent",'
                '"resTimeS":"$request_time",'
                '"referrer":"$http_referer",'
                '"userAgent":"$http_user_agent"'
            '}';
}
```

这个日志格式可以定义在 `/etc/nginx/nginx.conf` 的 http 字段中，各个 server 就可以直接引用

```
server {
    access_log /var/log/nginx/example.log access;
}
```

如果希望日期格式显示为北京时间，需要设置一下时区

``` bash
ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
echo "Asia/Shanghai" >> /etc/timezone
```

## gzip 压缩

返回的静态资源比较大，带宽成为瓶颈，可以考虑开启 gzip 压缩，200k 的文件能压缩到 几十k，效果还挺明显的，开启 gzip 的配置也很简单，直接修改 `/etc/nginx/nginx.conf` 即可

```
http {
    gzip on;
    gzip_min_length 1k;
    gzip_buffers 4 16k;
    gzip_comp_level 8;
    gzip_types text/plain application/javascript application/x-javascript text/css application/xml text/javascript;
    gzip_vary on;
    gzip_disable "MSIE [1-6]\.";
}
```

## 链接

- 代码链接: <https://github.com/hpifu/docker-nginx>
- 官方初学者教程: <http://nginx.org/en/docs/beginners_guide.html>
- 官方文档: <http://nginx.org/en/docs/>
- nginx 源码: <https://github.com/nginx/nginx>
