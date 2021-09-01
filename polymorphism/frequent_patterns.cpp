// Compile with
//     g++ -std=c++17 frequent_patterns.cpp -o frequent_patterns

#include <iostream>

// The simple pattern is to have an abstract base class, aka interface. You'll
// then implement the interface, but any code using your classes will only deal
// with the pointers or references to the interface.

// Variant 1: The interface defines the API using abstract methods as shown
// below.

// Do your utmost to avoid implementing anything at this level of abstraction.
// Only define the API, i.e. which methods exist and what they do on an
// appropriately high level of abstraction.
class AbstractBaseClass {
public:
  // See the topic on virtual destructors.
  virtual ~AbstractBaseClass() = default;

  // Virtual (`virtual`) and abstract (`= 0`).
  virtual void method() = 0;
};

// This might be the level at which you implement the common functionality of
// "all" derived classes.
class PartialImplementation : public AbstractBaseClass {
public:
  // Use `override` (in modern C++) to ask the compiler to check that you
  // really are overriding.
  void method() override {
    // As an example: we want to generate a string with a particular formatting,
    // but one part of the string depends on details of the subclasses.
    std::cout << "PartialImplementation: " << message() << "\n";
  }

protected:
  // We can keep playing this game... and delegate implementing the details
  // to the derived classes. Here we choose not to implement a default. Hence,
  // `PartialImplementation::message` is abstract.
  virtual std::string message() = 0;
};

class FullImplementation : public PartialImplementation {
protected:
  // here the variety comes in.
  std::string message() override { return "FullImplementation"; }
};

class SmileyImplementation : public PartialImplementation {
protected:
  // here the variety comes in.
  std::string message() override { return ":)"; }
};

// -----------------------------------------------------------------------------
// Variant 2: A slightly more complex pattern is the following one I learnt
// from Sutter's Mill, e.g.,
// https://herbsutter.com/2013/05/22/gotw-5-solution-overriding-virtual-functions/
//
// and despite the additional noise/uglyness, there is merit to the pattern.
// Also and maybe particularly for numerical codes.
class NonVirtualInterface {
public:
  virtual ~NonVirtualInterface() = default;

  // Not virtual!
  void method() {
    // Usually only does:
    do_method();

    // but now we have a single method where we could put logging or profiling
    // instructions that would automatically apply to all implementations of
    // the interface. Even those you did not write yourself.
  }

protected:
  // Here is where the abstract method goes, i.e. it's protected.
  virtual void do_method() = 0;
};

class NonVirtualImplementation : public NonVirtualInterface {
protected:
  void do_method() override { std::cout << "do something\n"; }
};

int main() {}
