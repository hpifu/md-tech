---
title: golang 反射
date: 2018-01-31 21:36:48
tags: [golang, 反射]
---

反射机制是指在程序运行的时候动态地获取对象的属性后者调用对象方法的功能。golang 支持反射，原生的 json 库就是用反射机制实现。

golang 的反射有两个主要的概念: `reflect.Type` 和 `reflect.Value` 从字面上理解就是类型和值

## `reflect.Type`

`reflect.Type` 指的就是一个类型，可以是基本类型 int，string，也可以是自定义的结构体类型，可以通过 `reflect.TypeOf()` 获取对象的类型

``` go
TypeOf(i interface{}) Type  // 获取任意一个对象的 Type 对象

(t *Type) NumField() int    // 字段数量
(t *Type) NumMethod() int   // 方法数量
(t *Type) Field(i int) StructField  // 返回第 i 个字段
(t *Type) FieldByName(name string) (StructField, bool)  // 根据字段名获取字段
(t *Type) Name()            // 字段名
(t *Type) Kind() Kind       // 字段类型

type StructField struct {
	Name    string      // 字段名
	PkgPath string      // 包路径
	Type    Type        // 字段类型
    Tag     StructTag   // 字段的 tag
}

(tag StructTag) Get(key string) string  // 获取字段的 tag 中的某个 key
```

## `reflect.Value`

`reflect.Value` 指的是一个值，主要的 api

``` go
ValueOf(i interface{}) Type  // 获取任意一个对象的 Value 对象

(v Value) NumField() int    // 字段数量
(v Value) NumMethod() int   // 方法数量
(v Value) Field(i int) Value        // 返回第 i 个字段
(v Value) Method(int) Value         // 返回第 i 个方法
(v Value) FieldByName(name string) Value    // 根据字段名获取字段
(v Value) MethodByName(string) Value        // 根据方法名获取字段
(v Value) Type() Type       // 获取的类型

(v Value) Int() int         // 返回 int
(v Value) String() string   // 返回 string
(v Value) Interface() interface{}   // 返回一个 interface，之后可以转成任意对象

(v Value) Call(in []Value) []Value      // 方法调用，参数和返回值都是数组的形式
(v Value) CallSlice(in []Value) []Value // 可变长参数函数调用
```

## 举例

``` go
type A struct {
	F1 int    `json:"f1"`
	F2 string `json:"f2"`
}
```

获取类型

``` go
t := reflect.TypeOf(A{})
So(t.NumField(), ShouldEqual, 2)
So(t.Field(0).Name, ShouldEqual, "F1")
So(t.Field(0).Type.Kind(), ShouldEqual, reflect.Int)
So(t.Field(0).Tag, ShouldEqual, `json:"f1"`)
So(t.Field(1).Name, ShouldEqual, "F2")
So(t.Field(1).Type.Kind(), ShouldEqual, reflect.String)
So(t.Field(1).Tag, ShouldEqual, `json:"f2"`)
```

获取 tag

``` go
So(reflect.TypeOf(A{}).Field(0).Tag.Get("json"), ShouldEqual, "f1")
So(reflect.TypeOf(A{}).Field(1).Tag.Get("json"), ShouldEqual, "f2")
```

获取值

``` go
a := A{
    F1: 10,
    F2: "hatlonely",
}
v := reflect.ValueOf(a)
So(v.NumField(), ShouldEqual, 2)
So(v.Field(0).Int(), ShouldEqual, 10)
So(v.Field(1).String(), ShouldEqual, "hatlonely")
So(v.Field(0).Type().Kind(), ShouldEqual, reflect.Int)
So(v.Field(1).Type().Kind(), ShouldEqual, reflect.String)
So(v.FieldByName("F1").Int(), ShouldEqual, 10)
So(v.FieldByName("F2").Interface().(string), ShouldEqual, "hatlonely")
```

调用方法，这里要注意，方法的 `receiver` 是指针还是对象，value 的类型应该和 receiver 保持一致

``` go
func (a *A) Add(b int) int {
	a.F1 += b
	return a.F1
}

func (a A) Mul(b int) int {
	return a.F1 * b
}

func (a A) Sum(vi ...int) int {
	sum := 0
	for _, i := range vi {
		sum += i
	}

	return sum + a.F1
}

a := A{
    F1: 10,
}

So(reflect.ValueOf(a).MethodByName("Mul").Call([]reflect.Value{reflect.ValueOf(20)})[0].Int(), ShouldEqual, 200)
So(a.F1, ShouldEqual, 10)
So(reflect.ValueOf(&a).MethodByName("Add").Call([]reflect.Value{reflect.ValueOf(20)})[0].Int(), ShouldEqual, 30)
So(a.F1, ShouldEqual, 30)
So(reflect.ValueOf(a).MethodByName("Sum").Call([]reflect.Value{
    reflect.ValueOf(30), reflect.ValueOf(40), reflect.ValueOf(50),
})[0].Int(), ShouldEqual, 150)
So(reflect.ValueOf(a).MethodByName("Sum").CallSlice([]reflect.Value{
    reflect.ValueOf([]int{30, 40, 50}),
})[0].Int(), ShouldEqual, 150)
```

## 链接

- 完整代码: <https://github.com/hatlonely/hellogolang/blob/master/internal/buildin/reflect_test.go>
