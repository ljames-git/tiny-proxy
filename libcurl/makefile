PWD := `pwd`
SRC := $(wildcard *.c) $(wildcard *.cpp) $(wildcard */*.c) $(wildcard */*.cpp)
OBJ := $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRC)))
DEP := $(patsubst %.cpp, %.d, $(patsubst %.c, %.d, $(SRC)))
GXX := g++
TARGET := libcurl.a
CINCLUDE := -I. -Iinclude
CFLAGS := -g -c -Wall -O0 $(CINCLUDE)
MACROS := -DBUILDING_LIBCURL -DHAVE_STRUCT_TIMEVAL -DHAVE_CONFIG_H
LDFLAGS := -lpthread


CFLAGS += $(MACROS)

.PHONY: all

all: $(TARGET)


$(TARGET): $(OBJ)
	ar crv $@ $(OBJ)

include $(DEP)

%.o: %.cpp
	$(GXX) -o $@ $(CFLAGS) $<

%.d: %.cpp
	set -e 
	rm -f $@
	$(GXX) -MM $(CINCLUDE) $(MACROS) $< | sed "s,\($*\)\.o[ :]*,\1.o $@ : ,g" > $@

%.d: %.c
	set -e 
	rm -f $@
	$(GXX) -MM $(CINCLUDE) $(MACROS) $< | sed "s,\($*\)\.o[ :]*,\1.o $@ : ,g" > $@

.PHONY: clean everything

everything: clean all

clean:  
	rm -f $(DEP) $(OBJ) $(TARGET)
