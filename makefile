PWD := `pwd`
SRC := $(wildcard *.c) $(wildcard *.cpp)
OBJ := $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRC)))
DEP := $(patsubst %.cpp, %.d, $(patsubst %.c, %.d, $(SRC)))
GXX := g++
TARGET := tiny_proxy
CFLAGS := -g -c -Wall -O0
MACROS := 
LDFLAGS := -lpthread


CFLAGS += $(MACROS)

.PHONY: all

all: $(TARGET)


$(TARGET): $(OBJ)
	$(GXX) $(LDFLAGS) $^ -o $@

include $(DEP)

%.d: %.cpp
	set -e 
	rm -f $@
	$(GXX) -MM  $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@
	echo -e '\t$$(GXX) -o $$@ $$(CFLAGS) $$(INCLUDEFLAGS) $$<' >> $@
	echo -e '\trm -f $@' >> $@

%.d: %.c
	set -e 
	rm -f $@
	$(GXX) -MM  $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@
	echo -e '\t$$(GXX) -o $$@ $$(CFLAGS) $$(INCLUDEFLAGS) $$<' >> $@

.PHONY: clean everything

everything: clean all

clean:  
	rm -f *.d *.o $(TARGET)
