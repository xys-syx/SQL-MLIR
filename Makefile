CXX      ?= clang++-17
TARGET   ?= bench
SRCS     := main.cpp sqlite_loader.cpp tectorwise.cpp \
            tectorwise_simd.cpp typer.cpp
BS       ?= 1000

OBJS     := $(SRCS:.cpp=.o)
CXXFLAGS := -std=c++17 -Wall -Wextra -O3 -march=native \
            -fopenmp -DBATCH_SIZE=$(BS)
LDFLAGS  := -fopenmp -lsqlite3

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: debug run clean

debug: CXXFLAGS = -std=c++17 -Wall -Wextra -g -O0 -fopenmp -DBATCH_SIZE=$(BS)
debug: $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) results.csv
