PWD := `pwd`
SRC := $(wildcard *.c) $(wildcard *.cpp)
OBJ := $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRC)))
DEP := $(patsubst %.cpp, %.d, $(patsubst %.c, %.d, $(SRC)))

TARGET := tiny_proxy

.PHONY: all clean

all: $(TARGET)


$(TARGET): $(OBJ)
	rm -f *.d
	g++ $^ -o $@

include $(DEP)
	    
%.d: %.cpp
	set -e 
	rm -f $@
	g++ -M  $< > $@

%.d: %.c
	set -e 
	rm -f $@
	g++ -M  $< > $@

clean:  
	rm -f *.d *.o $(TARGET)
