OBJS := $(patsubst %.cpp, %.o, $(wildcard *.c))
BIN := minimu9-ahrs

CPPFLAGS += -Wall

# Generate .d files with dependency info
CPPFLAGS += -MD -MP

all: $(BIN)

DEPS := $(OBJS:%.o=%.d)

clean:
	@rm -fv $(BIN) $(OBJS) $(DEPS) *.o

-include $(DEPS)
