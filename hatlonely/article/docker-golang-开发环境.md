---
title: docker golang 开发环境
date: 2019-10-21 10:13:50
tags: [docker, golang, 开发环境]
---

go 版本的不一致，依赖库不一致，导致编译失败；代码生成工具不一致，导致生成代码有问题；依赖工具缺失，导致测试不通过等等，开发环境和生产环境之间的差异会导致各种各样奇怪的问题，而 docker 特别擅长解决这种一致性问题

## 总体方案

最简单粗暴的一种方式就是每次都将代码拷贝到一个新的容器内，执行编译，编译完成后直接将容器制作成镜像，但这种方式每次编译都需要重新拉取依赖，比较耗时

我们采用在这个基础上稍作优化，启动一个固定的容器用于编译，编译完成后将生成的结果拷贝出来，制作新的镜像，能达到近乎本地编译的效果

## 编译环境镜像

没有直接使用 dockerhub 上的 golang 镜像，是因为我们的项目除了 golang 可能还依赖一些别的工具，比如 makefile，python，protoc 等等，就需要自己制作镜像了，这里我选 centos 作为基础镜像，主要是考虑到后面制作服务镜像的时候也仅需要 centos 即可，不需要完整的 golang 环境

```
FROM centos:centos7
RUN yum install -y kde-l10n-Chinese && yum reinstall -y glibc-common && localedef -c -f UTF-8 -i zh_CN zh_CN.utf8 
RUN yum install -y epel-release
RUN yum install -y docker-io
RUN yum install -y go
RUN yum install -y make

# behave support
RUN yum install -y python36 python36-setuptools python36-pip
RUN pip3 install --upgrade pip
RUN pip3 install requests
RUN pip3 install flask
RUN pip3 install redis
RUN pip3 install pymongo
RUN pip3 install behave
RUN pip3 install pyhamcrest
RUN pip3 install pymysql
RUN pip3 install cryptography
RUN pip3 install grpcio grpcio-tools

# grpc support
RUN yum install -y unzip
RUN curl -OL https://github.com/protocolbuffers/protobuf/releases/download/v3.10.0/protoc-3.10.0-linux-x86_64.zip
RUN unzip protoc-3.10.0-linux-x86_64.zip
RUN export GOPROXY=https://goproxy.cn && \
    mkdir -p /src/google.golang.org/grpc && \
    cd /src/google.golang.org/grpc && \
    export GOPATH=/ && \
    export GO111MODULE=on && \
    go mod init && \
    go get -u google.golang.org/grpc && \
    go get -u github.com/golang/protobuf/protoc-gen-go

ENV LANG=zh_CN.UTF-8
ENV LANGUAGE=zh_CN:zh
ENV LC_ALL=zh_CN.UTF-8
```

编译环境镜像可以尽可能多地安装需要的工具，不用担心镜像过大，因为这个镜像我们仅仅用来编译

这里我安装了 make，python，behave 以及 protobuf 工具

## 启动编译环境容器

``` Makefile
.PHONY: buildenv
buildenv:
	if [ -z "$(shell docker network ls --filter name=testnet -q)" ]; then \
		docker network create -d bridge testnet; \
	fi
	if [ -z "$(shell docker ps -a --filter name=go-build-env -q)" ]; then \
		docker run --name go-build-env --network testnet -d hatlonely/go-env:v1.1.0 tail -f /dev/null; \
	fi
```

`hatlonely/go-env:v1.1.0` 为上一步制作的镜像，启动命令 `tail -f /dev/null` 是为了让容器一直存在而不退出，这样就可以一直复用这个容器执行编译

## 编译

``` Makefile
.PHONY: image
image: buildenv
	docker exec go-build-env rm -rf /data/src/${gituser}/${repository}
	docker exec go-build-env mkdir -p /data/src/${gituser}/${repository}
	docker cp . go-build-env:/data/src/${gituser}/${repository}
	docker exec go-build-env bash -c "cd /data/src/${gituser}/${repository} && make output"
	mkdir -p docker/
	docker cp go-build-env:/data/src/${gituser}/${repository}/output/${repository} docker/
	docker build --tag=hatlonely/${repository}:${version} .
```

使用 `docker cp` 命令将本地代码拷贝到容器里面，执行 `docker exec` 在 docker 内部执行编译，编译完成后，再用 `docker cp` 将编译结果拷贝到本机的 `docker/` 目录

最后再用 Dockerfile 创建出一个新的服务镜像，对应的 Dockerfile 内容如下

## 服务镜像

```
FROM centos:centos7

RUN ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
RUN echo "Asia/Shanghai" >> /etc/timezone

COPY docker/tpl-go-http /var/docker/tpl-go-http
RUN mkdir -p /var/docker/tpl-go-http/log

EXPOSE 7060

WORKDIR /var/docker/tpl-go-http
CMD [ "bin/echo", "-c", "configs/echo.json" ]
```

## 链接

- Tips & Tricks for Making Your Golang Container Builds 10x Faster: <https://medium.com/windmill-engineering/tips-tricks-for-making-your-golang-container-builds-10x-faster-4cc618a43827>
- docker golang 镜像: <https://hub.docker.com/_/golang>
- 示例项目: <https://github.com/hpifu/tpl-go-http>
