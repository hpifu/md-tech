---
title: golang consul-grpc 服务注册与发现
date: 2018-06-23 21:32:28
tags: [golang, consul, grpc, 服务注册与发现, 健康检查, 微服务]
---

在微服务架构里面，每个小服务都是由很多节点组成，节点的添加删除故障希望能对下游透明，因此有必要引入一种服务的自动注册和发现机制，而 consul 提供了完整的解决方案，并且内置了对 GRPC 以及 HTTP 服务的支持

### 总体架构

![服务注册与发现](service_register_and_find.png)

- 服务调用: client 直连 server 调用服务
- 服务注册: 服务端将服务的信息注册到 consul 里
- 服务发现: 客户端从 consul 里发现服务信息，主要是服务的地址
- 健康检查: consul 检查服务器的健康状态

### 服务注册

服务端将服务信息注册到 consul 里，这个注册可以在服务启动可以提供服务的时候完成

完整代码参考: <https://github.com/hatlonely/hellogolang/blob/master/sample/addservice/internal/grpcsr/consul_register.go>

``` go
config := api.DefaultConfig()
config.Address = r.Address
client, err := api.NewClient(config)
if err != nil {
    panic(err)
}
agent := client.Agent()

IP := localIP()
reg := &api.AgentServiceRegistration{
		ID:      fmt.Sprintf("%v-%v-%v", r.Service, IP, r.Port), // 服务节点的名称
		Name:    fmt.Sprintf("grpc.health.v1.%v", r.Service),    // 服务名称
		Tags:    r.Tag,                                          // tag，可以为空
		Port:    r.Port,                                         // 服务端口
		Address: IP,                                             // 服务 IP
		Check: &api.AgentServiceCheck{     // 健康检查
            Interval: r.Interval.String(), // 健康检查间隔
            // grpc 支持，执行健康检查的地址，service 会传到 Health.Check 函数中
			GRPC:     fmt.Sprintf("%v:%v/%v", IP, r.Port, r.Service), 
			DeregisterCriticalServiceAfter: r.DeregisterCriticalServiceAfter.String(), // 注销时间，相当于过期时间
		},
	}

if err := agent.ServiceRegister(reg); err != nil {
    panic(err)
}
```

### 服务发现

客户端从 consul 里发现服务信息，主要是服务的地址

完整代码参考: <https://github.com/hatlonely/hellogolang/blob/master/sample/addservice/internal/grpclb/consul_resolver.go>

``` go
services, metainfo, err := w.client.Health().Service(w.service, "", true, &api.QueryOptions{
    WaitIndex: w.lastIndex, // 同步点，这个调用将一直阻塞，直到有新的更新
})
if err != nil {
    logrus.Warn("error retrieving instances from Consul: %v", err)
}
w.lastIndex = metainfo.LastIndex

addrs := map[string]struct{}{}
for _, service := range services {
    addrs[net.JoinHostPort(service.Service.Address, strconv.Itoa(service.Service.Port))] = struct{}{}
}
```

### 健康检查

consul 检查服务器的健康状态，consul 用 `google.golang.org/grpc/health/grpc_health_v1.HealthServer` 接口，实现了对 grpc健康检查的支持，所以我们只需要实现先这个接口，consul 就能利用这个接口作健康检查了

完整代码参考: <https://github.com/hatlonely/hellogolang/blob/master/sample/addservice/cmd/server/main.go>

``` go
// HealthImpl 健康检查实现
type HealthImpl struct{}

// Check 实现健康检查接口，这里直接返回健康状态，这里也可以有更复杂的健康检查策略，比如根据服务器负载来返回
func (h *HealthImpl) Check(ctx context.Context, req *grpc_health_v1.HealthCheckRequest) (*grpc_health_v1.HealthCheckResponse, error) {
	return &grpc_health_v1.HealthCheckResponse{
		Status: grpc_health_v1.HealthCheckResponse_SERVING,
	}, nil
}

grpc_health_v1.RegisterHealthServer(server, &HealthImpl{})
```

### 参考链接

- 完整工程代码: <https://github.com/hatlonely/hellogolang/tree/master/sample/addservice>
- consul 健康检查 api: <https://www.consul.io/api/agent/check.html>
- consul 服务注册 api: <https://www.consul.io/api/agent/service.html>
- grpc 健康检查: <https://github.com/grpc/grpc/blob/master/doc/health-checking.md>
