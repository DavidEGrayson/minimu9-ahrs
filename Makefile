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
#CPPFLAGS += -g -rdynamic

# Optimize the code.
CPPFLAGS += -O3

# Fix http://gcc.gnu.org/bugzilla/show_bug.cgi?id=42748
CPPFLAGS += -Wno-psabi

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
	install -m 0644 $(BIN)-calibrator.1 $(DESTDIR)$(man1dir)

-include $(DEPs) vector.h.d
