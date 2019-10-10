---
title: golang grpc 负载均衡
date: 2018-06-23 23:56:57
tags: [golang, grpc, 负载均衡, load balancer, consul]
thumbnail: /img/thumbnail/yellow_river.jpg
---

微服务架构里面，每个服务都会有很多节点，如果流量分配不均匀，会造成资源的浪费，甚至将一些机器压垮，这个时候就需要负载均衡，最简单的一种策略就是轮询，顺序依次选择不同的节点访问

grpc 在客户端提供了负载均衡的实现，并提供了服务地址解析和更新的接口(默认提供了 DNS 域名解析的支持)，方便不同服务的集成

### 使用示例

``` go
conn, err := grpc.Dial(
    "",
    grpc.WithInsecure(),
    // 负载均衡，使用 consul 作服务发现
    grpc.WithBalancer(grpc.RoundRobin(grpclb.NewConsulResolver(
        "127.0.0.1:8500", "grpc.health.v1.add",
    ))),
)
```

创建连接的时候可以使用 `WithBalancer` 选项来指定负载均衡策略，这里使用 RoundRobin 算法，其实就是轮询策略

### 与 consul 的集成

有了负载均衡策略，还需要一个地址解析和更新策略，可以使用 DNS 服务来实现，但如果我们使用 consul 来做服务的注册和发现，可以通过实现 `naming.Resolver` 和 `naming.Watcher` 接口来支持

- `naming.Resolver`: 实现地址解析
- `naming.Watcher`: 实现节点的变更，添加或者删除

``` go
func NewConsulResolver(address string, service string) naming.Resolver {
	return &consulResolver{
		address: address,
		service: service,
	}
}

type consulResolver struct {
	address string
	service string
}

func (r *consulResolver) Resolve(target string) (naming.Watcher, error) {
	config := api.DefaultConfig()
	config.Address = r.address
	client, err := api.NewClient(config)
	if err != nil {
		return nil, err
	}

	return &consulWatcher{
		client:  client,
		service: r.service,
		addrs:   map[string]struct{}{},
	}, nil
}

type consulWatcher struct {
	client    *api.Client
	service   string
	addrs     map[string]struct{}
	lastIndex uint64
}

func (w *consulWatcher) Next() ([]*naming.Update, error) {
	for {
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

		var updates []*naming.Update
		for addr := range w.addrs {
			if _, ok := addrs[addr]; !ok {
				updates = append(updates, &naming.Update{Op: naming.Delete, Addr: addr})
			}
		}

		for addr := range addrs {
			if _, ok := w.addrs[addr]; !ok {
				updates = append(updates, &naming.Update{Op: naming.Add, Addr: addr})
			}
		}

		if len(updates) != 0 {
			w.addrs = addrs
			return updates, nil
		}
	}
}

func (w *consulWatcher) Close() {
	// nothing to do
}
```

### 参考链接

- gRPC Name Resolution: <https://github.com/grpc/grpc/blob/master/doc/naming.md>
- Load Balancing in gRPC: <https://github.com/grpc/grpc/blob/master/doc/load-balancing.md>
- dns_resolver: <https://github.com/grpc/grpc-go/blob/30fb59a4304034ce78ff68e21bd25776b1d79488/naming/dns_resolver.go>
- 代码地址: <https://github.com/hatlonely/hellogolang/blob/master/sample/addservice/cmd/client/main.go>
