OBJs := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
BIN := minimu9-ahrs

CC := g++

CPPFLAGS += -I.

CPPFLAGS += -Wall

CPPFLAGS += --std=c++0x

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
