CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = caffeinated
SOURCES = src/main.c src/nosleep.c src/animation.c src/sysinfo.c src/procman.c src/nettools.c src/filetools.c src/flux.c src/battery.c src/clipboard.c src/utils.c src/hash.c src/encoding.c src/timer.c src/converters.c src/text.c src/git.c src/network_ext.c
OBJECTS = $(SOURCES:.c=.o)

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lkernel32 -lws2_32 -liphlpapi -lpsapi -lgdi32
    RM = del /Q
    TARGET := $(TARGET).exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LDFLAGS = -lX11 -lXss -lXrandr -lm
    endif
    RM = rm -f
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS) $(TARGET)

.PHONY: all clean
