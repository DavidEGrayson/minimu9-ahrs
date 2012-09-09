OBJs := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
BIN := minimu9-ahrs

CC := g++

CPPFLAGS += -I.

# All warnings
CPPFLAGS += -Wall

# Use a modern language
CPPFLAGS += --std=c++0x

# Use boost libraries
LDFLAGS += -lboost_program_options

# Put debugging info in there so we can get stack traces.
CPPFLAGS += -g -rdynamic

# Fix http://gcc.gnu.org/bugzilla/show_bug.cgi?id=42748
CPPFLAGS += -Wno-psabi

# Generate .d files with dependency info
CPPFLAGS += -MD -MP

all: vector.h.gch $(BIN)

$(BIN) : $(OBJs)

DEPs := $(OBJs:%.o=%.d)

vector.h.gch: vector.h
	$(CC) $(CPPFLAGS) $< -o $@

clean:
	@rm -fv $(BIN) $(OBJs) $(DEPs) *.o *.gch *.d

-include $(DEPs) vector.h.d
