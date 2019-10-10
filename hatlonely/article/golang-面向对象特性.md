---
title: golang 面向对象特性
date: 2018-06-19 16:54:26
tags: [golang, 面向对象, 接口, 继承, 多态, 组合, 覆盖, interface]
---

和其他高级语言一样，golang 也支持面向对象编程，支持得比较简单，有些特性并不支持，但是够用了

### 接口

接口使用 interface 关键字声明，任何实现接口定义方法的类都可以实例化该接口，接口和实现类之间没有任何依赖，你可以实现一个新的类当做 Sayer 来使用，而不需要依赖 Sayer 接口，也可以为已有的类创建一个新的接口，而不需要修改任何已有的代码，和其他静态语言相比，这可以算是 golang 的特色了吧

``` go
type Sayer interface {
    Say(message string)
    SayHi()
}
```

### 继承

继承使用组合的方式实现

``` go
type Animal struct {
	Name string
}

func (a *Animal) Say(message string) {
	fmt.Printf("Animal[%v] say: %v\n", a.Name, message)
}

type Dog struct {
    Animal
}
```

Dog 将继承 Animal 的 Say 方法，以及其成员 Name

### 覆盖

子类可以重新实现父类的方法

``` go
// override Animal.Say
func (d *Dog) Say(message string) {
	fmt.Printf("Dog[%v] say: %v\n", d.Name, message)
}
```

Dog.Say 将覆盖 Animal.Say

### 多态

接口可以用任何实现该接口的指针来实例化

``` go
var sayer Sayer

sayer = &Dog{Animal{Name: "Yoda"}}
sayer.Say("hello world")
```

但是不支持父类指针指向子类，下面这种写法是不允许的

``` go
var animal *Animal
animal = &Dog{Animal{Name: "Yoda"}}
```

同样子类继承的父类的方法引用的父类的其他方法也没有多态特性

``` go
func (a *Animal) Say(message string) {
	fmt.Printf("Animal[%v] say: %v\n", a.Name, message)
}

func (a *Animal) SayHi() {
	a.Say("Hi")
}

func (d *Dog) Say(message string) {
	fmt.Printf("Dog[%v] say: %v\n", d.Name, message)
}

func main() {
    var sayer Sayer

    sayer = &Dog{Animal{Name: "Yoda"}}
    sayer.Say("hello world")  // Dog[Yoda] say: hello world
    sayer.SayHi()             // Animal[Yoda] say: Hi
}
```

上面这段代码中，子类 Dog 没有实现 SayHi 方法，调用的是从父类 Animal.SayHi，而 Animal.SayHi 调用的是 Animal.Say 而不是Dog.Say，这一点和其他面向对象语言有所区别，需要特别注意，但是可以用下面的方式来实现类似的功能，以提高代码的复用性

``` go
func SayHi(s Sayer) {
	s.Say("Hi")
}

type Cat struct {
	Animal
}

func (c *Cat) Say(message string) {
	fmt.Printf("Cat[%v] say: %v\n", c.Name, message)
}

func (c *Cat) SayHi() {
	SayHi(c)
}

func main() {
    var sayer Sayer

    sayer = &Cat{Animal{Name: "Jerry"}}
	sayer.Say("hello world") // Cat[Jerry] say: hello world
	sayer.SayHi()            // Cat[Jerry] say: Hi
}
```

### 参考链接

- 完整代码参考: <https://github.com/hatlonely/hellogolang/blob/master/internal/buildin/object_oriented_test.go>
