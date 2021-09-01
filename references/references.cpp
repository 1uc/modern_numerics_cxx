#include <iostream>
#include <vector>

// In C++ there are three good ways of passing arguments to a function. Either
// by value, by reference or by const reference.
//
// by value: the argument is copied and the function receives the copy of the
//           argument. The function is permitted to modify the copy. Naturally
//           these modifications to the argument will be lost when the function
//           returns.
void foo_by_val(std::vector<double> x) {
  x[0] = -42.0;
  std::cout << "foo_by_val: x[0] = " << x[0] << "\n";
}

// by reference: a 'reference' to the argument is passed to the function, i.e.
//           the function is essentially told here is the address of the
//           argument. The function is permitted to modify the argument, and
//           since it is operating on the same object, the changes are visible
//           outside of the function.
//
//           This is almost exclusively used for output arguments.
void foo_by_ref(std::vector<double> &x) {
  x[0] = -42.0;
  std::cout << "foo_by_ref: x[0] = " << x[0] << "\n";
}

// by const reference: a 'const reference' is passed to the function. This is
//           just like a reference, but the function is not permitted to modify
//           the argument.
//
//           This is used when you don't want or need a full copy, and don't
//           want to modify the argument. This is almost always the correct way
//           of passing read-only arguments. Exceptions are very small objects,
//           e.g. `double`, `float`, etc.
void foo_by_cref(const std::vector<double> &x) {
  // x[0] = -42.0;  // Not possible.
  std::cout << "foo_by_cref: x[0] = " << x[0] << "\n";
}

// The motivation for using a (non-) const reference is clear: either you
// want to modify the object and must use a non-const reference, or you don't
// want to be able to modify the object and accept a const reference instead.

// Then why do we ever need to pass large objects by value? Well, if we need a
// copy anyway. For example because it can only be efficiently implemented if
// it is allowed to modify its argument, but these modification have no meaning
// outside of the function. One option would be to allocate a new copy inside
// the function, or, since there is anyway going to be a copy accept the
// argument by value.  So far nothing gained, but what if the place where the
// function is called knows it will no longer need the argument? Well, then it
// can move the argument when passing it to the function.

// These three options of passing arguments are not specific to functions and
// also works for methods and constructors. The most common use for passing
// large objects by value are constructors. Let's look at one:

class Foo {
public:
  // Since ultimately want a copy of the vector and store it in `x_`. We can
  // make the copy right away. Then instead of copy constructing `x` from `x_`
  // we can use the move constructor to initialize `x_`.
  Foo(std::vector<double> x) : x_(std::move(x)) {
    // When using this pattern always use a different name for the argument
    // and the member, since at this point, `x` refers to the argument to the
    // constructor and not a member also called `x`.
    //
    // Also by this time, you must no longer use `x`, since it has been moved.
    //
    // Source of many annoying errors.
  }

private:
  std::vector<double> x_;
};

int main() {

  std::vector<double> x{0.0, 1.0, 2.0};

  std::cout << "pre: x[0] = " << x[0] << "\n";
  foo_by_val(x);
  std::cout << "post: x[0] = " << x[0] << "\n";
  std::cout << "--------------------\n";

  std::cout << "pre: x[0] = " << x[0] << "\n";
  foo_by_cref(x);
  std::cout << "post: x[0] = " << x[0] << "\n";
  std::cout << "--------------------\n";

  std::cout << "pre: x[0] = " << x[0] << "\n";
  foo_by_ref(x);
  std::cout << "post: x[0] = " << x[0] << "\n";
  std::cout << "--------------------\n";

  // We don't need `x` anymore and `Foo` can take over the ownership of `x`.
  auto foo = Foo(std::move(x));

  // You must not use `x` as of here. This is slippery and exactly the type of
  // code we want to avoid. The issue is that the name `x` remains valid after
  // it ceased to be useful. Therefore, avoid having many lines of code where
  // one could accidentally use `x`.
  //
  // Use static code analysis, e.g. clang-tidy, to detect use-after-move.

  return 0;
}
