#include <iostream>

class A {};

void func1() {
    std::cout << "sizeof(A): " << sizeof(A) << std::endl;
    // sizeof(A): 1

    A a1;
    A a2;
    std::cout << "address(a1): " << &a1 << std::endl;
    std::cout << "address(a2): " << &a2 << std::endl;
    std::cout << "&a1 == &a2: " << (&a1 == &a2) << std::endl;
    // address(a1): 0x7ffee84b84a8
    // address(a2): 0x7ffee84b84a0
    // &a1 == &a2: 0

    A* a3 = new A();
    A* a4 = new A();
    std::cout << "address(a3): " << &a3 << std::endl;
    std::cout << "address(a4): " << &a4 << std::endl;
    std::cout << "&a3 == &a4: " << (&a3 == &a4) << std::endl;
    // address(a3): 0x7ffee84b8498
    // address(a4): 0x7ffee84b8490
    // &a3 == &a4: 0
}

class B {
    A a1;
    A a2;
};

void func2() {
    std::cout << "sizeof(B): " << sizeof(B) << std::endl;
    // sizeof(B): 2

    B b1;
    B b2;
    std::cout << "address(b1): " << &b1 << std::endl;
    std::cout << "address(b2): " << &b2 << std::endl;
    std::cout << "&b1 == &b2: " << (&b1 == &b2) << std::endl;
    // address(b1): 0x7ffee84b84d8
    // address(b2): 0x7ffee84b84d0
    // &b1 == &b2: 0
}

class C : public A {};

void func3() {
    std::cout << "sizeof(C): " << sizeof(C) << std::endl;
    // sizeof(C): 1

    C c1;
    C c2;
    std::cout << "address(c1): " << &c1 << std::endl;
    std::cout << "address(c2): " << &c2 << std::endl;
    std::cout << "&c1 == &c2: " << (&c1 == &c2) << std::endl;
    // address(c1): 0x7ffee84b84d8
    // address(c2): 0x7ffee84b84d0
    // &c1 == &c2: 0
}

class D : public A {
    int i;
};

void func4() {
    std::cout << "sizeof(D): " << sizeof(D) << std::endl;
    // sizeof(D): 4

    D d1;
    D d2;
    std::cout << "address(d1): " << &d1 << std::endl;
    std::cout << "address(d2): " << &d2 << std::endl;
    std::cout << "&d1 == &d2: " << (&d1 == &d2) << std::endl;
    // address(d1): 0x7ffee84b84d8
    // address(d2): 0x7ffee84b84d0
    // &d1 == &d2: 0
}

int main(int argc, const char* argv[]) {
    func1();
    func2();
    func3();
    func4();
    return 0;
}
