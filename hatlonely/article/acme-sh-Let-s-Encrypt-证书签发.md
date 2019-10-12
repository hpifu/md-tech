---
title: acme.sh Let's Encrypt 证书签发
date: 2019-07-13 02:27:51
tags: [ssl证书, https, acme.sh, Let's Encrypt]
---

Let's Encrypt 可以颁发免费的 ssl 证书，而 acme.sh 可以完成证书的申请，获取，续费的自动化，使用起来非常方便

## 安装

```
curl https://get.acme.sh | sh
```

这个安装会做三件事情：

1. 将脚本 `acme.sh` 下载到 `~/.acme.sh/`，所有的证书也将放到这个目录里面
2. `alias acme.sh=~/.acme.sh/acme.sh`
3. 创建一个 crontab 任务，检查以及更新证书

## 获取证书

支持 nginx 的自动配置，这种安装方式依赖 `nginx` 命令以及，`/etc/nginx/conf.d` 目录下的 `blog.hatlonely.com` 的配置

```
acme.sh --issue -d blog.hatlonely.com --nginx
```

## 安装证书

```
mkdir -p /etc/nginx/ssl/blog.hatlonely.com/
acme.sh --installcert -d blog.hatlonely.com \
        --key-file /etc/nginx/ssl/blog.hatlonely.com/blog.hatlonely.com.key \
        --fullchain-file /etc/nginx/ssl/blog.hatlonely.com/fullchain.cer \
        --reloadcmd  "service nginx force-reload"
```

证书被安装在 `/etc/nginx/ssl/blog.hatlonely.com` 目录下，并且过去会自动更新

## nginx 配置

```
upstream mini {
    keepalive 32;
    server 61.48.44.191:6000;
}

server {
    listen 443 ssl;
    server_name blog.hatlonely.com;

    ssl on;
    ssl_certificate      /etc/nginx/ssl/blog.hatlonely.com/fullchain.cer;
    ssl_certificate_key  /etc/nginx/ssl/blog.hatlonely.com/blog.hatlonely.com.key;

    access_log /var/log/nginx/wordpress.log;
    error_log /var/log/nginx/wordpress.err;

    location / {
        proxy_pass http://mini;
        proxy_redirect off;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $remote_addr;
        proxy_set_header X-Forwarded-Proto $scheme;

        proxy_buffering on;
        proxy_buffers 12 12k;
    }
}

server {
    listen 80;
    server_name blog.hatlonely.com;
    return 301 https://$server_name$request_uri;
}
```

## 常见问题

### 自动更新证书失败

可能是因为 acme.sh 需要更新，可用 `acme.sh --upgrade` 执行更新后再尝试，`acme.sh --upgrade --auto-upgrade` 设置自动更新 acme.sh

### 手动强制更新证书

```
acme.sh --renew -d blog.hatlonely.com
acme.sh --renew -d blog.hatlonely.com [--force]
```

## 链接

- github: <https://github.com/Neilpang/acme.sh>
- github 中文文档: <https://github.com/Neilpang/acme.sh/wiki/%E8%AF%B4%E6%98%8E>
- 快速签发 Let's Encrypt 证书指南: <https://www.cnblogs.com/esofar/p/9291685.html>
