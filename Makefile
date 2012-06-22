OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
BIN := minimu9-ahrs

CC := g++

CPPFLAGS += -Wall

# Generate .d files with dependency info
CPPFLAGS += -MD -MP

all: $(BIN)

$(BIN) : $(OBJS)

DEPS := $(OBJS:%.o=%.d)

clean:
	@rm -fv $(BIN) $(OBJS) $(DEPS) *.o

-include $(DEPS)
