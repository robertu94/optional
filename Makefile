CXX ?= clang++
CXXFLAGS += -std=c++11
ifdef DEBUG
	CXXFLAGS += -g -O0 -Wall -Werror -Wpedantic
else
	CXXFLAGS += -O3
endif

.PHONY: all clean
optional: optional.cc optional.hpp
	$(CXX) $(CXXFLAGS) $^ -o $@
test: optional
	./optional
clean:
	$(RM) $(wildcard *.o) $(BINS)
