---
title: golang grpc keepalive
date: 2019-10-21 08:25:35
tags: [golang, grpc]
---

最近遇到 grpc 客户端报错 "rpc error: code = Unavailable desc = transport is closing"，原因是连接长时间没有使用，被服务端断开，这种情况通过简单粗暴的重试策略可以解决，更加优雅的解决方案是增加保持连接策略

## 服务端

``` go
var kaep = keepalive.EnforcementPolicy{
	MinTime:             5 * time.Second, // If a client pings more than once every 5 seconds, terminate the connection
	PermitWithoutStream: true,            // Allow pings even when there are no active streams
}

var kasp = keepalive.ServerParameters{
	MaxConnectionIdle:     15 * time.Second, // If a client is idle for 15 seconds, send a GOAWAY
	MaxConnectionAge:      30 * time.Second, // If any connection is alive for more than 30 seconds, send a GOAWAY
	MaxConnectionAgeGrace: 5 * time.Second,  // Allow 5 seconds for pending RPCs to complete before forcibly closing connections
	Time:                  5 * time.Second,  // Ping the client if it is idle for 5 seconds to ensure the connection is still active
	Timeout:               1 * time.Second,  // Wait 1 second for the ping ack before assuming the connection is dead
}

server := grpc.NewServer(grpc.KeepaliveEnforcementPolicy(kaep), grpc.KeepaliveParams(kasp))
```

## 客户端

``` go
var kacp = keepalive.ClientParameters{
	Time:                10 * time.Second, // send pings every 10 seconds if there is no activity
	Timeout:             time.Second,      // wait 1 second for ping ack before considering the connection dead
	PermitWithoutStream: true,             // send pings even without active streams
}

conn, err := grpc.Dial(*addr, grpc.WithInsecure(), grpc.WithKeepaliveParams(kacp))
```

## 链接

- 服务端代码: <https://github.com/grpc/grpc-go/blob/master/examples/features/keepalive/server/main.go>
- 客户端代码: <https://github.com/grpc/grpc-go/blob/master/examples/features/keepalive/client/main.go>
