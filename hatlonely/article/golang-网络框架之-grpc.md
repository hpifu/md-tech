---
title: golang 网络框架之 grpc
date: 2018-02-03 14:49:30
tags: [golang, 网络框架, grpc, 微服务]
thumbnail: /img/thumbnail/moon1.jpg
---

grpc 是 google 开源的一款网络框架，具有极好的性能，可能是目前性能最好的网络框架，支持流式 rpc，可以很方便地构建消息订阅发布系统，支持几乎所有主流的语言，使用上面也很简单，公司很多服务基于 grpc 框架构建，运行非常稳定

开始之前首先你要知道网络框架为你做了哪些事情：

> 1. 网络协议序列化与反序列化
> 2. 网络底层通信
> 3. 并发管理

以及需要你做哪些事情：

> 1. 定义通信的内容（通过协议文件）
> 2. 实现通信的方法（实现协议接口）

以下面两个例子来分别说明两种 rpc 服务的简单用法

示例使用的完整代码在下列地址：
实现文件：<https://github.com/hatlonely/hellogolang/tree/master/cmd/grpc>
协议文件：<https://github.com/hatlonely/hellogolang/tree/master/api>

### 简单 echo 服务

要实现的这个服务很简单，功能和 echo 命令类似，用一个字符串请求服务器，返回相同的字符串

#### 获取 grpc

```
go get google.golang.org/grpc
go get google.golang.org/genproto/
```

`go get` 上面两个库就可以了。可能被墙了，需要 vpn；如果没有 vpn，可以找一台能下载的服务器下载下来再传到本地；如果也没有服务器，可以点击[这里](/resource/google.golang.org.tar.gz)下载，解压后放到 `vendor/` 目录下即可，不过可能不是最新版本

#### 定义协议文件

首先要定义通信的协议，grpc 使用的是 proto3 序列化协议，这是一个高效的协议，关于这个协议的跟多内容可以参考下面链接：<https://developers.google.com/protocol-buffers/docs/proto3>

```
syntax = "proto3";

package echo;

message EchoReq {
    string msg = 1;
}

message EchoRes {
    string msg = 1;
}

service Echo {
    rpc echo (EchoReq) returns (EchoRes);
}
```

执行如下命令会自动生成 `echo.pb.go` 文件，这个过程其实是把上面这个协议翻译成 golang：

```
protoc --go_out=plugins=grpc:. echo.proto
```

