CC      := gcc
LD      := gcc
CFLAGS  := $(CFLAGS)  -Werror -Wall -Wextra -Wpedantic -MMD -MP -g -fsanitize=address
LDFLAGS := $(LDFLAGS) -fsanitize=address
LDLIBS  := $(LDLIBS)  -lm

# SDL
CFLAGS  := $(CFLAGS) $(shell pkg-config --cflags sdl3)
LDLIBS  := $(LDLIBS) $(shell pkg-config --libs   sdl3)

.PHONY: all
all: fact

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

fact: src/main.o src/object.o src/vec.o src/render.o src/conveyor.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	rm -f fact
	rm -f src/*.o
	rm -f src/*.d

-include $(wildcard src/*.d)
