---
title: golang restful 框架之 go-swagger
date: 2018-02-08 20:54:48
tags: [golang, swagger, restful, go-swagger, 微服务, 网络框架]
---

restful 是这些年的高频词汇了，各大互联网公司也都纷纷推出了自己的 restful api，其实 restful 和 thrift，grpc 类似，就是一种协议，但是这种协议有点特殊的就是使用 http 接口，返回的对象一般是 json 格式，这样有个好处，就是可以供前端的 js 直接调用，使用非常方便，但 http 本身并不是一个高效的协议，后端的内部通信还是使用 grpc 或者 thrift 可以获得更高的性能

其实如果只是要用 http 返回 json 本身并不是一件很难的事情，不用任何框架，golang 本身也能很方便做到，但是当你有很多 api 的时候，这些 api 的维护和管理就会变得很复杂，你自己都无法记住这些 api 应该填什么参数，返回什么，当然你可以花很多时间去维护一份接口文档，这样不仅耗时而且很难保证文档的即时性，准确性以及一致性

swagger 有一整套规范来定义一个接口文件，类似于 thrift 和 proto 文件，定义了服务的请求内容和返回内容，同样也有工具可以生成各种不同语言的框架代码，在 golang 里面我们使用 go-swagger 这个工具，这个工具还提供了额外的功能，可以可视化显示这个接口，方便阅读

下面通过一个例子来简单介绍一下这个框架的使用，还是之前的点赞评论系统：<https://github.com/hatlonely/microservices>

### go-swagger 使用方法

#### api 定义文件

首先需要写一个 api 定义文件，这里我只展示其中一个接口 `countlike`，请求中带有某篇文章，返回点赞的次数

```
paths:
  /countlike:
    get:
      tags:
        - like
      summary: 有多少赞
      description: ''
      operationId: countLike
      consumes:
        - application/json
      produces:
        - application/json
      parameters:
        - name: title
          in: query
          description: 文章标题
          required: true
          type: string
      responses:
        '200':
          description: 成功
          schema:
            $ref: '#/definitions/CountLikeModel'
        '500':
          description: 内部错误
          schema:
            $ref: '#/definitions/ErrorModel'
definitions:
  CountLikeModel:
    type: object
    properties:
      count:
        type: integer
      title:
        type: string
        example: golang json 性能分析
  ErrorModel:
    type: object
    properties:
      message:
        type: string
        example: error message
      code:
        type: integer
        example: 400
```

这个是 yaml 语法，有点像去掉了括号的 json

这里完整地定义了请求方法、请求参数、正常返回接口、异常返回结果，有了这个文件只需要执行下面命令就能生成框架代码了

```
swagger generate server -f api/comment_like/comment_like.yaml
```

还可以下面这个命令可视化查看这个接口文件

```
swagger serve api/comment_like/comment_like.yaml
```

这个命令依赖 swagger 工具，可以通过下面命令获取

Mac

```
brew tap go-swagger/go-swagger
brew install go-swagger
```

Linux

```
go get -u github.com/go-swagger/go-swagger/cmd/swagger
export PATH=$GOPATH/bin:$PATH
```

执行完了之后，你发现多了几个文件夹，其中 `cmd` 目录里面包含 main 函数，是整个程序的入口，`restapi` 文件夹下面包含协议相关代码，其中 `configure_xxx.go` 是需要特别关注的，你需要在这个文件里面实现你具体的业务逻辑

现在你就其实已经可以运行程序了，`go run cmd/comment-like-server/main.go`，在浏览器里面访问一下你的 api，会返回一个错误信息，告诉你 api 还没有实现，下面就来实现一下吧

#### 业务逻辑实现

```
api.LikeCountLikeHandler = like.CountLikeHandlerFunc(func(params like.CountLikeParams) middleware.Responder {
    count, err := comment_like.CountLike(params.Title)
    if err != nil {
        return like.NewCountLikeInternalServerError().WithPayload(&models.ErrorModel{
            Code: http.StatusInternalServerError,
            Message: err.Error(),
        })
    }
    return like.NewCountLikeOK().WithPayload(&models.CountLikeModel{
        Count: count,
        Title: params.Title,
    })
})
```

你只需要在这些 handler 里面实现自己的业务逻辑即可，这里对协议的封装非常好，除了业务逻辑以及打包返回，没有多余的逻辑

再次运行，现在返回已经正常了

#### 统一处理

如果你对请求有一些操作需要统一处理，比如输出统一的日志之类的，可以重写这个函数，也在 `configure_xxx.go` 这个文件中

```
func setupGlobalMiddleware(handler http.Handler) http.Handler {
    return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
        w.Header().Set("Access-Control-Allow-Origin", "*")
        handler.ServeHTTP(w, r)
    })
}
```

这里我统一设置了一下头部，解决跨域访问问题

### 参考链接

- go-swagger 官方文档：<https://goswagger.io>
- go-swagger github：<https://github.com/go-swagger/go-swagger>
- OpenApi 2.0：<https://github.com/OAI/OpenAPI-Specification/blob/master/versions/2.0.md>
