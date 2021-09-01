# Use as
#     CXXFLAGS="-Wall -Wextra -std=c++17" make
#
# or
#     CXXFLAGS="-Wall -Wextra -std=c++17" make TARGET
#
# with TARGET equal to the name of one of the binaries, e.g., `syntax`.


TARGETS := usecase_odes syntax frequent_patterns virtual_destructor

ALL: $(TARGETS)

%: $*.cpp

clean:
	rm -f $(TARGETS)
