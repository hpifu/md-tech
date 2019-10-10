---
title: golang 网络框架之 thrift
date: 2018-02-04 14:43:26
tags: [golang, 网络框架, thrift, 微服务]
thumbnail: /img/thumbnail/the_social_network.jpg
---

thrift 最初是 facebook 开发使用的 rpc 通信框架，后来贡献给了 apache 基金会，出来得比较早，几乎支持所有的后端语言，使用非常广泛，是不可不知的一个网络框架

和 [grpc](http://hatlonely.github.io/2018/02/03/golang-网络框架之-grpc/) 一样，需要先定义通信协议，然后实现自己业务逻辑，下面还是通过一个简单示例（之前的echo程序）说明 thrift 的用法，下面示例使用的完整代码在下列地址：
实现文件：<https://github.com/hatlonely/hellogolang/tree/master/cmd/thrift>
协议文件：<https://github.com/hatlonely/hellogolang/tree/master/api/echo_thrift>

### 简单 echo 服务

#### 获取 thrift

```
go get git.apache.org/thrift.git/lib/go
```

#### 定义协议文件

```
namespace go echo

struct EchoReq {
    1: string msg;
}

struct EchoRes {
    1: string msg;
}

service Echo {
    EchoRes echo(1: EchoReq req);
}
```

执行 `thrift -r --gen go echo.thrift` 命令会生成 `gen-go` 文件夹，这个过程其实是将上面的协议翻译成 golang 代码

这个命令依赖于 thrift 工具，可以通过下面命令获取

Mac

```
brew install thrift
```

Linux

```
wget http://www-us.apache.org/dist/thrift/0.11.0/thrift-0.11.0.tar.gz
tar -xzvf thrift-0.11.0.tar.gz
cd thrift-0.11.0
./configure
make -j8
[sudo] make install 
```

#### 实现服务端

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

func main() {
    transport, err := thrift.NewTServerSocket(":3000")
    if err != nil {
        panic(err)
    }

    processor := echo.NewEchoProcessor(&EchoServerImp{})
    server := thrift.NewTSimpleServer4(
        processor,
        transport,
        thrift.NewTBufferedTransportFactory(8192),
        thrift.NewTCompactProtocolFactory(),
    )
    if err := server.Serve(); err != nil {
        panic(err)
    }
}
```

这个过程和 grpc 类似，不同的地方在于，thrift 支持更多的服务器类型，支持不同的协议打包方式，方便用户选择，这里的 compact 协议是一种压缩的协议，使用比较多

#### 实现客户端

```
func main() {
    var transport thrift.TTransport
    var err error
    transport, err = thrift.NewTSocket("localhost:3000")
    if err != nil {
        fmt.Errorf("NewTSocket failed. err: [%v]\n", err)
        return
    }

    transport, err = thrift.NewTBufferedTransportFactory(8192).GetTransport(transport)
    if err != nil {
        fmt.Errorf("NewTransport failed. err: [%v]\n", err)
        return
    }
    defer transport.Close()

    if err := transport.Open(); err != nil {
        fmt.Errorf("Transport.Open failed. err: [%v]\n", err)
        return
    }

    protocolFactory := thrift.NewTCompactProtocolFactory()
    iprot := protocolFactory.GetProtocol(transport)
    oprot := protocolFactory.GetProtocol(transport)
    client := echo.NewEchoClient(thrift.NewTStandardClient(iprot, oprot))

    var res *echo.EchoRes
    res, err = client.Echo(context.Background(), &echo.EchoReq{
        Msg: strings.Join(os.Args[1:], " "),
    })
    if err != nil {
        fmt.Errorf("client echo failed. err: [%v]", err)
        return
    }

    fmt.Printf("message from server: %v", res.GetMsg())
}
```

这个 client 相对复杂一些，需要和 server 端设置一致的打包方式，如果不一致会出现通信失败，这一点需要特别注意一下

### 参考链接

- thrift go 官网: <http://thrift.apache.org/tutorial/go>
- thrift github: <https://github.com/apache/thrift/>
- thrift go tutorial: <https://github.com/apache/thrift/tree/master/tutorial/go/src>

