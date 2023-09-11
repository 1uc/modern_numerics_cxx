#include <limits>
#include <type_traits>
#include <vector>

void common_limits() {
  // std::numeric_limits is a traits class which provides access to information
  // such as the largest or smallest possible value the various integer and
  // floating point types can take.
  auto eps = std::numeric_limits<double>::lowest();
  auto magic_value = std::numeric_limits<size_t>::max();
}

// Compute the minimum of `values`.
//
// Assumption: T can be either a floating point number or some integer type.
template <class T>
T minimum(const std::vector<T> &values);

namespace v1 {
template <class T>
T minimum(const std::vector<T> &values) {
  T xmin = values[0];

  // This loop doesn't traverse all elements. Instead the first iteration
  // has been dealt with as a special case.
  //
  // For purposes of explaining traits we pretend that we want to eliminate
  // this special case.
  for (size_t i = 1; i < values.size(); ++i) {
    xmin = std::min(xmin, values[i]);
  }
  return xmin;
}

}

namespace v2 {

template <class T>
T minimum(const std::vector<T> &values) {
  // We can pick a suitable value to initialize `xmin`...
  double xmin = std::numeric_limits<T>::max();

  // ... and this loop is not in standard form.
  for (auto x : values) {
    xmin = std::min(xmin, x);
  }

  return xmin;
}

}

namespace v3 {

// To demonstrate how to write/use traits we would like the minimum of an empty
// vector to be `inf` for floating point numbers and the largest representable
// number for integers.

// We need a struct that can serve as the trait.
template <class T>
struct minimum_traits;

// We could write one specialization for `int` ...
template <>
struct minimum_traits<int> {
  static int initialization_value() { return std::numeric_limits<int>::max(); }
};

// ... and another for double.
template <>
struct minimum_traits<double> {
  static double initialization_value() {
    return std::numeric_limits<double>::infinity();
  }
};

// This is a small gain, since we've moved the configurable parts outside the
// implementation of minimum; or put differently we can inject implementation
// details into `minimum` at compile time, without modifying the implementation
// of `minimum`.
template <class T>
T minimum(const std::vector<T> &values) {
  double xmin = minimum_traits<T>::initialization_value();

  for (auto x : values) {
    xmin = std::min(xmin, x);
  }

  return xmin;
}

}

namespace v4 {
// `v3` failed because we needed to write one specialization per type. That's
// too much repetition; and we can logically group all integer types and all
// floating point numbers together.

// Let's start over, again with a struct. This time is has two template
// parameters. One for the type; and another to play tricks with the compiler
// (we didn't bother giving it an name).
//
// It would be convenient if we could tell the compiler to ignore certain
// definitions, because that would allow us to write one version for integers
// and another for floating point numbers. There is a way, namely by writing
// code that has what looks like a bug. If that bug happens one of a few very
// particular locations then it falls under Substitution Failure Is Not An
// Error (SFINAE). Crucially, this mean that "the bug" doesn't result in a
// compiler error and instead that specialization will be ignored.
//
// Unfortunately, the body of the method is not covered by SFINAE. Instead we
// use a second template argument to create a place were we can trigger
// substitution failures.
template <class T, class = void>
struct minimum_traits;

// We will write a specialization of `minimum_traits<T, void>`. However, this
// one is only considered when `std::is_integral_v<T> == true`. If not, then
// `std::enable_if<std::is_integral_v<T>>::type` is a substitution failure,
// because `std::enable_if<false>` has no type definition `type`. Hence, it's a
// substitution failure and the specialization is ignored.
//
// Otherwise, `std::enable_if<true>::type == void` which means it's a
// specialization of `minimum<T, void>`.
template <class T>
struct minimum_traits<T, typename std::enable_if<std::is_integral_v<T>>::type> {
  static T initialization_value() { return std::numeric_limits<T>::max(); }
};

// We can reduce typing overhead; and maybe improve readability by writing our
// own `enable_if_*` traits. Note that it will inherit a typename `type` if and
// only if `std::is_floating_point_v<T>` is true. Meaning it the same
// substitution failure as the regular `std::enable_if`.
template <class T, class U = void>
struct enable_if_floating_point : std::enable_if<std::is_floating_point_v<T>> {
};

template <class T>
struct minimum_traits<T, typename enable_if_floating_point<T>::type> {
  static T initialization_value() { return std::numeric_limits<T>::infinity(); }
};

template <class T>
T minimum(const std::vector<T> &values) {
  double xmin = minimum_traits<T>::initialization_value();

  for (auto x : values) {
    xmin = std::min(xmin, x);
  }

  return xmin;
}

}
