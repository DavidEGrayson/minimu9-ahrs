OBJs := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
BIN := minimu9-ahrs

CC := g++

CPPFLAGS += -I. -Wall -Wextra -std=gnu++11 -O2

# Use the Eigen library.
EIGEN_NAME = eigen3
CPPFLAGS += $(shell pkg-config --cflags $(EIGEN_NAME))

# Use boost libraries
LDLIBS += -lboost_program_options

# Put debugging info in there so we can get stack traces.
#CPPFLAGS += -g -rdynamic -O0

# Generate .d files with dependency info
CPPFLAGS += -MD -MP

all: vector.h.gch $(BIN)

$(BIN) : $(OBJs)

DEPs := $(OBJs:%.o=%.d)

vector.h.gch: vector.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@

.PHONY: clean
clean:
	@rm -fv $(BIN) $(OBJs) $(DEPs) *.o *.gch *.d
	@rm -fr docs

.PHONY: docs
docs:
	doxygen

prefix = $(DESTDIR)/usr
bindir = $(prefix)/bin
sharedir = $(prefix)/share
mandir = $(sharedir)/man
man1dir = $(mandir)/man1

.PHONY: install
install: $(BIN)
	install $(BIN) $(bindir)
	install $(BIN)-calibrate $(bindir)
	install $(BIN)-calibrator $(bindir)
	install -m 0644 $(BIN).1 $(man1dir)
	install -m 0644 $(BIN)-calibrate.1 $(man1dir)
	install -m 0644 $(BIN)-calibrator.1 $(man1dir)

-include $(DEPs) vector.h.d
