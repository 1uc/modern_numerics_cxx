#include <cmath>
#include <iostream>

// Let's start by looking at a "function object" with read only state. A
// function object is a class which has the operator() defined. Let's pick
// something with sines and cosines:
//    f(x) = a*sin(x) + b*cos(x)

class SinusoidalV1 {
public:
  SinusoidalV1(double a, double b) : a(a), b(b) {}

  double operator()(double x) { return a * std::sin(x) + b * std::cos(x); }

private:
  double a;
  double b;
};

// What are the problems?
// 1. Inside operator() we can accidentally change the value of
//    `a` and `b`.
//
// 2. Even tough `operator()` only reads `a` and `b`, we can't do the
//    following:
//        void print_abit(const SinusoidalV1 &sinusoidal) {
//          std::cout << sinusoidal(42.0) << "\n";  // the offending line.
//        }

// The justification for const correctness is to avoid 1. and enable 2. The
// reason why we want 2. is that it clearly states that passing sinusoidal to
// `print_afew` does not change `sinusoidal`. We know this without looking at
// the definition of `print_afew` and any function it might pass `sinusoidal`
// to.

// =============================================================================
// Second attempt.

class Sinusoidal {
public:
  Sinusoidal(double a, double b) : a(a), b(b) {}

  // NEW ---------------------v
  double operator()(double x) const {
    return a * std::sin(x) + b * std::cos(x);
  }

private:
  double a;
  double b;
};

// The `const` at the end of the method promises, that the function will not
// modify its state. Additionally it also promises to no call other methods
// that might cause its state to change.
// Put simply: calling `operator()` will not change the object.

// Now the following works without any problems:
void print_abit(const Sinusoidal &sinusoidal) {
  std::cout << sinusoidal(42.0) << "\n";
}

void print_ref(Sinusoidal &sinusoidal) {
  std::cout << sinusoidal(42.0) << "\n";
}

// There's a nice subtlety to keep in mind:
// We can pass a temporary (or r-value reference) object to a function accepting
// const references; but not to a function accepting non-const references.

int main() {
  // This can get clumsy in say the following:
  print_abit(Sinusoidal(1.0, 2.0));

  auto sinusoidal = Sinusoidal(1.0, 2.0);
  print_ref(sinusoidal);
  // print_ref(Sinusoidal(1.0, 2.0)); // does not compile.

  // If you're wondering how you could possibly remember this:
  //
  //   a) when accepting a const reference you're saying: I'm not modifying the
  //   object, just looking. In particular you don't particularly care where it
  //   is stored. You also don't mind that the input will be destroyed as soon
  //   as the function returns.
  //
  //   b) when accepting a non-const reference you're saying, that you want to
  //   modify the contents of the object. Therefore, if you could pass the
  //   temporary object to the function, whatever modifications you make to the
  //   input would be destroyed before the calling function can observe the
  //   change. Not useful.
  //
  // Naturally, we can have this in different setting such as
  //     print_ref(make_sinusoidal(1.0, 2.0))
  // i.e. directly passing the return value of a function to `print_ref` wont
  // work either.

  return 0;
}

// To achieve const correctness you must:
//   1. Make a decision as to wether a function modifies its argument:
//      * if not pass the argument by value or as a const reference.
//      * if it does: pass by non-const reference.
//
//   2. For each method decide if it modifies the object:
//      * if not: make it a const method.
//      * if it does: don't make it a const method.

// -----------------------------------------------------------------------------
// Exceptions
//

// There algorithms that can be implemented much more efficiently if they have
// access to some internal scratch pad memory. Yet, on a high-level you don't
// consider the method to be modifying the object (except ofc the scratch pad,
// but that doesn't really count).
//
// Slippery slope. There is a way out and it may be the correct choice,
// occationally. Any method defined as `mutable` can be modified from within
// const methods.

class SinusoidalV3 {
public:
  SinusoidalV3(double a, double b) : a(a), b(b) {}

  double operator()(double x) const {
    // Imagine some situation where creating `tmp_` everytime is too expensive,
    // e.g. if one must allocate a small number of doubles only to do very litte
    // work on them.

    // To me the key is that `tmp_` is written before it is read. Meaning no
    // state is carried over from previous runs. Therefore, when dealing with
    // single threaded code, this behaves like it does not modify its state.
    tmp_ = a * b; // Legal, but really not meaningful.
    return a * std::sin(tmp_ * x) + b * std::cos(tmp_ * x);
  }

private:
  double a;
  double b;

  // v--- This allows us to modify `tmp_` from inside a const method.
  mutable double tmp_;
};
