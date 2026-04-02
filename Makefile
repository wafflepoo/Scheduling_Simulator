CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude -g
TARGET  = simulateur
SRCDIR  = src
INCDIR  = include
OBJDIR  = obj

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@
	@echo "Compilation reussie : ./$(TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

doc:
	doxygen Doxyfile
	@echo "Documentation générée dans doc/html/index.html"

install: $(TARGET)
	@if [ -w /usr/local/bin ]; then \
		cp $(TARGET) /usr/local/bin/$(TARGET); \
		echo "Installé dans /usr/local/bin/$(TARGET)"; \
	else \
		mkdir -p $(HOME)/bin; \
		cp $(TARGET) $(HOME)/bin/$(TARGET); \
		echo "Droits insuffisants sur /usr/local/bin."; \
		echo "Installé dans $(HOME)/bin/$(TARGET)"; \
	fi

clean:
	rm -rf $(OBJDIR) $(TARGET) doc/html doc/latex
	@echo "Nettoyage terminé"

.PHONY: all doc install clean
