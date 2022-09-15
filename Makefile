.PHONY: all plot

CXX = clang++-12
CXXFLAGS = -g3 -Wall -Wextra -fsanitize=address -std=c++2a
LDFLAGS = -fsanitize=address
PROGS = reliability

all: $(PROGS)

main.o: main.cpp reliability.h em_model.h wearout_model.h

reliability: main.o
	$(CXX) -o $@ $(LDFLAGS) $^

plot: $(PROGS)
	./reliability < constant-temperature.log > time-R.csv
	python3 plot-both.py

run: $(PROGS)
	./reliability < constant-temperature.log

clean:
	rm -f $(PROGS) *.o
