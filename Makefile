OBJs := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
BIN := minimu9-rpi-ahrs

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

.PHONY: clean
clean:
	@rm -fv $(BIN) $(OBJs) $(DEPs) *.o *.gch *.d
	@rm -fr docs

.PHONY: docs
docs:
	doxygen

prefix = /usr
bindir = $(prefix)/bin
sharedir = $(prefix)/share
mandir = $(sharedir)/man
man1dir = $(mandir)/man1

install: $(BIN)
	install $(INSTALL_OPTS) $(BIN) $(DESTDIR)$(bindir)
	install $(INSTALL_OPTS) -m 0644 $(BIN).1 $(DESTDIR)$(man1dir)

-include $(DEPs) vector.h.d
