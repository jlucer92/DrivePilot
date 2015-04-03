CC := g++
CFLAGS := -Wall -Wno-write-strings -std=c++0x -pthread#-v  
ALLDEPS := 
SRCDIR := src
BUILDDIR := build
LIB := -lm -lrt


SRCEXT := cpp
HDRPAT := -name *.h
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
HEADERS := $(shell find $(SRCDIR) -type f $(HDRPAT))
ALLDEP += $(HEADERS)
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
INC := -I include -I$(LIBDIR)/include/ -I$(LIBDIR)/lib/

all: $(OBJECTS)
	@echo "$(CC) $(CFLAGS) $(LIB) $(FW) $(INC) -o build/dp $(OBJECTS)" ; $(CC) $(CFLAGS) $(LIB) $(FW) $(INC) -o build/dp $(OBJECTS)

$(BUILDDIR)/%.o:	$(SRCDIR)/%.$(SRCEXT) $(ALLDEP)
	@mkdir -p $(BUILDDIR)
	@echo "$(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm build/*.o
	rm build/dp
