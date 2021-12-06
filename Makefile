CPPFLAGS =
CPPFLAGS += -std=c++17 -g -Wall -Werror -g -ggdb3
CPPFLAGS += -I/opt/boost_1_74_0/include

LDFLAGS = -ludev

all: tool

tool: tool.cpp
	$(CXX) -o $@ $^ $(CPPFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f tool tool.o

