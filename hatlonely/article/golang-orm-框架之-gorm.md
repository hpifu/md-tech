---
title: golang orm 框架之 gorm
date: 2018-02-08 14:28:09
tags: [golang, orm, gorm, 数据库, mysql]
thumbnail: /img/thumbnail/onepiece2.jpg
---

最近在想给这个小站增加点赞和评论功能，第三方系统又有各种限制，就想自己弄个后端，实现类似的功能，对于个人来说，数据量不是很大，单机的 mysql 足够存下所有数据，mysql 作为底层存储是个不错的选择

之前在公司是直接用的 `github.com/go-sql-driver/mysql` 访问数据库都是直接用写 sql，取出结果然后自己拼成对象，使用上面不是很方便，可读性也不好。想起之前研究 php laravel 框架的时候，直接把数据库层屏蔽了，用户看到的只有对象，使用非常方便，java 里面这种操作方式基本上已经成了标准做法，就去 github 上找了一下 golang 里面有没有类似的东西，果然已经有非常成熟的框架了，`github.com/jinzhu/gorm` 已经有 7k+ 的 star 了

ORM（Object Relation Mapping），对象关系映射，实际上就是对数据库的操作进行封装，对上层开发人员屏蔽数据操作的细节，开发人员看到的就是一个个对象，大大简化了开发工作，提高了生产效率

好了，下面我以这个点赞评论系统为例，介绍一下 gorm 的简单用法，以下使用的完整代码：<https://github.com/hatlonely/microservices/blob/master/internal/comment_like/comment_like.go>

### gorm 用法介绍

#### 库安装

```
go get -u github.com/jinzhu/gorm
```

#### 数据库连接

```golang
import (
    "github.com/jinzhu/gorm"
    _ "github.com/jinzhu/gorm/dialects/mysql"
）

var db *gorm.DB

func init() {
    var err error
    db, err = gorm.Open("mysql", "<user>:<password>/<database>?charset=utf8&parseTime=True&loc=Local")
    if err != nil {
        panic(err)
    }
}
```

连接比较简单，直接调用 `gorm.Open` 传入数据库地址即可

`github.com/jinzhu/gorm/dialects/mysql` 是 golang 的 mysql 驱动，实际上就是 `github.com/go-sql-driver/mysql` 作者这里为了好记，重新弄了个名字

这里我用的 mysql，实际上支持基本上所有主流的关系数据库，连接方式上略有不同

``` golang
db.DB().SetMaxIdleConns(10)
db.DB().SetMaxOpenConns(100)
```

还可以使用 `db.DB()` 对象设置连接池信息

#### 表定义

先来定义一个点赞表，这里面一条记录表示某个用户在某个时刻对某篇文章点了一个赞，用 ip + ua 来标识用户，title 标识文章标题

```golang
type Like struct {
    ID        int    `gorm:"primary_key"`
    Ip        string `gorm:"type:varchar(20);not null;index:ip_idx"`
    Ua        string `gorm:"type:varchar(256);not null;"`
    Title     string `gorm:"type:varchar(128);not null;index:title_idx"`
    Hash      uint64 `gorm:"unique_index:hash_idx;"`
    CreatedAt time.Time
}
```

gorm 用 tag 的方式来标识 mysql 里面的约束

创建索引只需要直接指定列即可，这里创建了两个索引，`ip_idx` 和 `title_idx`；如果需要多列组合索引，直接让索引的名字相同即可；如果需要创建唯一索引，指定为 `unique_index` 即可

支持时间类型，直接使用 `time.Time` 即可

#### 创建表

```golang
if !db.HasTable(&Like{}) {
    if err := db.Set("gorm:table_options", "ENGINE=InnoDB DEFAULT CHARSET=utf8").CreateTable(&Like{}).Error; err != nil {
        panic(err)
    }
}
```

直接通过 `db.CreateTable` 就可以创建表了，非常方便，还可以通过 `db.Set` 设置一些额外的表属性

#### 插入

```golang
like := &Like{
    Ip:        ip,
    Ua:        ua,
    Title:     title,
    Hash:      murmur3.Sum64([]byte(strings.Join([]string{ip, ua, title}, "-"))) >> 1,
    CreatedAt: time.Now(),
}

if err := db.Create(like).Error; err != nil {
    return err
}
```

先构造已给对象，直接调用 `db.Create()` 就可以插入一条记录了

#### 删除

```golang
if err := db.Where(&Like{Hash: hash}).Delete(Like{}).Error; err != nil {
    return err
}
```

先用 `db.Where()` 构造查询条件，再调用 `db.Delete()` 就可以删除

#### 查询

```golang
var count int
err := db.Model(&Like{}).Where(&Like{Ip: ip, Ua: ua, Title: title}).Count(&count).Error
if err != nil {
    return false, err
}
```

先用 `db.Model()` 选择一个表，再用 `db.Where()` 构造查询条件，后面可以使用 `db.Count()` 计算数量，如果要获取对象，可以使用 `db.Find(&Likes)` 或者只需要查一条记录 `db.First(&Like)`

#### 修改

```golang
db.Model(&user).Update("name", "hello")
db.Model(&user).Updates(User{Name: "hello", Age: 18})
db.Model(&user).Updates(User{Name: "", Age: 0, Actived: false}) // nothing update
```

我这个系统里面没有更新需求，这几个例子来自于官网，第一个是更新单条记录；第二个是更新整条记录，注意只有非空字段才会更新；第三个例子是不会更新的，在系统设计的时候要尽量避免这些空值有特殊的含义，如果一定要更新，可以使用第一种方式，设置单个值

#### 错误处理

其实你已经看到了，这里基本上所有的函数都是链式的，全部都返回 `db` 对象，任何时候调用 `db.Error` 就能获取到错误信息，非常方便

#### 事务

```golang
func CreateAnimals(db *gorm.DB) err {
    tx := db.Begin()
    if err := tx.Create(&Animal{Name: "Giraffe"}).Error; err != nil {
        tx.Rollback()
        return err
    }
    if err := tx.Create(&Animal{Name: "Lion"}).Error; err != nil {
        tx.Rollback()
        return err
    }
    tx.Commit()
    return nil
}
```

事务的处理也很简单，用 `db.Begin()` 声明开启事务，结束的时候调用 `tx.Commit()`，异常的时候调用 `tx.Rollback()`

#### 其他

还可以使用如下方式设置日志输出级别以及改变日志输出地方

```
db.LogMode(true)
db.SetLogger(gorm.Logger{revel.TRACE})
db.SetLogger(log.New(os.Stdout, "\r\n", 0))
```

也支持普通的 sql，但是建议尽量不要使用

### 参考链接

- gorm 官方文档: <http://gorm.io/>
- gorm github: <https://github.com/jinzhu/gorm>
