CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRCDIR = src
OBJDIR = obj
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
EXEC = scheduler

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	if not exist $(OBJDIR) mkdir $(OBJDIR)

doc:
	doxygen Doxyfile

clean:
	rm -rf $(OBJDIR) $(EXEC) doc/

install: $(EXEC)
	cp $(EXEC) /usr/local/bin/

.PHONY: all doc clean install