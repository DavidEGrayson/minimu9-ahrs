OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
BIN := minimu9-ahrs

CC := g++

CPPFLAGS += -I.

CPPFLAGS += -Wall

CPPFLAGS += --std=c++0x

# Fix http://gcc.gnu.org/bugzilla/show_bug.cgi?id=42748
CPPFLAGS += -Wno-psabi

# Generate .d files with dependency info
CPPFLAGS += -MD -MP

all: $(BIN)

$(BIN) : $(OBJS)

DEPS := $(OBJS:%.o=%.d)

clean:
	@rm -fv $(BIN) $(OBJS) $(DEPS) *.o

-include $(DEPS)
