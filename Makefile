CXX = clang++-12
#CXX = g++-10

# address sanitizer
CXXFLAGS = -g3 -Wall -Wextra -fsanitize=address -std=c++2a
LDFLAGS = -fsanitize=address

# memory sanitizer
# CXXFLAGS = -g3 -Wall -Wextra -fsanitize=memory -std=c++2a
# LDFLAGS = -fsanitize=memory

# valgrind
#CXXFLAGS = -g3 -Wall -Wextra -std=c++2a

# fast
# CXXFLAGS = -O3 -Wall -Wextra -std=c++2a

# profiling gprof
# CXXFLAGS = -pg -O3 -Wall -Wextra -std=c++2a

PROGS = reliability

all: $(PROGS)

clean:
	rm -f $(PROGS) *.o
