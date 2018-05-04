# The files to be compiled

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=%.o)
PROGS = $(SRCS:%.cpp=%)

# The rules

all: $(PROGS)

$(PROGS): % : %.cpp
	g++ -DUNIX -Wall -g -O0 -o $@ $@.cpp -I ../include -L .. -lnewbase

clean:
	rm -f $(PROGS) $(OBJS) *~

