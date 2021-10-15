// The header you need to include.
#include <memory>

#include <iostream>

// A class for later use.
class Foo {
public:
  Foo(int a, int b) : a(a), b(b) {}

  void print() const { std::cout << "a = " << a << ", b = " << b << "\n"; }

private:
  int a;
  int b;
};

class BarWithFoo {
public:
  BarWithFoo(std::shared_ptr<Foo> foo_) : foo(std::move(foo_)) {}

private:
  std::shared_ptr<Foo> foo;
};

int main() {
  // If ever possible use `make_shared` to create shared pointers:
  auto shared_foo = std::make_shared<Foo>(1, 2);

  // ... and `make_unique` to create unique pointers:
  auto unique_foo = std::make_unique<Foo>(4, 8);

  // Both behave like pointers.
  shared_foo->print();
  (*unique_foo).print();

  // We can pass a smart pointer to a class.
  auto bar = BarWithFoo(shared_foo);

  return 0;
}

// Closing remarks:
//   * The syntax is delightfully straightforward.
//   * These smart pointers allow you to avoid `new` and `delete`. Resulting
//     in significantly simpler code and fewer memory problems.
