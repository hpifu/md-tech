---
title: golang aws-sdk-go 之 s3 服务
date: 2018-03-04 18:04:18
tags: [golang, aws-sdk-go, s3, aws]
---

s3 是 aws 提供的分布式文件服务，价格比较优惠，经常被用来作为日志的持久化存储，大数据处理结果的输入输出等

s3 服务提供命令行工具，可以很方便地上传、下载、删除文件，普通 golang 程序如果需要访问 s3 上文件，一种简单方式可以先将 s3 上文件下载到本地，然后直接访问本地文件即可，但是这种方式需要一个额外的步骤，下载到本地，有额外的运维成本，需要额外的磁盘空间，使用上面不是很灵活，此外，微服务应该尽可能地降低对本地数据的依赖，这种设计也不符合微服务的设计思想

使用 aws-sdk-go 可以直接访问 s3 服务，实现文件的上传和读取

以下使用的代码：<https://github.com/hatlonely/hellogolang/blob/master/internal/aws-sdk-go/s3_test.go>

### 创建会话

首先需要创建一个会话，后续的访问都可以通过这个会话进行，如果访问的服务需要授权，也可以在 config 里面指定授权文件

```
sess := session.Must(session.NewSession(&aws.Config{
    Region: aws.String(endpoints.ApSoutheast1RegionID),
}))
service := s3.New(sess)
```

这里必须指定 s3 桶所在的地区

### 上传文件

```
fp, err := os.Open("s3_test.go")
So(err, ShouldBeNil)
defer fp.Close()

ctx, cancel := context.WithTimeout(context.Background(), time.Duration(30)*time.Second)
defer cancel()

_, err = service.PutObjectWithContext(ctx, &s3.PutObjectInput{
    Bucket: aws.String("hatlonely"),
    Key:    aws.String("test/s3_test.go"),
    Body:   fp,
})
So(err, ShouldBeNil)
```

使用 `PutObjectWithContext` 实现文件的上传，这里只能实现文件的上传，不能实现文件的写入，所以只能先将文件写入到本地，然后再整个上传

可以通过 context 设置访问超时时间

### 下载文件

```
ctx, cancel := context.WithTimeout(context.Background(), time.Duration(30)*time.Second)
defer cancel()

out, err := service.GetObjectWithContext(ctx, &s3.GetObjectInput{
    Bucket: aws.String("hatlonely"),
    Key: aws.String("test/s3_test.go"),
})
So(err, ShouldBeNil)
defer out.Body.Close()
scanner := bufio.NewScanner(out.Body)
for scanner.Scan() {
    Println(scanner.Text())
}
```

使用 `GetObjectWithContext` 接口读取文件，文件的内容在 out.Body 中，可以使用 scanner 接口，不断地按行读取文件内容

最后要记得调用 `out.Body.Close()`，释放资源

### 遍历目录

```
var objkeys []string

ctx, cancel := context.WithTimeout(context.Background(), time.Duration(30)*time.Second)
defer cancel()

out, err := service.ListObjectsWithContext(ctx, &s3.ListObjectsInput{
    Bucket: aws.String("hatlonely"),
    Prefix: aws.String("test/"),
})
So(err, ShouldBeNil)
for _, content := range out.Contents  {
    objkeys = append(objkeys, aws.StringValue(content.Key))
}
Println(objkeys)
```

大数据一般都是并发输出，每个节点都会输出一个文件，到一个指定的目录下面，所以有时候我们需要去获取一个目录下面到底有哪些文件，可以使用 `ListObjectsWithContext` 遍历一个目录下所有的文件，这个函数是递归的

```
var objkeys []string

ctx, cancel := context.WithTimeout(context.Background(), time.Duration(30)*time.Second)
defer cancel()

err := service.ListObjectsPagesWithContext(ctx, &s3.ListObjectsInput{
    Bucket: aws.String("hatlonely"),
    Prefix: aws.String("test/"),
}, func(output *s3.ListObjectsOutput, b bool) bool {
    for _, content := range output.Contents {
        objkeys = append(objkeys, aws.StringValue(content.Key))
    }
    return true
})
So(err, ShouldBeNil)
Println(objkeys)
```

也可以使用 `ListObjectsPagesWithContext` 传入一个回调函数，用于处理每个文件

### 参考链接

- aws-sdk-go：<https://github.com/aws/aws-sdk-go>
- aws-sdk-go api：<https://docs.aws.amazon.com/sdk-for-go/api/>
