---
title: golang 开发目录结构
date: 2018-01-16 21:56:43
tags: [golang, 开发, 目录结构]
---

在实际的项目中发现大家的目录结构都比较凌乱，基本每个人都有每个人的风格，一个项目在不断地变大，一些新的文件或目录又不断地被添加进来，从这里面去找到自己需要的信息的成本越来越高，一个统一的通用的目录结构非常有必要。

以下内容来自于github上的这个项目（<https://github.com/golang-standards/project-layout>）

### `/cmd`

main函数文件（比如 `/cmd/myapp.go`）目录，这个目录下面，每个文件在编译之后都会生成一个可执行的文件。

不要把很多的代码放到这个目录下面，这里面的代码尽可能简单。

### `/internal`

应用程序的封装的代码，某个应用私有的代码放到 `/internal/myapp/` 目录下，多个应用通用的公共的代码，放到 `/internal/common` 之类的目录。

### `/pkg`

一些通用的可以被其他项目所使用的代码，放到这个目录下面

### `/vendor`

项目依赖的其他第三方库，使用 [`glide`](https://github.com/Masterminds/glide) 工具来管理依赖

### `/api`

协议文件，`Swagger/thrift/protobuf` 等

### `/web`

web服务所需要的静态文件

### `/configs`

配置文件

### `/init`

服务启停脚本

### `/scripts`

其他一些脚本，编译、安装、测试、分析等等

### `/build`

持续集成目录

云 (AMI), 容器 (Docker), 操作系统 (deb, rpm, pkg)等的包配置和脚本放到 `/build/package/` 目录


### `/deployments`

部署相关的配置文件和模板

### `/test`

其他测试目录，功能测试，性能测试等

### `/docs`

设计文档

### `/tools`

常用的工具和脚本，可以引用 `/internal` 或者 `/pkg` 里面的库

### `/examples`

应用程序或者公共库使用的一些例子

### `/assets`

其他一些依赖的静态资源





