#include <iostream>
#include <cstdlib>
#include <cstring>

// Here we demonstrate a slightly subtle memory bug that's related to using
// `malloc` instead of `new`.
//
// It explains how `new` and `delete` are very different from `malloc` and
// `free`; and shows when the difference matters when using RAII.
//
// For the (rare) cases where one must use `malloc`, e.g. because one's dealing
// with a C API, it also shows how one can fix the bug by using placement new.
// Almost anything is cleaner than using placement new. In modern C++ it's not
// common at all. That said, if the choice is between placement new and a
// segfault, go with placement new.

namespace v1 {
int dtor_calls = 0;

// We start off with a simplified class that has a default ctor; and a
// non-trivial dtor.
class A {
public:
  A() = default;
  ~A() { dtor_calls += 1; }

  double a = 42.0;
};

// Someone implemented a C API we need to use to allocate A.
A* allocate() {
  return (A*) malloc(sizeof(A));
}

// ... and a matching function to deallocate A.
void deallocate(A* a) {
  free(a);
}

void lifecycle() {
  // We must use the C API to allocate `a`.
  A* a = allocate();

  std::cout << "after allocate: a.a = " << a->a << "\n";

  // That's bad because `a` has not been default allocated. Can we get the
  // default ctor to run? Yes:
  new(a) A;

  std::cout << "after placement new: a.a = " << a->a << "\n";

  // Because we need to call a C API to deallocate `a`, we know it can't/won't
  // run the dtor of `a`. So we need to do so ourselves:
  a->~A();

  deallocate(a);
  std::cout << "dtor_calls = " << dtor_calls << "\n";

}
}

// The example above is too trivial. One might be tempted to say: just make
// sure you assign before you read from `a`. Which happens to work in some
// cases.

namespace v2 {
int dtor_calls = 0;

// Let's look at a heap allocated array of doubles.
class array {
public:
  array() = default;
  array(size_t n) : ptr((double*) malloc(n*sizeof(double))) {}
  ~array() {
    dtor_calls += 1;
    free(ptr);
  }

  const array& operator=(array&& other) {
    free(ptr); // free checks that `ptr` isn't null.

    ptr = other.ptr;
    other.ptr = nullptr;
    return *this;
  }

  double * ptr = nullptr;
};

// Someone implemented a C API we need to use to allocate `array`.
array* allocate() {
  auto * a = (array*) malloc(sizeof(array));
  // We simulate that the memory wasn't zeroed out:
  memset(a, 255, sizeof(array));

  return a;
}

// ... and a matching function to deallocate `array`.
void deallocate(array* a) {
  free(a);
}

void lifecycle() {
  auto * a = allocate();
  // As expected, a is in some invalid state and anyway we need it to hold `n`
  // doubles. It might be tempting to do:
  // a = array(n);

  // ... but that's rewarded with a segfault.
  std::cout << "a->ptr = " << a->ptr << "\n";

  // The problem is that we need `ptr` to be `nullptr` if and only if it
  // doesn't point to valid memory. In the move assignment, we need to know
  // that when `ptr != nullptr` it's safe to free the memory pointed to by
  // `ptr`.

  new(a) array;

  // Now it's safe to assign to `a`:
  size_t n = 42;
  *a = array(n);

  // Alternatively, we can call a non-default ctor:
  // new(a) array(42);

  // We've already destroyed the temporary `array(n)` that we
  // moved to `*a`.
  std::cout << "before ~array = " << dtor_calls << "\n";

  a->~array();
  std::cout << "after ~array = " << dtor_calls << "\n";

  deallocate(a);
  std::cout << "after deallocate = " << dtor_calls << "\n";
}

}

namespace v3 {
// The problem is recursive. The following will fail for `array<array<T>>` for
// the exact same reasons explained above.
template<class T>
class array {
public:
  array() = default;
  array(size_t n) : ptr((T*) malloc(n*sizeof(T))) {}
  ~array() {
    free(ptr);
  }

  const array& operator=(array&& other) {
    free(ptr); // free checks that `ptr` isn't null.

    ptr = other.ptr;
    other.ptr = nullptr;
    return *this;
  }

  T* ptr = nullptr;
};
}


int main() {
  v1::lifecycle();
  v2::lifecycle();
}
