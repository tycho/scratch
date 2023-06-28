LTOFLAG := -flto
EXTRAFLAGS ?=

CFLAGS := -O2 $(LTOFLAG) -fno-exceptions -fno-rtti -fno-strict-aliasing -fwrapv $(EXTRAFLAGS)
LDFLAGS := $(LTOFLAG) $(EXTRAFLAGS)

SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

all: test

clean:
	rm -f test $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

test: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS)
