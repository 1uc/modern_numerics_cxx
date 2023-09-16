// In the usecase we'd like to show how to manipulate types by showing how to
// build a struct of array datastructure.
//
// The goal is to write a datastructure that generalizes,
//
//   struct SoA_Int_Double {
//     std::vector<int> int_field;
//     std::vector<double> double_field;
//   };
//
// to support the following API:
//
//   auto soa = SoA<int, double>(n);
//   int i5 = soa.get<0>(5);
//   double d2 = soa.get<1>(2);

// First problem: we can't write one member per template parameter, because we
// don't know how many we have. One natural solution would be to use recursion.
// However, we'd like to demonstrate how to manipulate types. Therefore, we'll
// use existing data structures.
//
// If only we could take `T` and convert it to `std::vector<T>` then we could
// maybe do:
//
//   std::tuple<std::vector<T1>, std::vector<T2>, ...>
//
// to store the vectors.
//
// We need std::vector and std::tuple.
#include <tuple>
#include <vector>
#include <iostream>

// We need a template because we need it to work for different types, we also
// need a variadic template because we need it to work for arbitrary structs,
// some maybe have one member others five.

// This is simply the syntax for a variadic template. The `Args` are referred
// to as a template parameter pack. We'll be allowed to traverse and apply
// transformations on template parameter packs, i.e. on a sequence of types.
template <class... Args>
class SoA {
private:
  // Next we show how the language allows us to apply "functions" to types. The
  // "function" is `std::vector<.>` and we'd like to apply it to each of the
  // template parameters. The magic is in the `...`. The jargon is "pack
  // expansion", see
  //   https://en.cppreference.com/w/cpp/language/parameter_pack
  //
  // What's happening is that the compiler takes `std::vector<Args>` and
  // creates a pattern (or "function") `std::vector<.>`, next for each `T` in
  // `Args...` it'll substitute the T for the `.` in the pattern, i.e. where
  // `Args` use to be. Then it concatenates the expressions, separated by
  // commas.
  //
  // For example, for SoA<int, double> it'll create
  // `std::tuple<std::vector<int>, std::vector<double>>.
  std::tuple<std::vector<Args>...> data;

public:
  // We need a constructor. It'll suffice to have a ctor which creates
  // default initialized vectors of the same size. We're faced with the same
  // problem, we'd like to write
  //
  //   data{std::vector<int>(n), std::vector<double>(n)}
  //
  // but more generically. Same solution: `std::vector<.>(n)` is the
  // "function"/pattern we'd like to apply; and therefore this is the syntax to
  // use:
  SoA(size_t n) : data{std::vector<Args>(n)...} {}

  // Finally, let's add an accessor. Since the user is kind enough to tell us
  // the index of the element of the struct they want, we can simply forward
  // that information to `std::get`.
  template <size_t tuple_index>
  auto& get(size_t vector_index) {
    return std::get<tuple_index>(data)[vector_index];
  }
};

int main() {
  SoA<int, double> soa(6);

  soa.get<0>(5) = 42;
  soa.get<1>(2) = 0.42;

  std::cout << "i5 = " << soa.get<0>(5) << std::endl;
  std::cout << "d2 = " << soa.get<1>(2) << std::endl;

  return 0;
}