实际项目中可以把这个命令放到一个 [Makefile](https://github.com/hatlonely/hellogolang/blob/master/api/echo_proto/Makefile) 文件中，执行 `make` 命令即可生成代码

上面命令依赖 `protoc` 工具，以及 golang 插件 `protoc-gen-go`，可以通过如下命令获取

Mac

```
brew install grpc
go get -u github.com/golang/protobuf/{proto,protoc-gen-go}
```

Linux

```
wget https://github.com/google/protobuf/releases/download/v3.2.0/protobuf-cpp-3.2.0.tar.gz
tar -xzvf protobuf-cpp-3.2.0.tar.gz
cd protobuf-3.2.0
./configure
make -j8
[sudo] make install
go get -u github.com/golang/protobuf/{proto,protoc-gen-go}
```

#### 实现协议接口

```
type EchoServerImp struct {

}

func (e *EchoServerImp) Echo(ctx context.Context, req *echo.EchoReq) (*echo.EchoRes, error) {
    fmt.Printf("message from client: %v\n", req.GetMsg())

    res := &echo.EchoRes{
        Msg: req.GetMsg(),
    }

    return res, nil
}
```

首先要定义一个接口的实现类 `EchoServerImp`，接口的的定义可以在上面生成的文件 `echo.pb.go` 中找到，这个类里面也可以有一些和业务逻辑相关的成员变量，这里我们的需求比较简单，没有其他的成员

然后需要在接口函数里面实现我们具体的业务逻辑，这里仅仅把请求里面的内容读出来，再写回到响应里面

你还可以为这个类增加其他的函数，比如初始化之类的，根据你具体的业务需求就好

#### 实现服务端

```
func main() {
    server := grpc.NewServer()
    echo.RegisterEchoServer(server, &EchoServerImp{})

    address, err := net.Listen("tcp", ":3000")
    if err != nil {
        panic(err)
    }
    
    if err := server.Serve(address); err != nil {
        panic(err)
    }
}
```

把我们刚刚实现的类实例注册到 grpc 里，再绑定到本地的一个端口上就可以了，现在可以启动服务了 `go run echo_server.go`

#### 实现客户端

```
func main() {
    conn, err := grpc.Dial("127.0.0.1:3000", grpc.WithInsecure())
    if err != nil {
        fmt.Errorf("dial failed. err: [%v]\n", err)
        return
    }

    client := echo.NewEchoClient(conn)
    res, err := client.Echo(context.Background(), &echo.EchoReq{
        Msg: strings.Join(os.Args[1:], " "),
    })

    if err != nil {
        fmt.Errorf("client echo failed. err: [%v]", err)
        return
    }

    fmt.Printf("message from server: %v", res.GetMsg())
}
```

创建一个 client 之后，就可以像访问本地方法一样访问我们的服务了，`go run echo_client.go hellogrpc`

### 流式 rpc 服务

实现一个 counter 服务，客户端传过来一个数字，服务端从这个数字开始，不停地向下计数返回

#### 定义协议文件

```
syntax = "proto3";

package counter;

message CountReq {
    int64 start = 1;
}

message CountRes {
    int64 num = 1;
}

service Counter {
    rpc count (CountReq) returns (stream CountRes);
}
```

定义一个流式的 rpc 只需要在返回的字段前加一个 stream 关键字就可以

#### 实现服务端

```
type CounterServerImp struct {

}

func (c *CounterServerImp) Count(req *counter.CountReq, stream counter.Counter_CountServer) error {
    fmt.Printf("request from client. start: [%v]\n", req.GetStart())

    i := req.GetStart()
    for {
        i++
        stream.Send(&counter.CountRes{
            Num: i,
        })
        time.Sleep(time.Duration(500) * time.Millisecond)
    }

    return nil
}

func main() {
    server := grpc.NewServer()
    counter.RegisterCounterServer(server, &CounterServerImp{})

    address, err := net.Listen("tcp", ":3000")
    if err != nil {
        panic(err)
    }

    if err := server.Serve(address); err != nil {
        panic(err)
    }
}
```

接口实现上需要写一个死循环，不停地调用 `Send` 函数返回结果即可

#### 实现客户端

```
func main() {
    start, _ := strconv.ParseInt(os.Args[1], 10, 64)

    conn, err := grpc.Dial("127.0.0.1:3000", grpc.WithInsecure())
    if err != nil {
        fmt.Errorf("dial failed. err: [%v]\n", err)
        return
    }
    client := counter.NewCounterClient(conn)

    stream, err := client.Count(context.Background(), &counter.CountReq{
        Start: start,
    })
    if err != nil {
        fmt.Errorf("count failed. err: [%v]\n", err)
        return
    }

    for {
        res, err := stream.Recv()
        if err != nil {
            fmt.Errorf("client count failed. err: [%v]", err)
            return
        }

        fmt.Printf("server count: %v\n", res.GetNum())
    }
}
```

客户端的 `Count` 接口返回的是一个 `stream`，不断地调用这个 `stream` 的 `Recv` 方法，可以不断地获取来自服务端的返回

### 参考链接

- grpc 官方文档中文版: <http://doc.oschina.net/grpc?t=60133>
- grpc 官方示例: <https://github.com/grpc/grpc-go/tree/master/examples/route_guide>
- proto3 语法: <https://developers.google.com/protocol-buffers/docs/proto3>
