NOTE: The state is of the collection is that of a draft! It's certainly full of
typos and likely contains some more serious errors too.

# Modern Numerics oriented C++
This is a very incomplete and drafty collection of topics in C++ that are
useful in the context of numerical simulations. It should generalize to other
areas since it's rarely highly numerics specific.

## Compiling
Unless there's a build system in place:

    g++ -std=c++17 FILE.cpp -o FILE

is a good choice; and a bad habit since you should turn on warnings, e.g.,
`-Wall`, `-Wextra`, `-Wconversion`.
