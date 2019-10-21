---
title: golang 依赖管理之 glide
date: 2018-01-27 13:47:01
tags: [golang, 依赖管理, glide]
---

依赖管理是一个语言非常重要的特性，很大程度上决定着一个语言的流行程度，流行的语言大多都有非常成熟的依赖管理工具，java 的 maven 和 gradle，javascript 的 npm，python 的 pip，这些工具极大地降低了我们使用第三方库的成本，提高了生产效率，而 c++ 比较奇葩，并没有这样统一的依赖管理工具，大公司好一点，有专门的团队去做这样的工具解决依赖的问题，小公司就只能自己把源码拉下来，放到固定的目录，然后编译成二进制，运气不好的话，还要自己解决各种兼容性的问题，如果有版本更新，这个过程还得重复一遍，第三方库的使用和维护成本之高，让人简直就想放弃……

Golang 是自带依赖管理工具的，直接 `go get <packages>` 就可以把依赖拉下来，但是这种方式有个缺陷，没有版本控制，你每次拉下来的 `package` 都是 `master` 分支上的版本，这样是很危险的，源代码更新可能会有一些对外接口上面的调整，这些调整很有可能就导致你的程序编译通不过，而更致命的是，新的代码引入了一些新的 bug 或者接口语义上的变化会导致你的程序崩溃，所以早期的 gopher 开发了另一个依赖管理工具 [`godep`](https://github.com/tools/godep)解决了版本管理的问题，最近，golang 官方也在开发一个新的依赖管理工具 [`dep`](https://github.com/golang/dep)，但今天我给大家推荐的是 [`glide`](https://github.com/Masterminds/glide) 这款工具，和其他工具相比呢，这款工具支持更多的特性，包括支持依赖的自动分析，指定版本范围，依赖清理等等，而且使用起来也比较简单。这里有一些工具的对比：<https://github.com/Masterminds/glide/wiki/Go-Package-Manager-Comparison>

下面我给大家简单介绍一下 `glide` 在实际项目中的使用


## 安装

Linux

```
curl https://glide.sh/get | sh
```

Mac

```
brew install glide
```

## 初始化

```
glide init
```

这个命令会自动分析你代码里面的依赖，然后创建一个 `glide.yaml` 来描述你当前项目的依赖，生成的这个文件是可以手动编辑的，可以手动修改一些版本之类的信息

如果有些文件夹不希望被扫描，可以通过修改 `glide.yaml`，增加 `excludeDirs`，希望忽略某些引入的包可以使用 `ignore`，更多信息可以参考：<https://glide.readthedocs.io/en/latest/glide.yaml/>

```
ignore:
- api
excludeDirs:
- api
- cmd
```

提醒一下，这个操作必须在 `$GOPATH/src/` 的子目录下面，这个和 golang 本身的包管理机制有关，如果没有设置 `$GOPATH`，记得设置一下 `export GOPATH=<directory>`

## 依赖下载

```
glide update
```

这个命令会下载 `glide.yaml` 里面的依赖库，并且同样会分析并下载依赖库依赖的其他第三方库，下载的依赖会放到与 `glide.yaml` 同级的 `vendor` 目录，同时还会生成一个 `glide.lock` 文件，这个文件里面描述了当前依赖的版本信息，不要手工编辑这个文件

如果你在中国，这个步骤里面可能会碰到有些 `gopkg` 的库拉不下来，也不知道为啥要把这个也禁了……如果你碰到这个问题，你可以手动把这些库下载到 `${GOROOT}/src/golang.org/x` 下面

```
git clone https://github.com/golang/crypto.git
git clone https://github.com/golang/sys.git
git clone https://github.com/golang/sync.git
git clone https://github.com/golang/text.git
git clone https://github.com/golang/net.git
```

## 添加依赖

```
glide get --all-dependencies github.com/foo/bar
```

也可以指定版本

```
glide get --all-dependencies github.com/foo/bar#^1.2.3
```

除了 `github` 上的依赖，也可以是其他的平台，比如 `gitee`，或者自己公司搭建的 gitlab，只要有权限就可以，还有一点需要注意，版本必须是三位数字的版本号，否则可能识别不了

## 安装依赖

```
glide install
```

这个命令是在一个已经使用 glide 管理依赖的项目里，需要在新环境下重新安装依赖使用的，这个命令会按照 `glide.lock` 的信息，把所有的依赖拉取到本地，和 `glide update` 不同的是，`glide update` 会来去最新的版本，并且会修改 `glide.lock`，而 `glide install` 只下载之前的依赖

## 依赖镜像

```
glide mirror set [original] [replacement]
```

有些库如果访问不了可以通过这种方式设置镜像，golang.org 被禁的问题也可以用如下方法解决

```
glide mirror set https://golang.org/x/crypto https://github.com/golang/crypto --vcs git
glide mirror set https://golang.org/x/net https://github.com/golang/net --vcs git
glide mirror set https://golang.org/x/text https://github.com/golang/text --vcs git
glide mirror set https://golang.org/x/sys https://github.com/golang/sys --vcs git
```

## 参考链接

- glide github: <https://github.com/Masterminds/glide>
- glide 官网: <https://glide.sh/>
- glide 文档: <https://glide.readthedocs.io/en/latest/>
- go 依赖包管理工具对比: <https://ieevee.com/tech/2017/07/10/go-import.html>
