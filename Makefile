
EXE := adtool
SRCDIR := src
OBJDIR := obj

SOURCES += $(shell find $(SRCDIR)/ -name '*.c')
OBJS := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))

CFLAGS += -Isrc/libs/
CFLAGS += -g -Wall -Wformat
CFLAGS += `pkg-config --cflags gtk+-3.0`
CFLAGS += -rdynamic
LIBS += `pkg-config --libs gtk+-3.0`
LIBS += -lldap -llber -lresolv -lgsasl

$(shell mkdir -p $(OBJDIR))

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/libs/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
