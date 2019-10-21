---
title: golang 依赖管理之 mod
date: 2019-10-21 20:10:01
tags: [golang, 依赖管理, mod]
---

go 很早就考虑了依赖管理的问题，内置 `go get` 命令，可以直接获取对应的依赖，非常方便，但是有一个巨大的缺陷，没有版本的维护和管理，而版本不一致可能会导致各种兼容性问题，因此出现了很多第三方依赖管理工具，`dep` 和 `glide` 就是其中的佼佼者，到了 go 1.11 官方终于推出了自己的依赖管理工具 `mod`，并内置在 go 版本中，`go mod` 使用简单，功能强大，并且能自动兼容大多数之前的第三方工具，大量优秀的开源库都已经切换到了 `go mod`，大有一统江湖之势

## GO111MODULE

一个最大的变化是 golang 的项目终于不再依赖 `$GOPATH` 目录了，之前的项目由于 `import` 机制问题，所有项目都位于 `$GOPATH/src` 目录下，虽然也没有太大的问题，但总是让人感觉很怪，go 1.11 终于调整了这个问题，将代码从 `$GOPATH` 中迁出了，为了兼容之前研发模式，也仍然支持放到 `$GOPATH` 下，通过 `GO111MODULE` 环境变量控制

- `GO111MODULE=off`: 关闭 mod，查找 `vendor` 目录和 `$GOPATH` 路径下的依赖
- `GO111MODULE=on`: 开启 mod, 仅根据 `go.mod` 下载和查找依赖
- `GO111MODULE=auto`: 默认值，在非 `$GOPATH` 路径并且包含 `go.mod` 的项目中才开启 mod

## 主要命令

``` bash
go mod init     # 在新的 go 项目中执行，自动分析依赖，创建 go.sum
go mod tidy     # 自动分析依赖，并自动添加和删除依赖
go mod vendor   # 创建 vendor 目录，将依赖拷贝到当前的 vendor 文件夹下
go mod download # 手动下载依赖
```

1. 对于一个新的 go 项目，只需要新建项目的时候执行一下 `go mod init`
2. 之后每次库更新，只需要先在代码中 import 对应库，然后执行 `go mod tidy` 即可（也可用 `go mod download` 手动下载）

## 库版本更换

手动修改 `go.mod` 文件中的 require 字段，重新执行 `go mod tidy` 即可

```
require (
	github.com/gin-gonic/gin v1.4.0
)
```

golang 的版本使用以 `v` 开头的三位版本号，其中第一位表示有重大本本的更新，当发布一个 `v2+` 版本的库时，`module my-module` 应该改为 `module my-module/v2`，否则引入该库需要在增加 `+incompatible` 后缀

```
require (
	github.com/lestrrat-go/file-rotatelogs v2.2.0+incompatible
)
```

## 解决 GFW 问题

因为一些原因，国内的网络访问不到 golang.org 上的库，好在大部分库在 github 上都有镜像，可以用 `replace` 命令设置镜像，下面是我碰到的一些库

```
replace (
	cloud.google.com/go => github.com/googleapis/google-cloud-go v0.0.0-20190603211518-c8433c9aaceb
	go.etcd.io/bbolt => github.com/etcd-io/bbolt v1.3.4-0.20191001164932-6e135e5d7e3d
	go.uber.org/atomic => github.com/uber-go/atomic v1.4.1-0.20190731194737-ef0d20d85b01
	go.uber.org/multierr => github.com/uber-go/multierr v1.2.0
	go.uber.org/zap => github.com/uber-go/zap v1.10.1-0.20190926184545-d8445f34b4ae
	golang.org/x/crypto => github.com/golang/crypto v0.0.0-20190605123033-f99c8df09eb5
	golang.org/x/exp => github.com/golang/exp v0.0.0-20190510132918-efd6b22b2522
	golang.org/x/image => github.com/golang/image v0.0.0-20190523035834-f03afa92d3ff
	golang.org/x/lint => github.com/golang/lint v0.0.0-20190409202823-959b441ac422
	golang.org/x/mobile => github.com/golang/mobile v0.0.0-20190607214518-6fa95d984e88
	golang.org/x/net => github.com/golang/net v0.0.0-20190606173856-1492cefac77f
	golang.org/x/oauth2 => github.com/golang/oauth2 v0.0.0-20190604053449-0f29369cfe45
	golang.org/x/sync => github.com/golang/sync v0.0.0-20190423024810-112230192c58
	golang.org/x/sys => github.com/golang/sys v0.0.0-20190602015325-4c4f7f33c9ed
	golang.org/x/text => github.com/golang/text v0.3.2
	golang.org/x/time => github.com/golang/time v0.0.0-20190308202827-9d24e82272b4
	golang.org/x/tools => github.com/golang/tools v0.0.0-20190608022120-eacb66d2a7c3
	google.golang.org/api => github.com/googleapis/google-api-go-client v0.6.0
	google.golang.org/appengine => github.com/golang/appengine v1.6.1
	google.golang.org/genproto => github.com/google/go-genproto v0.0.0-20190605220351-eb0b1bdb6ae6
	google.golang.org/grpc => github.com/grpc/grpc-go v1.21.1
)
```

GO 1.12 之后支持了一个新的环境变量 GOPROXY，用来设置依赖的代理地址，有两个共用的地址：社区的 `goproxy.io` 和又拍云的 `goproxy.cn`，亲测好用

```
export GO111MODULE=on
export GOPROXY=https://goproxy.io
```

## 缓存

go mod 更新依赖后会在本地缓存，缓存路径 `$GOPATH/pkg/mod`

## IDE 支持

### goland

启用 mod 配置

【Goland】→【Preference】→【Go Module (vgo)】→ 【Enable Go Modules (vgo)】→ 【OK】

启用 mod 之后，goland 会自动检查依赖，并自动更新 go.sum，从而引入依赖库，一般情况下都是好使的，偶尔不好使，手动执行下 `go mod tidy` 即可

### vscode

vscode 好像不会自动更新，手动执行 `go mod tidy` 之后重启才能生效

## 链接

- 官网: <https://blog.golang.org/using-go-modules>
- Go Modules: <https://blog.csdn.net/ytd7777/article/details/86898187>
- goproxy.io: <https://goproxy.io/>
