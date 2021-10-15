// `std::shared_ptr` is a reference counted smart pointer. As the name suggests
// it's for dealing with objects that have shared ownership of the object the
// smart pointer refers to. Ownership means controlling the life time of the
// object. In particular, making sure the object is kept alive sufficiently
// long.
//
// The idea is to deallocate the object as soon as there are no more
// `shared_ptr`s holding that object. Conceptually this mean a `shared_ptr`
// consists of a pointer to the object and a pointer to an integer type
// counting the number of `smart_ptr`s referring to this object. The count is
// increased atomically. Which implies that copying `shared_ptr` can be copied
// safely in a multi-threaded context. Note, that this does not imply that
// using the object is thread safe.
//
// The atomic increment/decrement is a source for concern w.r.t. performance.
// However, in a typical HPC application `shared_ptr` aren't copied in the
// inner most loop.
//
// `std::unique_ptr` can hold objects that shall only have one owner. A
// `unique_ptr` can be moved but not copied. Just like the `shared_ptr` it ties
// the lifetime of the contained object to the lifetime of the smart pointer.
//
// Pitfall: If one were to ever create a cycle of `shared_ptr`. Then the
// reference count would never drop to zero, even though the smart pointers
// can't be access anymore. This is where a `std::weak_ptr` comes into play. It
// avoids the problem of a dangling pointer. While the object a `weak_ptr`
// refers to can be destroyed while the `weak_ptr` is alive, unlike a raw
// pointer, the `weak_ptr` will be equal to a `nullptr` if the object has been
// destroyed. Therefore, it's possible to avoid accessing the freed memory.
