// Compile with
//     g++ -std=c++17 usecase_odes.cpp -o usecase_odes
//
//
// Ambitious project: solve the ODE
//    dy/dt = -2.0 y
// numerically.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

// A RHS of an ODE shall accept a vector `y` and the current time `t`.
// The RHS will store the right hand side into a vector `dydt`.

/// Interface of a RHS.
class RHS {
public:
  virtual ~RHS() = default;

  virtual void operator()(std::vector<double> &dydt,
                          const std::vector<double> &y,
                          double t) const = 0;
};

/// A class implmenting the interface.
class ExpRHS : public RHS {
public:
  ~ExpRHS() override = default;

  void operator()(std::vector<double> &dydt,
                  const std::vector<double> &y,
                  double /* t */) const override {
    for (std::size_t i = 0; i < y.size(); ++i) {
      dydt[i] = -2.0 * y[i];
    }
  }
};

// Okay, we've seen one example of an interface, i.e. `RHS`, and a class
// implementing this interface, i.e. `ExpRHS`.
//
// Let's use the same pattern again for a Runge-Kutta step.

/// Interface of one step of a RK method.
class RKStep {
public:
  virtual ~RKStep() = default;

  // It starts with `virtual`, this signals that subclasses can override this
  // method.
  //
  // It ends in ` = 0`, because this is an abstract base class which defines
  // the interface. You don't specify any operations, you only state the
  // methods that all subclasses *must* implement. To prevent the compiler from
  // complaining that you haven't defined the methods, we add the `= 0`. A
  // class is termed abstract if it (or any of its base classes) have one
  // method which is abstract, i.e. ends in `= 0` (that has not been
  // overridden).
  //
  // You should also define on a high-level, what this method does, e.g.
  // "`advance` will take the current state `y0` (approx. y(t)) and advance it
  // to `y1` (approx. y(t + dt))".
  virtual void advance(std::vector<double> &y1,
                       const std::vector<double> &y0,
                       double t,
                       double dt) const = 0;
};

// To implement one step of Forward Euler we must create a new class and
// inherit from `RKStep`. The `: public RKStep` denotes inheritance. The
// `public` is not optional and while there are other things you can write
// there `public` is the only sane choice in all but some very rare cases.
// (I think I've never used anything else, yet.)
class ForwardEulerStep : public RKStep {
public:
  ~ForwardEulerStep() override = default;

  // 1. We must store the right hand side.
  // 2. We will need a place to store the rate of change `dydt`. We want to
  //    preallocate a vector. Hence we must know how big the vector is.
  //    Generally, here you set up any state that is important to the subclass,
  //    but not general to every subclass.
  //
  // You could argue that the `rhs` is something every subclass needs.
  // Nevertheless, you should not make it part of `RKStep`. Instead you can
  // have a class in between and have all "normal" RK steps inherit from that
  // class. You never know if there wont at some point be a reason to have an
  // abnormal `RKStep` which doesn't need or worse can't deal with a RHS.
  ForwardEulerStep(std::shared_ptr<RHS> rhs, std::size_t n_vars)
      : rhs(std::move(rhs)), dydt(n_vars) {

    // You probably noticed the `shared_ptr`. You can ignore it for now. The
    // pattern of accepting it by value and then moving it is intentional.
    // However, this is also a separate pattern/topic.
  }

  // Time to override `advance`. The modern way is to have override at the end
  // (because having `override` will trigger essential warnings if you're not
  // actually overriding anything, e.g., due to subtle difference in the
  // signature).
  void advance(std::vector<double> &y1,
               const std::vector<double> &y0,
               double t,
               double dt) const override {
    assert(y1.size() == y0.size());

    (*rhs)(dydt, y0, t);

    for (std::size_t i = 0; i < y0.size(); ++i) {
      y1[i] = y0[i] + dt * dydt[i];
    }
  }

private:
  std::shared_ptr<RHS> rhs; // Just an example with a `shared_ptr`.

  // Here we can put stuff the outside world doesn't need to know about.
  mutable std::vector<double> dydt;
  // `mutable`?! so `advance` is const, i.e. it promises to not change itself.
  // If you're strict about this, then you can't have any scratch pad like
  // internal state. However, we need some. Any `mutable` member variable
  // ignores the `const` requirement of methods, e.g. we can change it even in
  // a `const` method. This has great potential for misuse.
  //
  // This gets really slippery when combined with threads.
};

// This is just some code using the previously defined interfaces.
std::vector<double>
solve_ode(const RKStep &rk_step, std::vector<double> y0, double T, double dt) {
  // Detail: we want to reuse y0. Therefore, the slightly costly but
  // non-confusing solution to accept `y0` by value.

  std::vector<double> y1(y0.size());

  double t = 0.0;
  while (t < T) {
    rk_step.advance(y1, y0, t, dt);

    std::swap(y1, y0);
    t += dt;
  }

  return y0;
}

// This is a factory, it can be used to generate a polymorphic object, given
// runtime (as opposed to compile time) information. Here the string
// `rhs_name`.
//
// You'll notice it returns a (smart pointer) to a `RHS`, i.e. the base class.
// You'll understand why at the latest once you add another right hand side.
std::shared_ptr<RHS> make_rhs(const std::string &rhs_name) {
  if (rhs_name == "exp") {
    return std::shared_ptr<ExpRHS>();
  }

  return nullptr; // probably you'd throw an exception instead.
}

// Some uninteresting code.
std::vector<double> ic() { return std::vector<double>{1.0, 2.0, 3.0}; }

// More uninteresting code.
std::vector<double> soln(double t) {
  double e = std::exp(-2.0 * t);
  return std::vector<double>{1.0 * e, 2.0 * e, 3.0 * e};
}

int main() {
  double T = 1.0;
  double dt = 0.01;

  // For teaching purposes, we will solve the same ODE several times. This
  // either resembles a Monte Carlo setting, or because solving the ODE is only
  // one part of some more compilicated algorithm.
  for (int i = 0; i < 3; ++i) {
    auto y0 = ic();

    auto rhs = std::make_shared<ExpRHS>();
    auto rk_step = ForwardEulerStep(rhs, y0.size());

    auto y1 = solve_ode(rk_step, y0, T, dt);
    auto y_exact = soln(T);

    std::cout << "Error: " << y1[0] - y_exact[0] << ", " << y1[1] - y_exact[1]
              << ", " << y1[2] - y_exact[2] << "\n";

    // The interesting part happens here:

    // Yes, exactly there is no code here to delete the `rhs`. If we were not
    // using a smart pointer, this would be a memory leak. However, since we've
    // used a smart pointer the RHS is cleaned up as expected.  Forgetting a
    // `delete` is a) easy to the point that it's the default thing to do; b)
    // hard to debug since there's no failure until you run out of memory. This
    // alone is reason enough to use a smart pointer.
  }

  return 0;
}

// Exercises:
// 1. Implement another RK method and use it.
// 2. Implement another right hand side, extend the factory `make_rhs` and
//    finally use the new RHS.
