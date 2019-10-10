---
title: golang 构建工具之 Makefile
date: 2018-04-11 23:51:44
tags: [golang, Makefile, 构建工具]
thumbnail: /img/thumbnail/mountain.jpg
---

可能是因为编译太简单了，golang 并没有一个官方的构建工具（类似于 java 的 maven 和 gradle之类的），但是除了编译，我们可能还需要下载依赖，运行测试，甚至像 easyjson，protobuf，thrift 这样的工具下载和代码生成，如果没有构建工具，这些工作就会非常麻烦

为了解决这个问题，之前写过一个 `everything.sh` 的脚本，把所有的操作都封装在这个脚本里面，只需要执行类似于 `sh everything.sh dependency` 的命令就可以完成对应的工作，大大简化了构建过程，但是也有一个问题，shell 脚本本身的可读性并不是很好，而且对于各个操作之间的依赖不好描述

一次偶然的机会，在 github 上看到有人用 Makefile，就尝试了一下，发现真的非常合适，Makefile 本身就是用来描述依赖的，可读性非常好，而且与强大的 shell 结合在一起，基本可以实现任何想要的功能

下面是我在实际项目中使用的一个 Makefile，支持的功能包括

- `make build`: 编译
- `make vendor`: 下载依赖
- `make api`: 生成协议代码
- `make json`: easyjson 代码生成
- `make test`: 运行单元测试
- `make benchmark`: 运行性能测试
- `make stat`: 代码复杂度统计，代码行数统计
- `make clean`: 清理 build 目录
- `make deep_clean`: 清理所有代码以外的其他文件
- `make third`: 下载所有依赖的第三方工具
- `make protoc`: 下载 protobuf 工具
- `make glide`: 下载 glide 依赖管理工具
- `make golang`: 下载 golang 环境
- `make cloc`: 下载 cloc 统计工具
- `make gocyclo`: 下载 gocyclo 圈复杂度计算工具
- `make easyjson`: 下载 easyjson 工具

