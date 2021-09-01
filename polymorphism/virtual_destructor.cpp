// Compile with
//     g++ -std=c++17 virtual_destructor.cpp -o virtual_destructor

#include <iostream>
#include <memory>

// Let's set up a class hierarchy and focus on which destructor is called.

// Simply a base class.
class Base {
public:
  virtual ~Base() { std::cout << "Base::~Base() \n"; }
};

// ... first derived class.
class Derived : public Base {
public:
  ~Derived() override { std::cout << "Derived::~Derived() \n"; }
};

// ... and a third level.
class MoreDerived : public Derived {
public:
  ~MoreDerived() override { std::cout << "MoreDerived::~MoreDerived() \n"; }
};

// A small factory for creating objects. Note that a smart pointer to the
// base class is returned.
std::unique_ptr<Base> make_obj(const std::string &key) {
  if (key == "base") {
    return std::make_unique<Base>();
  } else if (key == "derived") {
    return std::make_unique<Derived>();
  } else {
    return std::make_unique<MoreDerived>();
  }
}

void create_and_destroy(const std::string &key) {
  // Let's create an object and manage its lifetime with a smart pointer.
  auto b = make_obj(key);

  // The smart pointer is about to go out of scope and as a result the object
  // it points to will also be destroyed.
  //
  // Important, the smart pointer has type `std::unique_ptr<Base>`. Therefore,
  // when it calls the destructor the conventional rules w.r.t. polymorphism
  // apply.
  //
  // Since we've declared the destructor to be `virtual` the destructor of the
  // actual type of the object is called, as opposed to `Base::~Base`.
  // Naturally, the destructor recursively calls the destructor of its
  // immediate base class(es) to ensure everything gets cleaned up correctly.
}

int main() {
  std::cout << "-- Base ----------------------------------------------\n";
  create_and_destroy("base");

  std::cout << "-- Derived -------------------------------------------\n";
  create_and_destroy("derived");

  std::cout << "-- MoreDerived ---------------------------------------\n";
  create_and_destroy("more_derived");

  return 0;
}

// Final remarks:
//   * Please experiment with the behaviour when then destructors are not
//     virtual. Observe closely what gets called, and take note of the potential
//     implications w.r.t. resource leaks.
//
//   * If you're confused, there's no need to be. Since this behaves exactly
//     like normal polymorphism. The only reason it's mentioned separately is
//     because it's very easy to forget and very import not to.
