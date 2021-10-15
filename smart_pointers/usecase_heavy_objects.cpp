// Compile with
//     g++ -Wall -Wextra -std=c++17 usecase_heavy_objects.cpp \
//         -o usecase_heavy_objects
//
// Topic: Example of a smart pointer for heavy objects.

#include <memory>
#include <vector>

// In FVM simulations one frequently needs a grid. In the unstructured case it's
// quite heavy, since the vertices, cell-centers and incidence needs to be
// stored.
struct Grid {
  std::vector<double> cell_centers; // magically only has one coordinate.
};

std::shared_ptr<Grid> make_uniform_grid(int nx, int ny) {
  auto grid = std::make_unique<Grid>();
  grid->cell_centers = std::vector<double>(nx * ny);

  // compute the cell-centers, etc.

  return grid;
}

// A functor to compute cell-averages of a functions.
class CellAverage {
public:
  // CellAverage should share ownership of the grid. Meaning the grid must not
  // be deallocated before this object is destroyed. If it were, there would be
  // a dangling pointer.
  CellAverage(std::shared_ptr<Grid> grid) : grid_(std::move(grid)) {}

  template <class F>
  void operator()(std::vector<double> &u_bar, const F &f) const {
    cell_average(u_bar, *grid_, f);
  }

private:
  std::shared_ptr<Grid> grid_; // It can have access to the grid.
};

// By contrast, `cell_average` does not need to keep the grid alive explicitly.
// Assuming that `cell_average` wasn't passed a dangling reference, we can be
// sure, that the object `grid` refers to, will only be destroyed after this
// function returns.
//
// Therefore, it's best accept a `Grid` instead of a `std::shared_ptr<Grid>`.
template <class F>
void cell_average(std::vector<double> &u_bar, const Grid &grid, const F &f) {
  for (int i = 0; i < grid.cell_centers.size(); ++i) {
    u_bar[i] = f(grid.cell_centers[i]); // mid-point rule.
  }
}

// The right-hand side of an ODE, which is space dependent.
class SpaceDependentRHS {
public:
  SpaceDependentRHS(std::shared_ptr<Grid> grid) : grid_(std::move(grid)) {}

  void operator()(std::vector<double> &dudt) const {
    for (int i = 0; i < grid_->cell_centers.size(); ++i) {
      dudt[i] = 42.0; // some dummy operation.
    }
  }

private:
  std::shared_ptr<Grid> grid_; // This also has access to the grid.
};

// A class to create some complexity.
class SomeODESolver {
public:
  SomeODESolver(SpaceDependentRHS rhs) : rhs_(std::move(rhs)) {}
  void operator()(std::vector<double> &u1,
                  const std::vector<double> &u0,
                  double dt) const {
    rhs_(u1);
    for (int i = 0; i < u1.size(); ++i) {
      u1[i] = u0[i] + dt * u1[i];
    }
  }

private:
  SpaceDependentRHS rhs_;
};

SomeODESolver make_ode_solver(std::shared_ptr<Grid> grid) {
  auto rhs = SpaceDependentRHS(std::move(grid));
  return SomeODESolver(std::move(rhs));
}

std::tuple<int, CellAverage, SomeODESolver> make_simulation() {
  auto grid = make_uniform_grid(100, 100);
  auto avg = CellAverage(grid);
  auto ode_solver = make_ode_solver(grid);
  int n_cells = grid->cell_centers.size();

  return {n_cells, std::move(avg), std::move(ode_solver)};
}

int main() {
  auto [n_cells, avg, ode_solver] = make_simulation();

  std::vector<double> u0(n_cells);
  avg(u0, [](double x) { return x * x; });

  std::vector<double> u1(n_cells);
  ode_solver(u1, u0, 0.0001);

  // Even though we don't have direct access to the grid, and therefore can't
  // tidy it up, it will be cleaned up properly, due to how a shared pointer
  // works.
};

// Closing remarks:
//
//   * Multiple objects, also nested, can easily share access to that one grid.
//
//   * Thanks to the shared pointer destroying the grid "as soon as nobody needs
//     it anymore" is a sensible statement, and is exactly what happens.
//
//   * Even though `CellAverage` and `SpaceDependentRHS` contain a heavy
//     object, namely the grid, copying objects of either type is relatively
//     light-weight.
