CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g
TARGET = simulateur
SRCDIR = src
INCDIR = include
OBJDIR = obj

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

doc:
	doxygen Doxyfile

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

clean:
	rm -rf $(OBJDIR) $(TARGET) doc/html doc/latex

.PHONY: all doc install clean