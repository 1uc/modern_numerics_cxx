#include <cstddef>
#include <iostream>
#include <tuple>

void usage() {
  // Usage of `std::tuple` is simple and intuitive.
  auto ix = std::tuple<int, double>{1, 3.0};

  auto i = std::get<0>(ix);
  auto x = std::get<1>(ix);
}

// The question is, how does any of this even work? Note that clearly it's not
// just a very extensive list of explicit specializations.

namespace v1 {
// We need a way of creating a struct with the right elements. One way to do so
// is recursion and inheritance. This has the side effect that the elements
// will be arranged in reverse order. Unfortunately, using recursion and
// aggregation is met with a minor road block that simply obscures the
// essentials.

// A template class we can specialize.
template <class... Args>
class tuple_impl;

// The recursion works by giving one type a name; and recurse on the rest.
template <class T, class... Args>
class tuple_impl<T, Args...> : public tuple_impl<Args...> {
private:
  // Now we have a place to store a value of type `T`.
  T value;
};

// Once we're out of types, we stop the recursion.
template <>
class tuple_impl<> {};

}

namespace v2 {
// We need a way of accessing the values. To do so we will number the arguments
// and expose that information to `tuple_impl`. We'd like to implement:
//     auto ix = ...;
//     auto i = ix.get<0>();
//     auto x = ix.get<1>();

// A template class we can specialize.
template <size_t index, class... Args>
class tuple_impl;

template <size_t index, class T, class... Args>
class tuple_impl<index, T, Args...> : public tuple_impl<index + 1, Args...> {
private:
  using super = tuple_impl<index + 1, Args...>;

public:
  // We'd like to specialize `get<index>()` and make it return our `value`.
  // Sounds like a task for enable_if. We can use the return type as the place
  // to cause SFINAE.
  template <size_t requested_index>
  typename std::enable_if<requested_index == index, T>::type get() {
    return value;
  }

  // We'd like the inherited `get` specialization to be visible:
  using super::get;

private:
  T value;
};

template <size_t index>
class tuple_impl<index> {
public:
  // End of the recursion.
  template <size_t requested_index>
  typename std::enable_if<requested_index >= index, void>::type get() {
    static_assert(false, "Past the end.");
  }
};

}

namespace v3 {
// We need a constructor.

template <size_t index, class... Args>
class tuple_impl;

template <size_t index, class T, class... Args>
class tuple_impl<index, T, Args...> : public tuple_impl<index + 1, Args...> {
private:
  using super = tuple_impl<index + 1, Args...>;

public:
  // Same trick, peal of one type by giving it a name and recurse on the rest.
  // Here you can observe that the elements are initialized in reverse order.
  tuple_impl(T value, Args &&...args)
      : super(std::forward<Args>(args)...), value(value) {}

  using super::get;
  template <size_t requested_index>
  typename std::enable_if<requested_index == index, T>::type get() {
    return value;
  }

private:
  T value;
};

// Unchanged.
template <size_t index>
class tuple_impl<index> {
public:
  template <size_t requested_index>
  typename std::enable_if<requested_index == index, void>::type get() {
    static_assert(false, "Past the end.");
  }
};

}

// Time for some final window dressing.
template <class... Args>
class tuple {
public:
  tuple(Args &&...args) : impl(std::forward<Args>(args)...) {}

  template <size_t requested_index>
  auto get() {
    // Remember, since `impl` depends on our template parameters, `impl.get`
    // could be anything. Therefore, the compiler demands help: `template` if
    // its a template; and `typename` if it had been a type.
    return impl.template get<requested_index>();
  }

private:
  // Using aggregation will prevent users from writing code that takes a const
  // reference to the base type. Hence, this hides the implementation a little
  // bit better.
  v3::tuple_impl<0, Args...> impl;
};

int main() {
  auto ix = tuple<int, double>(1, 3.1);

  // Note that due to alignment/padding, this isn't `sizeof(int) +
  // sizeof(double)`.
  static_assert(sizeof(tuple<int, double>) == sizeof(std::tuple<int, double>),
                "Size differs from std::tuple.");

  std::cout << ix.get<0>() << std::endl;
  std::cout << ix.get<1>() << std::endl;

  // Causes a compilation error:
  // std::cout << ix.get<2>() << std::endl;
}
