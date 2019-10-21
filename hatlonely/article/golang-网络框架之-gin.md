---
title: golang 网络框架之 gin
date: 2018-02-03 14:49:30
tags: [golang, 网络框架, gin, 微服务]
---

golang 原生 http 库已经可以很方便地实现一个 http server 了，但对于复杂的 web 服务来说，路由解析，请求参数解析，对象返回等等，原生 api 就显得有些不太够用了，而 gin 是一个功能完备，性能很高的 web 网络框架，特别适合 web api 的开发

## hello world

``` go
package main

import "github.com/gin-gonic/gin"

func main() {
	r := gin.New()
	r.GET("/ping", func(c *gin.Context) {
		c.String(200, "hello world")
	})
	r.Run() // listen and serve on 0.0.0.0:8080
}
```

如这个 `hello world` 程序所示 gin 所有的业务逻辑都在 `func(c *gin.Context)` 函数中实现，请求和返回都通过这个 gin.Context 传递

## 请求参数解析

gin 提供了丰富的请求参数获取方式

``` go
(c *Context) Query(key string) string               // 获取 GET 参数
(c *Context) QueryArray(key string) []string        // 获取 GET 参数数组
(c *Context) DefaultQuery(key, defaultValue string) // 获取 GET 参数，并提供默认值
(c *Context) Param(key string) string               // 获取 Param 参数，类似于 "/user/:id"
(c *Context) GetRawData() ([]byte, error)           // 获取 body 数据
```

但这些函数我都不建议使用，建议用结构体来描述请求，再使用 bind api 直接将获取请求参数

``` go
type HelloWorldReq struct {
    Token    string `json:"token"`
    ID       int    `json:"id" uri:"id"`
    Email    string `json:"email" form:"email"`
    Password string `json:"password" form:"password"`
}

req := &HelloWorldReq{
    Token: c.GetHeader("Authorization"),    // 头部字段无法 bind，可以通过 GetHeader 获取
}

// 用请求中的 Param 参数填充结构体中的 uri 字段
if err := c.BindUri(req); err != nil {
    return nil, nil, http.StatusBadRequest, fmt.Errorf("bind uri failed. err: [%v]", err)
}

// GET 请求中用 Query 参数填充 form 字段
// 非 GET 请求，将 body 中的 json 或者 xml 反序列化后填充 form 字段
if err := c.Bind(req); err != nil {
    return nil, nil, http.StatusBadRequest, fmt.Errorf("bind failed. err: [%v]", err)
}
```

http 的客户端 ip 一般在请求头的 `X-Forwarded-For` 和 `X-Real-Ip` 中，gin 提供了 `(c *Context) ClientIP() string` 来获取 ip

## 返回包体

``` go
(c *Context) String(code int, format string, values ...interface{}) // 返回一个字符串
(c *Context) JSON(code int, obj interface{})                        // 返回一个 json
(c *Context) Status(code int)                                       // 返回一个状态码
```

## 文件上传和返回

从请求中获取文件

``` go
fh, err := ctx.FormFile("file")
if err != nil {
    return err
}

src, err := fh.Open()
if err != nil {
    return err
}
defer src.Close()
```

返回文件

``` go
(c *Context) File(filepath string)
```

## cros 跨域

服务端返回的头部中有个字段 "Access-Control-Allow-Origin"，如果该字段和请求的域不同，浏览器会被浏览器拒绝，其实这个地方我理解应该是客户端没有权限访问，服务端不该返回结果，浏览器认为结果不可用，所以提示跨域错误，而这个头部字段还只能写一个地址，或者写成 *，对所有网站都开放，要想对多个网站开发，我们可以根据请求的 "Origin" 字段，动态地设置 "Access-Control-Allow-Origin" 字段，满足权限得设置成请求中的 "Origin" 字段，gin 的有个插件 `github.com/gin-contrib/cors` 就是专门用来做这个事情的，可以在 AllowOrigins 中设置多个网站，还可以设置通配符（需设置 `AllowWildcard` 为 `true`）

``` go
import "github.com/gin-contrib/cors"

r := gin.New()
r.Use(cors.New(cors.Config{
    AllowOrigins:     []string{"a.example.com", "b.example.com"},
    AllowMethods:     []string{"PUT", "POST", "GET", "OPTIONS"},
    AllowHeaders:     []string{"Origin", "Content-Type", "Content-Length", "Accept-Encoding", "X-CSRF-Token", "Authorization", "Accept", "Cache-Control", "X-Requested-With"},
    AllowCredentials: true,
}))
```

## cookies

``` go
// maxAge 为过期时间
// domain 是网站的地址，如需跨域共享 cookie，可以设置成域名，
//      比如 a.example.com 和 b.example.com，可以将 domain 设置成 example.com
// secure 为 https 设为 true，http 设为 false
// httpOnly 设置为 false，否则 axios 之类的库访问不到 cookie
(c *Context) SetCookie(name, value string, maxAge int, path, domain string, secure, httpOnly bool)
```

另外，axios 需要设置 `withCredentials: true` cookie 才能正常返回

## 链接

- github 地址: <https://github.com/gin-gonic/gin>
- 代码示例: <https://github.com/hpifu/tpl-go-http>
