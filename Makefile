CC       := gcc
LD       := gcc
CFLAGS   := $(CFLAGS) -Werror -Wall -Wextra -Wpedantic -MMD -MP -g -fsanitize=address
LD_FLAGS := $(LD_FLAGS) -fsanitize=address
LD_LIBRARIES := $(LD_LIBRARIES) -lSDL3 -lm

.PHONY: all
all: fact

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

fact: src/main.o src/object.o src/vec.o
	$(LD) $(LD_FLAGS) -o $@ $^ $(LD_LIBRARIES)

.PHONY: clean
clean:
	rm -f fact
	rm -f src/*.o
	rm -f src/*.d

-include $(wildcard src/*.d)
