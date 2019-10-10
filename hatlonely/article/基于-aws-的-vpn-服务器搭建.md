---
title: 基于 aws 的 vpn 服务器搭建
date: 2018-03-04 13:53:58
tags: [vpn, aws]
---

搭 vpn 的原因主要有两个：

1. google，“内事不知问百度，外事不知问 google”，技术这件事情明显是外事，百度在技术搜索上面要不就是和问题不太相关，要不就是已经是过时的内容，确实体验比较差
2. 一些 golang 库没有办法通过 `go get` 正常下载

之前在淘宝上还可以买到这种 vpn 账号，现在也越来越难买了，还是自己搭一个比较省事

### 墙外的服务器

要搭 vpn 首先需要有一台墙外的服务器，香港的或者国外的都可以，操作系统建议安装 `CentOS 7`

这里我使用 aws 的服务，主要是可以免费一年，唯一需要的是绑定一个 visa 信用卡。然后创建一台 EC2 实例，选择 `Centos 7` 系统镜像，在 `AWS Marketplace` 中搜索 “centos” 关键字即可

### 安装 vpn 服务

github 上有个大神写了个一键安装的脚本：<https://github.com/hwdsl2/setup-ipsec-vpn/blob/master/README-zh.md>，直接执行下列命令即可完成安装: 

```
wget https://git.io/vpnsetup-centos -O vpnsetup.sh && sudo sh vpnsetup.sh
```

安装成功后，会在屏幕上显示 vpn 凭证

```
Server IP: xx.xx.xx.xx
IPsec PSK: xxxxxxxxxxx
Username: vpnuser
Password: xxxxxxxxxxxx
```

现在 vpn 服务还不能访问，需要将 vpn 服务端口对外网开放，vpn 服务使用到的端口有 500/4500/50/51/1701，通过设置安全组可以开放这些端口，也可以暴力一点，开发所有端口

### 客户端访问

#### Mac

1. `打开系统偏好设置` → `网络`
2. 点 `+` 添加网络，接口选 `vpn`，vpn 类型选 `IPSec 上的 L2TP`，服务名称随意
3. 选中刚刚创建的网络，将上面生成的 vpn 凭证填入，服务器地址填 `Server Ip`，账户名称填 `Username`，点开鉴定设置，密码填 `Password`，共享的秘钥填 `IPsec PSK`
4. 点击 `连接` 即可

#### iOS

1. `设置` → `通用` → `vpn`
2. 添加 vpn 配置 → 类型选择 `L2TP` → 填入上述 vpn 凭证即可

### 添加用户

直接编辑 `/etc/ppp/chap-secrets` 文件，新增用户即可，无需重启服务

```
sudo vim /etc/ppp/chap-secrets
```

文件样例如下:

```
"user1" l2tpd "password1" *
"user2" l2tpd "password2" *
"user3" l2tpd "password3" *
```

### 参考链接

- IPsec VPN 服务器一键安装脚本：<https://github.com/hwdsl2/setup-ipsec-vpn/blob/master/README-zh.md>
- IPsec 服务端口: <https://serverfault.com/questions/451381/which-ports-for-ipsec-lt2p>
- 配置 IPsec/L2TP VPN 客户端：<https://github.com/hwdsl2/setup-ipsec-vpn/blob/master/docs/clients-zh.md>
- 管理 VPN 用户：<https://github.com/hwdsl2/setup-ipsec-vpn/blob/master/docs/manage-users-zh.md>
