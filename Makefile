CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Iinclude
TARGET  = bzip2_stage1
SRCDIR  = src
SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/config.c \
          $(SRCDIR)/block.c \
          $(SRCDIR)/rle.c \
		  $(SRCDIR)/bwt.c \
		  $(SRCDIR)/mtf.c \
		  $(SRCDIR)/rle2.c \
		  $(SRCDIR)/huffman.c

OBJECTS = $(SOURCES:.c=.o)

# ── Default target (Linux) ──────────────────────────────────────────────
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ── Windows cross-compile ───────────────────────────────────────────────
windows:
	$(MAKE) CC=x86_64-w64-mingw32-gcc TARGET=$(TARGET).exe

# ── Utility targets ─────────────────────────────────────────────────────
clean:
	if exist $(SRCDIR)\*.o del /q $(SRCDIR)\*.o
	if exist $(TARGET) del /q $(TARGET)
	if exist $(TARGET).exe del /q $(TARGET).exe

.PHONY: all windows clean
