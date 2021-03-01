CXX := g++

SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:src/%.cpp=obj/%.o)
DEPS := $(SRCS:src/%.cpp=obj/%.d)

DEPFLAGS = -MMD -MP

Run: $(OBJS)
	$(CXX) -o $@ $^

$(OBJS) : obj/%.o : src/%.cpp
	$(CXX) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(DEPS) main