``` Makefile
export PATH:=${PATH}:${GOPATH}/bin:$(shell pwd)/third/go/bin:$(shell pwd)/third/protobuf/bin:$(shell pwd)/third/cloc-1.76

.PHONY: all
all: third vendor api json build test stat

build: cmd/rta_server/*.go internal/*/*.go scripts/version.sh Makefile vendor api json
	@echo "编译"
	@rm -rf build/ && mkdir -p build/bin/ && \
	go build -ldflags "-X 'main.AppVersion=`sh scripts/version.sh`'" cmd/rta_server/main.go && \
	mv main build/bin/rta_server && \
	cp -r configs build/configs/

vendor: glide.lock glide.yaml
	@echo "下载 golang 依赖"
	glide install

api: vendor
	@echo "生成协议文件"
	@rm -rf api && mkdir api && \
	cd vendor/gitlab.mobvista.com/vta/rta_proto.git/ && \
	protoc --go_out=plugins=grpc:. *.proto && \
	cd - && \
	cp vendor/gitlab.mobvista.com/vta/rta_proto.git/* api/

json: internal/rcommon/rta_common_easyjson.go

internal/rcommon/rta_common_easyjson.go: internal/rcommon/rta_common.go Makefile
	easyjson internal/rcommon/rta_common.go

.PHONY: test
test: vendor api json
	@echo "运行单元测试"
	go test -cover internal/rranker/*.go
	go test -cover internal/rserver/*.go
	go test -cover internal/rworker/*.go
	go test -cover internal/rloader/*.go
	go test -cover internal/rrecall/*.go
	go test -cover internal/rmaster/*.go
	go test -cover internal/rsender/*.go

benchmark: benchmarkloader benchmarkall

.PHONY: benchmarkloader
benchmarkloader: vendor api json
	@echo "运行 loader 性能测试"
	go test -timeout 2h -bench BenchmarkS3Loader_Load -benchmem -cpuprofile cpu.out -memprofile mem.out -run=^$$ internal/rloader/*
	go tool pprof -svg ./rloader.test cpu.out > cpu.benchmarkloader.svg
	go tool pprof -svg ./rloader.test mem.out > mem.benchmarkloader.svg

.PHONY: benchmarkserver
benchmarkserver: vendor api json
	@echo "运行 server 性能测试"
	go test -timeout 2h -bench BenchmarkServer -benchmem -cpuprofile cpu.out -memprofile mem.out -run=^$$ internal/rserver/*
	go tool pprof -svg ./rserver.test cpu.out > cpu.benchmarkserver.svg
	go tool pprof -svg ./rserver.test mem.out > mem.benchmarkserver.svg

.PHONY: benchmarkall
benchmarkall: vendor api json
	@echo "运行 server 性能测试"
	go test -timeout 2h -bench BenchmarkAll -benchmem -cpuprofile cpu.out -memprofile mem.out -run=^$$ internal/rserver/*
	go tool pprof -svg ./rserver.test cpu.out > cpu.benchmarkall.svg	
	go tool pprof -svg ./rserver.test mem.out > mem.benchmarkall.svg

.PHONY: benchmarkcache
benchmarkcache: vendor api json
	@echo "测试 redis 集群性能"
	go test -timeout 5m -bench BenchmarkRtaCacheBatch -benchmem -cpuprofile cpu.out -memprofile mem.out -run=^$$ internal/rserver/*

.PHONY: stat
stat: cloc gocyclo
	@echo "代码行数统计"
	@ls internal/*/* scripts/* configs/* Makefile | xargs cloc --by-file
	@echo "圈复杂度统计"
	@ls internal/*/* | grep -v _test | xargs gocyclo
	@ls internal/*/* | grep -v _test | xargs gocyclo | awk '{sum+=$$1}END{printf("总圈复杂度: %s", sum)}'

.PHONY: clean
clean:
	rm -rf build

.PHONY: deep_clean
deep_clean:
	rm -rf vendor api build third

third: protoc glide golang cloc gocyclo easyjson

.PHONY: protoc
protoc: golang
	@hash protoc 2>/dev/null || { \
		echo "安装 protobuf 代码生成工具 protoc" && \
		mkdir -p third && cd third && \
		wget https://github.com/google/protobuf/releases/download/v3.2.0/protobuf-cpp-3.2.0.tar.gz && \
		tar -xzvf protobuf-cpp-3.2.0.tar.gz && \
		cd protobuf-3.2.0 && \
		./configure --prefix=`pwd`/../protobuf && \
		make -j8 && \
		make install && \
		cd ../.. && \
		protoc --version; \
	}
	@hash protoc-gen-go 2>/dev/null || { \
		echo "安装 protobuf golang 插件 protoc-gen-go" && \
		go get -u github.com/golang/protobuf/{proto,protoc-gen-go}; \
	}

.PHONY: glide
glide: golang
	@mkdir -p $$GOPATH/bin
	@hash glide 2>/dev/null || { \
		echo "安装依赖管理工具 glide" && \
		curl https://glide.sh/get | sh; \
	}

.PHONY: golang
golang:
	@hash go 2>/dev/null || { \
		echo "安装 golang 环境 go1.10" && \
		mkdir -p third && cd third && \
		wget https://dl.google.com/go/go1.10.linux-amd64.tar.gz && \
    	tar -xzvf go1.10.linux-amd64.tar.gz && \
		cd .. && \
		go version; \
	}

.PHONY: cloc
cloc:
	@hash cloc 2>/dev/null || { \
		echo "安装代码统计工具 cloc" && \
		mkdir -p third && cd third && \
		wget https://github.com/AlDanial/cloc/archive/v1.76.zip && \
		unzip v1.76.zip; \
	}

.PHONY: gocyclo
gocyclo: golang
	@hash gocyclo 2>/dev/null || { \
		echo "安装代码圈复杂度统计工具 gocyclo" && \
		go get -u github.com/fzipp/gocyclo; \
	}

.PHONY: easyjson
easyjson: golang
	@hash easyjson 2>/dev/null || { \
		echo "安装 json 编译工具 easyjson" && \
		go get -u github.com/mailru/easyjson/...; \
	}
```