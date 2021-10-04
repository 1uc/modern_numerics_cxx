// Compile with
//     g++ -Wall -Wextra -std=c++17 syntax.cpp -o syntax

// Topic: The essential syntax required to write polymorphic classes.

// clang-format off
#include <iostream>

// This is a base class with two methods, one polymorphic and another that
// isn't.
class Base {
public:
  virtual void poly()     { std::cout << "Base::poly() \n"; }
          void non_poly() { std::cout << "Base::non_poly() \n"; }
};

// This subclass overrides / redefines the two methods from the base class.
class Derived : public Base {
public:
  void poly()     override { std::cout << "Derived::poly() \n"; }
  void non_poly()          { std::cout << "Derived::non_poly() \n"; }
};

class MoreDerived : public Derived {
public:
  void poly()     override { std::cout << "MoreDerived::poly() \n"; }
  void non_poly()          { std::cout << "MoreDerived::non_poly() \n"; }
};

class OldStyleDerived : public Base {
public:
  void poly()     { std::cout << "OldStyleDerived::poly() \n"; }
  void non_poly() { std::cout << "OldStyleDerived::non_poly() \n"; }
};

class OldStyleDerivedWithTypo : public Base {
public:
  // This ----vvvvv is a subtle mistake, with grave consequences, at runtime.
  void poly() const { std::cout << "OldStyleDerivedWithTypo::poly() \n"; }

  // With `override` this bug is caught at compile time:
  // void poly() const override { std::cout << "OldStyleDerivedWithTypo::poly() \n"; }

  void non_poly() { std::cout << "OldStyleDerivedWithTypo::non_poly() \n"; }
};


// The important thing to observe is that `b` is a reference of type `Base` and
// we can pass not only objects of type `Base` but also of a type derived from
// `Base`.
//
// The question is: how does this piece of code behave?
void use_through_base(Base &b) {
  b.poly();
  b.non_poly();
}

// Write down the output, before running the executable it :)
int main() {
  Base b;
  Derived d;
  MoreDerived r;

  // Note that we are using `b` and `d` directly. Meaning `b` has type `Base`
  // and `d` has type `Derived`.
  b.poly();
  b.non_poly();

  d.poly();
  d.non_poly();

  r.poly();
  r.non_poly();
  std::cout << "----------------------------------------\n";

  use_through_base(b);
  use_through_base(d);
  use_through_base(r);

  std::cout << "========================================\n";

  OldStyleDerived o;
  OldStyleDerivedWithTypo ot;

  o.poly();
  o.non_poly();

  ot.poly();
  ot.non_poly();

  std::cout << "----------------------------------------\n";

  use_through_base(o);
  use_through_base(ot);


  return 0;
}

// Closing remarks:
//   * This code isn't const correct. Making this code const correct is not
//     related to polymorphism. You can apply the ideas from the
//     `const_correctness` section.
//
//     There are const correct examples in this folder.
//
//   * Please experiment by adding and removing `virtual` and `override` to
//     `poly` in all meaningful combinations, especially those corresponding to
//     errors due to inattentiveness.
//
//     You should notice that adding `virtual` (but not `override`) only in
//     `Derived` or `MoreDerived` does not provide the desired polymorphic
//     behaviour; and the compiler will not alert you to this fact.
//
//     You should notice that using `override` is the only way of stating that
//     this function overrides a method and instruct the compiler to produce an
//     error if it doesn't.
//
//     The `OldStyleDerivedWithTypo` demonstrates how you could easily make
//     mistakes, by having subtly mismatching signatures.
//
//   * One could redundantly add `virtual` in `Derived::poly`. This is not
//     recommended by the Core Guidelines [1].
//
//   * The Core Guidelines [1] explain the reasoning very well.
//
//     [1]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-override
