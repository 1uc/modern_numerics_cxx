// Compile with
//     g++ -Wall -Wextra -std=c++17 usecase_files.cpp -o usecase_files
//
// Topic: Example of a RAII wrapper for a file.

#include <iostream>
#include <memory>
#include <vector>

// Idealization of a typical C-style interface to a file or more generally a
// resource. Usually, the methods would be free functions that set the value of
// an integer (the handle) and return an error code, which must be handled.
//
// Nevertheless, it demonstrates the core problems with managing a C-style
// abstraction of a resource.
class FileHandle {
public:
  // One must explicitly open the file.
  void open(std::string name) {
    std::cout << "opening: " << name << "\n";
    name_ = std::move(name);
  }

  // Worse yet: one *must* close the file, but exactly once.
  void close() {
    std::cout << "closing: " << name_ << "\n";
    name_ = "closed.";
  }

  // Finally, one can write to the file, while it's open.
  void write() { std::cout << "writing to: " << name_ << "\n"; }

private:
  std::string name_ = "uninitialized.";
};

// Okay, let's create a safe wrapper for this "resource". Imagine, that the
// actual difficult task of dealing with the resource is implemented in a C
// library, e.g., HDF5 files. We only want to write a wrapper, not reimplement
// functionality provided by the C interface.
class File {
public:
  // The resource is acquired the instant the object is initialized. So there's
  // never an object of type `File` which hasn't been initialized properly.
  // There's no way of forgetting to 'open' the file before using it.
  File(std::string name) : fh_(std::make_unique<FileHandle>()) {
    fh_->open(std::move(name));
  }

  // Even more important: the resource is released when the object is destroy.
  // This ties the lifetime of the resource to the lifetime of the object.
  ~File() {
    if (fh_ != nullptr) {
      fh_->close();
    }
  }

  // Typical for resources is that C-style interfaces to the resource provide a
  // handle to the resource. Copying the handle doesn't copy the resource,
  // e.g., copying a file handle will not create two files with identical
  // contents. It's similar for memory: the C-style resource handle for memory
  // is the pointer to the first byte. One can easily copy the resource handle
  // aka pointer. However, this doesn't imply that there's two copies of the
  // memory. Only two ways of accessing the same resource. A RAII abstraction
  // of memory is the std::vector.
  //
  // One way of detecting that something is non-copyable is to think of how it
  // must be closed. For example, if we were to copy the file handle, the
  // two handles would still point to the same file, and that file must only be
  // closed once. Not twice. Same for a pointer, we must call `free` only once.
  //
  // Having multiple handles to the same resource gets out of hand quickly.
  // Therefore, I consider handles non-copyable.
  //
  // tl;dr delete the copy constructor
  File(const File &) = delete;

  // What one can do is move resource handles. Since this doesn't change the
  // number of handles to the same resource, this isn't too complicated and a
  // lot less error prone.
  File(File &&other) { (*this) = std::move(other); }

  // If we can't copy-construct we probably can't do copy assignment either.
  File &operator=(const File &) = delete;

  // Implementing move assignment is useful and again not too complicated.
  File &operator=(File &&other) {
    // Avoid self assignment.
    if (this == &other) {
      return *this;
    }

    // Maybe not needed, but allows us to move a valid object back into an
    // object which has previously been moved.
    if (fh_ != nullptr) {
      fh_->close();
    }

    fh_ = std::move(other.fh_);
    return *this;
  }

  void write() { fh_->write(); }

private:
  // Keeping the handle inside a unique pointer is not essential. If there's a
  // value of the handle which signifies "invalid", then one can replace the
  //     if(fh_ != nullptr)
  // with the appropriate check and use the handle directly.
  std::unique_ptr<FileHandle> fh_;
};

int main() {

  // The problems of C-style resources.
  {
    FileHandle foo; // uninitialized
    foo.write();    // crash.

    // Doesn't look too evil, but you can forget. Also, do you need to close
    // the handle first?
    foo.open("foo (fh)");     // fine.
    foo.write();              // fine.
    foo.close();              // fine.
    foo.write();              // crash.
    foo.close();              // closed twice, also crashes.
    foo.open("foo (reborn)"); // fine.

    FileHandle bar;
    bar.open("bar (fh)");
    foo = bar; // sure... but what does this mean?

    foo.close(); // fine.
    bar.close(); // crash.

    // Also "foo (reborn)" got leaked.
  }

  // Let's look at RAII-style resources.
  std::cout << "------------------------------------------------------------\n";
  {
    File foo("foo (file)");
    File bar("bar (file)");

    bar.write();
    foo.write();
    foo = std::move(bar);
    foo.write();

    // There's no resource that isn't ready to be used. There's no double
    // closing. There's no leak. By design, none of these are possible.

    // Only issue: we must not use `bar` after moving away from it. Therefore,
    // we try to not move, or structure the code such that the move happens
    // such that the object that has been moved away from goes out of scope
    // quickly.
    //
    // Note: std::move is a way back into the bug prone world of C-style
    // resources.
  }

  std::cout << "------------------------------------------------------------\n";
  {
    // Small bonus: we can put RAII resouces into containers and it all just
    // works.
    std::vector<File> files;
    files.emplace_back("f1 (vec)");
    files.emplace_back("f2 (vec)");

    File foo("foo (vec)");
    files.push_back(std::move(foo));
  }

  std::cout << "------------------------------------------------------------\n";
  {
    // Small bonus: RAII plays nice with exceptions (or at least tries).

    try {
      File foo("foo (exception)");
      throw std::runtime_error("oops.");
    } catch (...) {
      // do nothing;
    }
  }

  return 0;
}

// Closing remarks:
//
//   * But I absolutely need multiple objects to have access to the same
//     `File`. Okay, no problem, use an `std::shared_ptr<File>`. Or create
//     a `SharedFile` which essentially contains a `std::shared_ptr<File>`
//     and not much else.
