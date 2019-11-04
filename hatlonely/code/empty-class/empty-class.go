package main

// go version go1.12.5 darwin/amd64

import (
	"fmt"
	"unsafe"
)

type A struct{}

func func1() {
	fmt.Println("sizeof(A):", unsafe.Sizeof(A{}))
	// sizeof(A): 0

	a1 := A{}
	a2 := A{}
	fmt.Printf("a1 == a2: %v\n", &a1 == &a2)
	// a1 == a2: false

	a3 := A{}
	a4 := A{}
	fmt.Printf("address(a3): %p\n", &a3) // a3 逃逸到堆区
	fmt.Printf("address(a4): %p\n", &a4) // a4 逃逸到堆区
	fmt.Printf("a3 == a4: %v\n", &a3 == &a4)
	// address(a3): 0x1190fd0
	// address(a4): 0x1190fd0
	// a3 == a4: true
}

type B struct {
	a1 A
	a2 A
}

func func2() {
	fmt.Println("sizeof(B):", unsafe.Sizeof(B{}))
	// sizeof(B): 0

	b1 := B{}
	b2 := B{}
	fmt.Printf("b1 == b2: %v\n", &b1 == &b2)
	// b1 == b2: false

	b3 := B{}
	b4 := B{}
	fmt.Printf("address(b3): %p\n", &b3) // b3 逃逸到堆区
	fmt.Printf("address(b4): %p\n", &b4) // b4 逃逸到堆区
	fmt.Printf("b3 == b4: %v\n", &b3 == &b4)
	// address(b3): 0x1190fd0
	// address(b4): 0x1190fd0
	// b3 == b4: true
}

func func3() {
	fmt.Println("sizeof([100]A):", unsafe.Sizeof([100]A{}))
	// sizeof([100]A): 0

	var as1 [100]A
	var as2 [100]A

	fmt.Printf("as1 == as2: %v\n", &as1 == &as2)
	// as1 == as2: false

	fmt.Printf("as1[0] == as2[1]: %v\n", &as1[0] == &as2[1])
	// as1[0] == as2[1]: true

	var as3 [100]A
	var as4 [100]A
	fmt.Printf("address(as3): %p\n", &as3) // as3 逃逸到堆区
	fmt.Printf("address(as4): %p\n", &as4) // as4 逃逸到堆区
	fmt.Printf("as3 == as4: %v\n", &as3 == &as4)
	// address(as3): 0x1190fd0
	// address(as4): 0x1190fd0
	// as3 == as4: true

	fmt.Println(len(as3), len(as4))
}

func func4() {
	fmt.Println("sizeof(make([]A, 100)):", unsafe.Sizeof(make([]A, 100)))
	// sizeof(make([]A, 100)): 24

	as1 := make([]A, 100)
	as2 := make([]A, 100)

	fmt.Printf("as1 == as2: %v\n", &as1 == &as2)
	// as1 == as2: false

	fmt.Printf("as1[0] == as2[1]: %v\n", &as1[0] == &as2[1])
	// as1[0] == as2[1]: true
}

func main() {
	func1()
	func2()
	func3()
	func4()
}
