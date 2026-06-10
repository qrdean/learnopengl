CFLAGS = -Wall -Wextra -Iinclude $(shell pkg-config --cflags glfw3)
LDFLAGS = $(shell pkg-config --libs glfw3) -ldl

main: main.c src/glad.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean compile_flags.txt

compile_flags.txt:
	pkg-config --cflags glfw3 | tr ' ' '\n' > $@

clean:
	rm -f main
